// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020-2022 Xilinx, Inc
// Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.

// ------ I N C L U D E   F I L E S -------------------------------------------
// Local - Include Files
#include "SubCmdProgram.h"

#include "OO_ChangeBoot.h"
#include "OO_FactoryReset.h"
#include "OO_UpdateBase.h"
#include "OO_UpdateShell.h"
#include "OO_UpdateXclbin.h"

#include "tools/common/XBHelpMenusCore.h"
#include "tools/common/XBUtilitiesCore.h"
#include "tools/common/XBUtilities.h"
#include "tools/common/XBUtilitiesCore.h"
namespace XBU = XBUtilities;

#include "xrt.h"
#include "core/common/system.h"
#include "core/common/device.h"
#include "core/common/error.h"
#include "core/common/query_requests.h"
#include "core/common/message.h"
#include "core/common/utils.h"
#include "flash/flasher.h"
#include "core/common/info_vmr.h"
// Remove linux specific code
#ifdef __linux__
#include "core/pcie/linux/scan.h"
#endif

// 3rd Party Library - Include Files
#include <boost/format.hpp>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

// System - Include Files
#include <iostream>

#ifdef _WIN32
#pragma warning(disable : 4996) //std::asctime
#endif


// =============================================================================

// ----- C L A S S   M E T H O D S -------------------------------------------

std::string device_str;
bool help = false;

SubCmdProgram::SubCmdProgram(bool _isHidden, bool _isDepricated, bool _isPreliminary)
    : SubCmd("program",
             "Update image(s) for a given device")
{
  const std::string longDescription = "Updates the image(s) for a given device.";
  setLongDescription(longDescription);
  setExampleSyntax("");
  setIsHidden(_isHidden);
  setIsDeprecated(_isDepricated);
  setIsPreliminary(_isPreliminary);

  m_commonOptions.add_options()
    ("device,d", boost::program_options::value<decltype(device_str)>(&device_str), "The Bus:Device.Function (e.g., 0000:d8:00.0) device of interest.")
    ("help", boost::program_options::bool_switch(&help), "Help to use this sub-command")
  ;

  addSubOption(std::make_shared<OO_UpdateBase>("base", "b"));
  addSubOption(std::make_shared<OO_UpdateShell>("shell", "s"));
  addSubOption(std::make_shared<OO_FactoryReset>("revert-to-golden"));
  addSubOption(std::make_shared<OO_UpdateXclbin>("user", "u"));
  addSubOption(std::make_shared<OO_ChangeBoot>("boot", "", true));
}

void
SubCmdProgram::execute(const SubCmdOptions& _options) const
// Reference Command:  [-d card] [-r region] -p xclbin
//                     Download the accelerator program for card 2
//                       xbutil program -d 2 -p a.xclbin
{
  XBU::verbose("SubCommand: program");

  XBU::verbose("Option(s):");
  for (auto & aString : _options) {
    std::string msg = "   ";
    msg += aString;
    XBU::verbose(msg);
  }

  // Parse sub-command ...
  po::variables_map vm;
  auto topOptions = process_arguments(vm, _options, m_commonOptions, m_hiddenOptions, m_positionals, m_subOptionOptions, false);

  // Check for a suboption
  auto optionOption = checkForSubOption(vm);

  if (optionOption) {
    optionOption->execute(_options);
    return;
  }

  // Check to see if help was requested or no command was found
  if (help) {
    printHelp(m_commonOptions, m_hiddenOptions, m_subOptionOptions);
    return;
  }

  std::cout << "\nERROR: Missing operation.  No action taken.\n\n";
  printHelp(m_commonOptions, m_hiddenOptions, m_subOptionOptions);
  throw xrt_core::error(std::errc::operation_canceled);
}
