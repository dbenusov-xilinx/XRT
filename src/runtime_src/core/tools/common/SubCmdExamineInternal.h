// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.

#ifndef __SubCmdExamineInternal_h_
#define __SubCmdExamineInternal_h_

#include "Report.h"
#include "SubCmd.h"

class SubCmdExamineInternal : public SubCmd {
 public:
  virtual void execute(const SubCmdOptions &_options) const;

 public:
  SubCmdExamineInternal(bool _isHidden, bool _isDepricated, bool _isPreliminary, bool is_user_space);

 private:
  static ReportCollection   m_report_collection;
  bool                      m_is_user_space;
  std::string               m_device;
  std::vector<std::string>  m_reportNames; // Default set of report names are determined if there is a device or not
  std::vector<std::string>  m_elementsFilter;
  std::string               m_format; // Don't define default output format.  Will be defined later.
  std::string               m_output;
  bool                      m_help;
};

#endif

