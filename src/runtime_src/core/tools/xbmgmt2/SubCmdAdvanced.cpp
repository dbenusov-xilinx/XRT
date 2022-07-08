/**
 * Copyright (C) 2021-2022 Xilinx, Inc
 * Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
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

// ------ I N C L U D E   F I L E S -------------------------------------------
// Local - Include Files
#include "SubCmdAdvanced.h"
#include "OO_Hotplug.h"

#include "common/system.h"
#include "common/device.h"
#include "common/xclbin_parser.h"

#include "tools/common/XBUtilitiesCore.h"
#include "tools/common/XBUtilities.h"
namespace XBU = XBUtilities;

// 3rd Party Library - Include Files
#include <boost/program_options.hpp>
namespace po = boost::program_options;

// System - Include Files
#include <iostream> 

#include "common/system.h"
#include "common/device.h"
#include <boost/format.hpp>
#include <map>

static bool help = false;

// ----- C L A S S   M E T H O D S -------------------------------------------

SubCmdAdvanced::SubCmdAdvanced(bool _isHidden, bool _isDepricated, bool _isPreliminary)
    : SubCmd("advanced", 
             "Low level command operations")
{
  const std::string longDescription = "Low level command operations.";
  setLongDescription(longDescription);
  setExampleSyntax("");
  setIsHidden(_isHidden);
  setIsDeprecated(_isDepricated);
  setIsPreliminary(_isPreliminary);

  // Top level options
  po::options_description common_options("Common Options"); 
  common_options.add_options()
    ("help", boost::program_options::bool_switch(&help), "Help to use this sub-command")
  ;
  addUsage(common_options);

  po::options_description hidden_options("Hidden Options");
  addHiddenOptions(hidden_options);

  addSubCmd(std::make_shared<OO_Hotplug>("hotplug"));
}


void
SubCmdAdvanced::execute(const SubCmdOptions& _options) const
{
  XBU::verbose("SubCommand: advanced");

  // =========== Process the options ========================================

  // 1) Process the common top level options 
  po::variables_map vm;
  // Used for the suboption arguments
  auto topOptions = process_arguments(vm, _options, false);

  // DRC check between suboptions
  for (unsigned int index1 = 0; index1 < m_sub_options.size(); ++index1)
    for (unsigned int index2 = index1 + 1; index2 < m_sub_options.size(); ++index2)
      conflictingOptions(vm, m_sub_options[index1]->longName(), m_sub_options[index2]->longName());

  // Find the subOption;
  std::shared_ptr<OptionOptions> optionOption;
  for (auto subOO : m_sub_options) {
    if (vm.count(subOO->longName().c_str()) != 0) {
      optionOption = subOO;
      break;
    }
  }

  // No suboption print help
  if (!optionOption) {
    printHelp();
    return;
  }

  // 2) Process the top level options
  if (help)
    topOptions.push_back("--help");

  optionOption->setGlobalOptions(getGlobalOptions());

  // Execute the option
  optionOption->execute(topOptions);
}
