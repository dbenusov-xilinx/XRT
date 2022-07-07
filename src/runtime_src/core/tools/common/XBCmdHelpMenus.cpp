/**
 * Copyright (C) 2020-2022 Xilinx, Inc
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
#include "XBCmdHelpMenus.h"

#include "XBUtilitiesCore.h"
#include "XBHelpMenusCore.h"

namespace XBU = XBUtilities;


// 3rd Party Library - Include Files
#include <boost/property_tree/json_parser.hpp>
#include <boost/format.hpp>
namespace po = boost::program_options;

// System - Include Files
#include <iostream>
#include <algorithm>
#include <numeric>

// ------ N A M E S P A C E ---------------------------------------------------
using namespace XBUtilities;

// ------ F U N C T I O N S ---------------------------------------------------
void 
XBUtilities::report_commands_help( const std::string &_executable, 
                                   const std::string &_description,
                                   const boost::program_options::options_description& _optionDescription,
                                   const boost::program_options::options_description& _optionHidden,
                                   const SubCmdsCollection &_subCmds)
{ 
  // Formatting color parameters
  // Color references: https://en.wikipedia.org/wiki/ANSI_escape_code
  const std::string fgc_header     = XBU::is_escape_codes_disabled() ? "" : ec::fgcolor(XBU::FGC_HEADER).string();
  const std::string fgc_headerBody = XBU::is_escape_codes_disabled() ? "" : ec::fgcolor(XBU::FGC_HEADER_BODY).string();
  const std::string fgc_usageBody  = XBU::is_escape_codes_disabled() ? "" : ec::fgcolor(XBU::FGC_USAGE_BODY).string();
  const std::string fgc_subCmd     = XBU::is_escape_codes_disabled() ? "" : ec::fgcolor(XBU::FGC_SUBCMD).string();
  const std::string fgc_subCmdBody = XBU::is_escape_codes_disabled() ? "" : ec::fgcolor(XBU::FGC_SUBCMD_BODY).string();
  const std::string fgc_reset      = XBU::is_escape_codes_disabled() ? "" : ec::fgcolor::reset();

  // Helper variable
  static std::string sHidden = "(Hidden)";

  XBU::usage_options usage_op;
  usage_op.options.add(_optionDescription);

  // -- Command description
  {
    static const std::string key = "DESCRIPTION: ";
    auto formattedString = XBU::wrap_paragraphs(_description, static_cast<unsigned int>(key.size()), XBU::maxColumnWidth - static_cast<unsigned int>(key.size()), false);
    boost::format fmtHeader(fgc_header + "\n" + key + fgc_headerBody + "%s\n" + fgc_reset);
    if ( !formattedString.empty() )
      std::cout << fmtHeader % formattedString;
  }

  // -- Command usage
  boost::program_options::positional_options_description emptyPOD;
  std::string usage = XBU::create_usage_string(usage_op, emptyPOD);
  usage += " [command [commandArgs]]";
  boost::format fmtUsage(fgc_header + "\nUSAGE: " + fgc_usageBody + "%s%s\n" + fgc_reset);
  std::cout << fmtUsage % _executable % usage;

  // -- Sort the SubCommands
  SubCmdsCollection subCmdsReleased;
  SubCmdsCollection subCmdsDepricated;
  SubCmdsCollection subCmdsPreliminary;

  for (auto& subCmdEntry : _subCmds) {
    // Filter out hidden subcommand
    if (!XBU::getShowHidden() && subCmdEntry->isHidden()) 
      continue;

    // Depricated sub-command
    if (subCmdEntry->isDeprecated()) {
      subCmdsDepricated.push_back(subCmdEntry);
      continue;
    }

    // Preliminary sub-command
    if (subCmdEntry->isPreliminary()) {
      subCmdsPreliminary.push_back(subCmdEntry);
      continue;
    }

    // Released sub-command
    subCmdsReleased.push_back(subCmdEntry);
  }

  // Sort the collections by name
  auto sortByName = [](const auto& d1, const auto& d2) { return d1->getName() < d2->getName(); };
  std::sort(subCmdsReleased.begin(), subCmdsReleased.end(), sortByName);
  std::sort(subCmdsPreliminary.begin(), subCmdsPreliminary.end(), sortByName);
  std::sort(subCmdsDepricated.begin(), subCmdsDepricated.end(), sortByName);


  // -- Report the SubCommands
  boost::format fmtSubCmdHdr(fgc_header + "\n%s COMMANDS:\n" + fgc_reset);  
  boost::format fmtSubCmd(fgc_subCmd + "  %-10s " + fgc_subCmdBody + "- %s\n" + fgc_reset); 
  unsigned int subCmdDescTab = 15;

  if (!subCmdsReleased.empty()) {
    std::cout << fmtSubCmdHdr % "AVAILABLE";
    for (auto & subCmdEntry : subCmdsReleased) {
      std::string sPreAppend = subCmdEntry->isHidden() ? sHidden + " " : "";
      auto formattedString = XBU::wrap_paragraphs(sPreAppend + subCmdEntry->getShortDescription(), subCmdDescTab, XBU::maxColumnWidth, false);
      std::cout << fmtSubCmd % subCmdEntry->getName() % formattedString;
    }
  }

  if (!subCmdsPreliminary.empty()) {
    std::cout << fmtSubCmdHdr % "PRELIMINARY";
    for (auto & subCmdEntry : subCmdsPreliminary) {
      std::string sPreAppend = subCmdEntry->isHidden() ? sHidden + " " : "";
      auto formattedString = XBU::wrap_paragraphs(sPreAppend + subCmdEntry->getShortDescription(), subCmdDescTab, XBU::maxColumnWidth, false);
      std::cout << fmtSubCmd % subCmdEntry->getName() % formattedString;
    }
  }

  if (!subCmdsDepricated.empty()) {
    std::cout << fmtSubCmdHdr % "DEPRECATED";
    for (auto & subCmdEntry : subCmdsDepricated) {
      std::string sPreAppend = subCmdEntry->isHidden() ? sHidden + " " : "";
      auto formattedString = XBU::wrap_paragraphs(sPreAppend + subCmdEntry->getShortDescription(), subCmdDescTab, XBU::maxColumnWidth, false);
      std::cout << fmtSubCmd % subCmdEntry->getName() % formattedString;
    }
  }

  XBU::report_option_help("OPTIONS", usage_op.options, emptyPOD);

  if (XBU::getShowHidden()) 
    XBU::report_option_help(std::string("OPTIONS ") + sHidden, _optionHidden, emptyPOD);
}

void 
XBUtilities::report_subcommand_help( const std::string &_executableName,
                                     const std::string &_subCommand,
                                     const std::string &_description, 
                                     const std::string &_extendedHelp,
                                     const boost::program_options::options_description &_optionDescription,
                                     const boost::program_options::options_description &_optionHidden,
                                     const SubCmd::SubOptionOptions & _subOptionOptions,
                                     const boost::program_options::options_description &_globalOptions)
{
  // Formatting color parameters
  // Color references: https://en.wikipedia.org/wiki/ANSI_escape_code
  const std::string fgc_header       = XBUtilities::is_escape_codes_disabled() ? "" : ec::fgcolor(FGC_HEADER).string();
  const std::string fgc_headerBody   = XBUtilities::is_escape_codes_disabled() ? "" : ec::fgcolor(FGC_HEADER_BODY).string();
  const std::string fgc_commandBody  = XBUtilities::is_escape_codes_disabled() ? "" : ec::fgcolor(FGC_SUBCMD).string();
  const std::string fgc_usageBody    = XBUtilities::is_escape_codes_disabled() ? "" : ec::fgcolor(FGC_USAGE_BODY).string();

  const std::string fgc_ooption      = XBUtilities::is_escape_codes_disabled() ? "" : ec::fgcolor(FGC_OOPTION).string();
  const std::string fgc_ooptionBody  = XBUtilities::is_escape_codes_disabled() ? "" : ec::fgcolor(FGC_OOPTION_BODY).string();
  const std::string fgc_poption      = XBUtilities::is_escape_codes_disabled() ? "" : ec::fgcolor(FGC_POSITIONAL).string();
  const std::string fgc_poptionBody  = XBUtilities::is_escape_codes_disabled() ? "" : ec::fgcolor(FGC_POSITIONAL_BODY).string();
  const std::string fgc_extendedBody = XBUtilities::is_escape_codes_disabled() ? "" : ec::fgcolor(FGC_EXTENDED_BODY).string();
  const std::string fgc_reset        = XBUtilities::is_escape_codes_disabled() ? "" : ec::fgcolor::reset();

  // -- Command
  boost::format fmtCommand(fgc_header + "\nCOMMAND: " + fgc_commandBody + "%s\n" + fgc_reset);
  if ( !_subCommand.empty() )
    std::cout << fmtCommand % _subCommand;
 
  // -- Command description
  {
    auto formattedString = XBU::wrap_paragraphs(_description, 15, XBU::maxColumnWidth, false);
    boost::format fmtHeader(fgc_header + "\nDESCRIPTION: " + fgc_headerBody + "%s\n" + fgc_reset);
    if ( !formattedString.empty() )
      std::cout << fmtHeader % formattedString;
  }

  // -- Usage
  std::string usageSubCmds;
  for (const auto & subCmd : _subOptionOptions) {
    if (subCmd->isHidden()) 
      continue;

    if (!usageSubCmds.empty()) 
      usageSubCmds.append(" | ");

    usageSubCmds.append(subCmd->longName());
  }

  std::cout << boost::format(fgc_header + "\nUSAGE: " + fgc_usageBody + "%s %s [-h] --[ %s ] [commandArgs]\n" + fgc_reset) % _executableName % _subCommand % usageSubCmds;

  // -- Options
  boost::program_options::positional_options_description emptyPOD;
  XBU::report_option_help("OPTIONS", _optionDescription, emptyPOD, false);

  // -- Global Options
  XBU::report_option_help("GLOBAL OPTIONS", _globalOptions, emptyPOD, false);

  if (XBU::getShowHidden()) 
    XBU::report_option_help("OPTIONS (Hidden)", _optionHidden, emptyPOD, false);

  // Extended help
  {
    boost::format fmtExtHelp(fgc_extendedBody + "\n  %s\n" +fgc_reset);
    auto formattedString = XBU::wrap_paragraphs(_extendedHelp, 2, XBU::maxColumnWidth, false);
    if (!formattedString.empty()) 
      std::cout << fmtExtHelp % formattedString;
  }
}
