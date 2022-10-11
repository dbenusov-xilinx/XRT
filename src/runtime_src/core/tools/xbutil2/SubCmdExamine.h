// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 Xilinx, Inc
// Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.

#ifndef __SubCmdExamine_h_
#define __SubCmdExamine_h_

#include "tools/common/SubCmd.h"
#include "tools/common/SubCmdExamineInternal.h"

class SubCmdExamine : public SubCmdExamineInternal {
 public:
  SubCmdExamine(bool _isHidden, bool _isDepricated, bool _isPreliminary);
};

#endif
