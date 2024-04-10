// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.

#ifndef PCIDEV_ALVEO_H
#define PCIDEV_ALVEO_H

#include "core/pcie/linux/device_linux.h"
#include "core/pcie/linux/pcidev.h"

namespace shim_alveo {

class pdev : public xrt_core::pci::dev
{
public:
  pdev(std::shared_ptr<const xrt_core::pci::drv> driver, std::string sysfs_name);
  ~pdev();

  virtual int
  pcieBarRead(uint64_t offset, void* buf, uint64_t len) const override;

  virtual int
  pcieBarWrite(uint64_t offset, const void* buf, uint64_t len) const override;

  virtual int
  poll(int devhdl, short events, int timeout_ms) override;

  virtual int
  flock(int devhdl, int op) override;

  virtual int
  get_partinfo(std::vector<std::string>& info, void* blob = nullptr) override;

  virtual std::shared_ptr<dev>
  lookup_peer_dev() override;

  // Hand out a "device" instance that is specific to this type of device.
  // Caller will use this device to access device specific implementation of ishim.
  virtual std::shared_ptr<xrt_core::device>
  create_device(xrt_core::device::handle_type handle, xrt_core::device::id_type id) const override;

  // Hand out an opaque "shim" handle that is specific to this type of device.
  // On legacy Alveo device, this handle can be used to lookup a device instance and
  // make xcl HAL API calls.
  // On new platforms, this handle can only be used to look up a device. HAL API calls
  // through it are not supported any more.
  virtual xrt_core::device::handle_type
  create_shim(xrt_core::device::id_type id) const override;

private:
  int
  map_usr_bar() const;

  mutable std::mutex m_lock;

  // BAR mapped in by tools, default is BAR0
  int m_user_bar =              0;
  size_t m_user_bar_size =      0;
  // Virtual address of memory mapped BAR0, mapped on first use, once mapped, never change.
  mutable char *m_user_bar_map = reinterpret_cast<char *>(MAP_FAILED);
};

} // namespace shim_alveo

#endif
