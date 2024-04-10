// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.

#include "pcidrv_xclmgmt.h"

#include "pcidev_alveo.h"
#include "system_linux.h"

namespace {

struct X
{
  X() { xrt_core::pci::register_driver(std::make_shared<xrt_core::pci::drv_xclmgmt>()); }
} x;

}

namespace xrt_core { namespace pci {

std::shared_ptr<dev>
drv_xclmgmt::
create_pcidev(const std::string& sysfs) const
{
  return std::make_shared<shim_alveo::pdev>(shared_from_this(), sysfs);
}

}} // xrt_core :: pci
