/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2017-2020 Xilinx, Inc. All rights reserved.
 * Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
 * 
 *  Authors: Sonal Santan
 *           Jan Stephan <j.stephan@hzdr.de>
 *  Code copied verbatim from SDAccel xcldma kernel mode driver
 */

#include "mgmt-core.h"
#include "../xocl_xclbin.h"

static int err_info_ioctl(struct xclmgmt_dev *lro, void __user *arg)
{

	struct xclmgmt_err_info obj = { 0 };
	u32	val = 0, level = 0;
	u64	t = 0;
	int	i = 0;

	mgmt_info(lro, "Enter error_info IOCTL");

	xocl_af_get_prop(lro, XOCL_AF_PROP_TOTAL_LEVEL, &val);
	if (val > ARRAY_SIZE(obj.mAXIErrorStatus)) {
		mgmt_err(lro, "Too many levels %d", val);
		return -EINVAL;
	}

	obj.mNumFirewalls = val;
	memset(obj.mAXIErrorStatus, 0, sizeof (obj.mAXIErrorStatus));
	for (i = 0; i < obj.mNumFirewalls; ++i) {
		obj.mAXIErrorStatus[i].mErrFirewallID = i;
	}

	xocl_af_get_prop(lro, XOCL_AF_PROP_DETECTED_LEVEL, &level);
	if (level >= val) {
		mgmt_err(lro, "Invalid detected level %d", level);
		return -EINVAL;
	}
	obj.mAXIErrorStatus[level].mErrFirewallID = level;

	xocl_af_get_prop(lro, XOCL_AF_PROP_DETECTED_STATUS, &val);
	obj.mAXIErrorStatus[level].mErrFirewallStatus = val;

	xocl_af_get_prop(lro, XOCL_AF_PROP_DETECTED_TIME, &t);
	obj.mAXIErrorStatus[level].mErrFirewallTime = t;

	if (copy_to_user(arg, &obj, sizeof(struct xclErrorStatus)))
		return -EFAULT;
	return 0;
}

static int version_ioctl(struct xclmgmt_dev *lro, void __user *arg)
{
	struct xclmgmt_ioc_info obj;
	printk(KERN_INFO "%s: %s \n", DRV_NAME, __FUNCTION__);
	device_info(lro, &obj);
	if (copy_to_user(arg, &obj, sizeof(struct xclmgmt_ioc_info)))
		return -EFAULT;
	return 0;
}

static int bitstream_ioctl_axlf(struct xclmgmt_dev *lro, const void __user *arg)
{
	void *copy_buffer = NULL;
	size_t copy_buffer_size = 0;
	struct xclmgmt_ioc_bitstream_axlf ioc_obj = { 0 };
	struct axlf xclbin_obj = { {0} };
	int ret = 0;

	if (copy_from_user((void *)&ioc_obj, arg, sizeof(ioc_obj)))
		return -EFAULT;
	if (copy_from_user((void *)&xclbin_obj, ioc_obj.xclbin,
		sizeof(xclbin_obj)))
		return -EFAULT;
	if (memcmp(xclbin_obj.m_magic, ICAP_XCLBIN_V2, sizeof(ICAP_XCLBIN_V2)))
		return -EINVAL;

	copy_buffer_size = xclbin_obj.m_header.m_length;
	/* Assuming xclbin is not over 1G */
	if (copy_buffer_size > 1024 * 1024 * 1024)
		return -EINVAL;
	copy_buffer = vmalloc(copy_buffer_size);
	if (copy_buffer == NULL)
		return -ENOMEM;

	if (copy_from_user((void *)copy_buffer, ioc_obj.xclbin,
		copy_buffer_size)) {
		vfree(copy_buffer);
		return -EFAULT;
	}

	ret = xocl_xclbin_download(lro, copy_buffer);
	if (ret) {
		vfree(copy_buffer);
		return ret;
	}

	/*
	 * aws v1 requires preloading xclbin through xbmgmt. this preloaded
	 * xclbin may also need to be cached in xclmgmt.
	 * once cached, the xclbin cached will be freed next time a new xclbin
	 * is to be loaded or xclmgmt is unloaded.
	 */
	if (lro->preload_xclbin) {
		vfree(lro->preload_xclbin);
		lro->preload_xclbin = NULL;
	}
	if (atomic_read(&lro->cache_xclbin))
		lro->preload_xclbin = copy_buffer;
	else
		vfree(copy_buffer);

	return ret;
}

long mgmt_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct xclmgmt_dev *lro;
	long result = 0;
	lro = (struct xclmgmt_dev *)filp->private_data;

	BUG_ON(!lro);

	if (!lro->status.ready || _IOC_TYPE(cmd) != XCLMGMT_IOC_MAGIC)
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		result = !XOCL_ACCESS_OK(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		result = !XOCL_ACCESS_OK(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));

	if (result)
		return -EFAULT;

	mutex_lock(&lro->busy_mutex);

	switch (cmd) {
	case XCLMGMT_IOCINFO:
		result = version_ioctl(lro, (void __user *)arg);
		break;
	case XCLMGMT_IOCICAPDOWNLOAD:
		printk(KERN_ERR
			"Bitstream ioctl with legacy bitstream not supported");
		result = -EINVAL;
		break;
	case XCLMGMT_IOCICAPDOWNLOAD_AXLF:
		result = bitstream_ioctl_axlf(lro, (void __user *)arg);
		break;
	case XCLMGMT_IOCFREQSCALE:
		result = ocl_freqscaling_ioctl(lro, (void __user *)arg);
		break;
	case XCLMGMT_IOCREBOOT:
		result = capable(CAP_SYS_ADMIN) ? pci_fundamental_reset(lro) : -EACCES;
		break;
	case XCLMGMT_IOCERRINFO:
		result = err_info_ioctl(lro, (void __user *)arg);
		break;
	default:
		printk(KERN_DEBUG "MGMT default IOCTL request %u\n", cmd & 0xff);
		result = -ENOTTY;
	}
	mutex_unlock(&lro->busy_mutex);
	return result;
}

