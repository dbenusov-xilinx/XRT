// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.

#include "pcidrv_xocl.h"

#include "system_linux.h"

namespace {

struct X
{
  X() { xrt_core::pci::register_driver(std::make_shared<xrt_core::pci::drv_xocl>()); }
} x;

}