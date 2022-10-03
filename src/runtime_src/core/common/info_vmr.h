/**
 * Copyright (C) 2022 Xilinx, Inc
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You may
 * not use this file except in compliance with the License. A copy of the
 * License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#ifndef COMMON_INFO_VMR_H
#define COMMON_INFO_VMR_H

// Local - Include Files
#include "device.h"

namespace xrt_core {
namespace vmr {

XRT_CORE_COMMON_EXPORT
boost::property_tree::ptree
vmr_info(const xrt_core::device * device);

XRT_CORE_COMMON_EXPORT
bool
is_default_boot(const xrt_core::device* device);

}} // vmr, xrt

#endif
