// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2024 Advanced Micro Devices, Inc. All rights reserved.

#include "pcidev_alveo.h"
#include "pcidrv.h"

#include <cassert>
#include <fstream>
#include <mutex>
#include <poll.h>
#include <regex>
#include <stdexcept>
#include <sys/file.h>
#include <vector>

#define DEV_TIMEOUT	90 // seconds

namespace {

/*
 * wordcopy()
 *
 * Copy bytes word (32bit) by word.
 * Neither memcpy, nor std::copy work as they become byte copying
 * on some platforms.
 */
inline void*
wordcopy(void *dst, const void* src, size_t bytes)
{
  // assert dest is 4 byte aligned
  assert((reinterpret_cast<intptr_t>(dst) % 4) == 0);

  using word = uint32_t;
  volatile auto d = reinterpret_cast<word*>(dst);
  auto s = reinterpret_cast<const word*>(src);
  auto w = bytes/sizeof(word);

  for (size_t i=0; i<w; ++i)
    d[i] = s[i];

  return dst;
}

inline size_t
bar_size(const std::string &dir, unsigned bar)
{
  std::ifstream ifs(dir + "/resource");
  if (!ifs.good())
    return 0;
  std::string line;
  for (unsigned i = 0; i <= bar; i++) {
    line.clear();
    std::getline(ifs, line);
  }
  long long start, end, meta;
  if (sscanf(line.c_str(), "0x%llx 0x%llx 0x%llx", &start, &end, &meta) != 3)
    return 0;
  return end - start + 1;
}

} // namespace

namespace shim_alveo {

pdev::
pdev(std::shared_ptr<const xrt_core::pci::drv> driver, std::string sysfs)
  : xrt_core::pci::dev(driver, sysfs)
{
  std::string err;

  sysfs_get<int>("", "userbar", err, m_user_bar, 0);
  m_user_bar_size = bar_size(xrt_core::pci::sysfs::dev_root + m_sysfs_name, m_user_bar);
  m_user_bar_map = reinterpret_cast<char *>(MAP_FAILED);
}

pdev::
~pdev()
{
  if (m_user_bar_map != MAP_FAILED)
    ::munmap(m_user_bar_map, m_user_bar_size);
}

int
pdev::
map_usr_bar() const
{
  std::lock_guard<std::mutex> l(m_lock);

  if (m_user_bar_map != MAP_FAILED)
    return 0;

  int dev_handle = open("", O_RDWR);
  if (dev_handle < 0)
    return -errno;

  m_user_bar_map = (char *)::mmap(0, m_user_bar_size,
                                PROT_READ | PROT_WRITE, MAP_SHARED, dev_handle, 0);

  // Mapping should stay valid after handle is closed
  // (according to man page)
  (void)close(dev_handle);

  if (m_user_bar_map == MAP_FAILED)
    return -errno;

  return 0;
}

int
pdev::
pcieBarRead(uint64_t offset, void* buf, uint64_t len) const
{
  if (m_user_bar_map == MAP_FAILED) {
    int ret = map_usr_bar();
    if (ret)
      return ret;
  }
  (void) wordcopy(buf, m_user_bar_map + offset, len);
  return 0;
}

int
pdev::
pcieBarWrite(uint64_t offset, const void* buf, uint64_t len) const
{
  if (m_user_bar_map == MAP_FAILED) {
    int ret = map_usr_bar();
    if (ret)
      return ret;
  }
  (void) wordcopy(m_user_bar_map + offset, buf, len);
  return 0;
}

int
pdev::
poll(int dev_handle, short events, int timeout_ms)
{
  pollfd info = {dev_handle, events, 0};
  return ::poll(&info, 1, timeout_ms);
}

int
pdev::
get_partinfo(std::vector<std::string>& info, void *blob)
{
  std::vector<char> buf;
  if (!blob) {
    std::string err;
    sysfs_get("", "fdt_blob", err, buf);
    if (!buf.size())
      return -ENOENT;

    blob = buf.data();
  }

  struct fdt_header *bph = (struct fdt_header *)blob;
  uint32_t version = be32toh(bph->version);
  uint32_t off_dt = be32toh(bph->off_dt_struct);
  const char *p_struct = (const char *)blob + off_dt;
  uint32_t off_str = be32toh(bph->off_dt_strings);
  const char *p_strings = (const char *)blob + off_str;
  const char *p, *s;
  uint32_t tag;
  uint32_t level = 0;

  p = p_struct;
  while ((tag = be32toh(GET_CELL(p))) != FDT_END) {
    if (tag == FDT_BEGIN_NODE) {
      s = p;
      p = PALIGN(p + strlen(s) + 1, 4);
      std::regex e("partition_info_([0-9]+)");
      std::cmatch cm;
      std::regex_match(s, cm, e);
      if (cm.size())
        level = std::stoul(cm.str(1));
      continue;
    }

    if (tag != FDT_PROP)
      continue;

    int sz = be32toh(GET_CELL(p));
    s = p_strings + be32toh(GET_CELL(p));
    if (version < 16 && sz >= 8)
      p = PALIGN(p, 8);

    if (strcmp(s, "__INFO")) {
      p = PALIGN(p + sz, 4);
      continue;
    }

    if (info.size() <= level)
      info.resize(level + 1);

    info[level] = std::string(p);

    p = PALIGN(p + sz, 4);
  }
  return 0;
}

int
pdev::
flock(int dev_handle, int op)
{
  if (dev_handle == -1) {
    errno = -EINVAL;
    return -1;
  }
  return ::flock(dev_handle, op);
}

std::shared_ptr<xrt_core::pci::dev>
pdev::
lookup_peer_dev()
{
  if (!m_is_mgmt)
    return nullptr;

  int i = 0;
  for (auto udev = xrt_core::pci::get_dev(i, true); udev; udev = xrt_core::pci::get_dev(i, true), ++i)
    if (udev->m_domain == m_domain && udev->m_bus == m_bus && udev->m_dev == m_dev)
      return udev;

  return nullptr;
}
  
xrt_core::device::handle_type
pdev::
create_shim(xrt_core::device::id_type id) const
{
  return xclOpen(id, nullptr, XCL_QUIET);
}

std::shared_ptr<xrt_core::device>
pdev::
create_device(xrt_core::device::handle_type handle, xrt_core::device::id_type id) const
{
  return std::shared_ptr<xrt_core::device_linux>(new xrt_core::device_linux(handle, id, !m_is_mgmt));
}

} // namespace shim_alveo
