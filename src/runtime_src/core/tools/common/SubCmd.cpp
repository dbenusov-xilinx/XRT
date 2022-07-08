/**
 * Copyright (C) 2019-2022 Xilinx, Inc
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
#include "SubCmd.h"
#include <iostream>
#include <boost/format.hpp>

#include "core/common/error.h"
#include "XBUtilitiesCore.h"
#include "XBHelpMenusCore.h"
namespace XBU = XBUtilities;
namespace po = boost::program_options;

SubCmd::SubCmd(const std::string & _name, 
               const std::string & _shortDescription)
  : m_executableName("")
  , m_subCmdName(_name)
  , m_shortDescription(_shortDescription)
  , m_longDescription("")
  , m_isHidden(false)
  , m_isDeprecated(false)
  , m_isPreliminary(false)
  , m_defaultDeviceValid(true)
{
  // Empty
}

void 
SubCmd::report_subcommand_help( bool removeLongOptDashes,
                                const std::string& customHelpSection) const
{
  // Formatting color parameters
  // Color references: https://en.wikipedia.org/wiki/ANSI_escape_code
  const std::string fgc_header      =  XBU::is_escape_codes_disabled() ? "" : XBU::ec::fgcolor(XBU::FGC_HEADER).string();
  const std::string fgc_headerBody  =  XBU::is_escape_codes_disabled() ? "" : XBU::ec::fgcolor(XBU::FGC_HEADER_BODY).string();
  const std::string fgc_poption      = XBU::is_escape_codes_disabled() ? "" : XBU::ec::fgcolor(XBU::FGC_POSITIONAL).string();
  const std::string fgc_poptionBody  = XBU::is_escape_codes_disabled() ? "" : XBU::ec::fgcolor(XBU::FGC_POSITIONAL_BODY).string();
  const std::string fgc_usageBody   =  XBU::is_escape_codes_disabled() ? "" : XBU::ec::fgcolor(XBU::FGC_USAGE_BODY).string();
  const std::string fgc_extendedBody = XBU::is_escape_codes_disabled() ? "" : XBU::ec::fgcolor(XBU::FGC_EXTENDED_BODY).string();
  const std::string fgc_reset       =  XBU::is_escape_codes_disabled() ? "" : XBU::ec::fgcolor::reset();

  // -- Command description
  {
    static const std::string key = "DESCRIPTION: ";
    auto formattedString = XBU::wrap_paragraphs(m_longDescription, static_cast<unsigned int>(key.size()), XBU::maxColumnWidth - static_cast<unsigned int>(key.size()), false);
    boost::format fmtHeader(fgc_header + "\n" + key + fgc_headerBody + "%s\n" + fgc_reset);
    if ( !formattedString.empty() )
      std::cout << fmtHeader % formattedString;
  }

  // Sub Command usage
  std::cout << boost::format(fgc_header + "\nUSAGES:\n");
  boost::format fmtCmdUsage(fgc_usageBody + "%s %s [--%s]\n" + fgc_reset);
  for (const auto & subCmd : m_sub_options) {
    if (subCmd->isHidden())
      continue;

    std::cout << fmtCmdUsage % m_executableName % m_subCmdName % subCmd->longName();
  }

  // Command Usage
  boost::format fmtUsage(fgc_usageBody + "%s %s%s\n" + fgc_reset);
  for (const auto& usage_path : m_options.usage_paths) {
    const std::string usage = XBU::create_usage_string(usage_path, m_options.all_positionals, removeLongOptDashes);
    std::cout << fmtUsage % m_executableName % m_subCmdName % usage;
  }

  // -- Add positional arguments
  boost::format fmtOOSubPositional(fgc_poption + "  %-15s" + fgc_poptionBody + " - %s\n" + fgc_reset);
  for (auto option : m_options.all_options.options()) {
    if ( !XBU::isPositional( option->canonical_display_name(po::command_line_style::allow_dash_for_short),
                          m_options.all_positionals))  {
      continue;
    }

    std::string optionDisplayFormat = XBU::create_option_format_name(option.get(), false);
    unsigned int optionDescTab = 33;
    auto formattedString = XBU::wrap_paragraphs(option->description(), optionDescTab, XBU::maxColumnWidth, false);

    std::string completeOptionName = option->canonical_display_name(po::command_line_style::allow_dash_for_short);
    std::cout << fmtOOSubPositional % ("<" + option->long_name() + ">") % formattedString;
  }


  // -- Options
  XBU::report_option_help("OPTIONS", m_options.all_options, m_options.all_positionals, false, removeLongOptDashes);

  // -- Custom Section
  std::cout << customHelpSection << "\n";

  // -- Global Options
  XBU::report_option_help("GLOBAL OPTIONS", m_globalOptions, m_options.all_positionals, false);

  if (XBU::getShowHidden()) 
    XBU::report_option_help("OPTIONS (Hidden)", m_options.hidden_options, m_options.all_positionals, false);

  // Extended help
  {
    boost::format fmtExtHelp(fgc_extendedBody + "\n  %s\n" +fgc_reset);
    auto formattedString = XBU::wrap_paragraphs(m_exampleSyntax, 2, XBU::maxColumnWidth, false);
    if (!formattedString.empty()) 
      std::cout << fmtExtHelp % formattedString;
  }
}

static void
addUniqueOptions( const boost::program_options::options_description & new_options,
                  boost::program_options::options_description & existing_options)
{
  for (const auto& new_option : new_options.options()) {
    const auto& d = existing_options.find_nothrow(new_option->long_name(), false, false, false);
    if (!d)
      existing_options.add(new_option);
  }
}

void 
SubCmd::addUsage(const boost::program_options::options_description& options, const std::string& description)
{
  XBU::usage_options common_usage(options, description);
  m_options.usage_paths.push_back(common_usage);
  addCommonOptions(common_usage.options);
}

void
SubCmd::addSubCmd(const std::shared_ptr<OptionOptions>& sub_cmd)
{
  m_sub_options.emplace_back(sub_cmd);

  // Store the name within the appropriate option list
  po::options_description options("");
  options.add_options()(sub_cmd->longName().c_str(), sub_cmd->description().c_str());
  if (sub_cmd->isHidden())
    addHiddenOptions(options);
  else
    addCommonOptions(options);

  sub_cmd->setExecutable(getExecutableName());
  sub_cmd->setCommand(getName());
}

void
SubCmd::addCommonOptions(const boost::program_options::options_description & options)
{
  addUniqueOptions(options, m_options.all_options);
}

void
SubCmd::addHiddenOptions(const boost::program_options::options_description & options)
{
  addUniqueOptions(options, m_options.hidden_options);
}

void
SubCmd::addPositional(const std::string& name, int max_count)
{
  // Positionals cannot be validated as they are position based
  // vs name based
  m_options.all_positionals.add(name.c_str(), max_count);
}

void 
SubCmd::printHelp(bool removeLongOptDashes,
                const std::string& customHelpSection) const
{
  report_subcommand_help(removeLongOptDashes, customHelpSection);
}

void
SubCmd::printHelp( const boost::program_options::options_description & _optionDescription,
                   const boost::program_options::options_description & _optionHidden,
                   bool removeLongOptDashes,
                   const std::string& customHelpSection) const
{
  boost::program_options::positional_options_description emptyPOD;
  XBUtilities::report_subcommand_help(m_executableName, m_subCmdName, m_longDescription,  m_exampleSyntax, _optionDescription, _optionHidden, emptyPOD, m_globalOptions, removeLongOptDashes, customHelpSection);
}

void
SubCmd::printHelp( const boost::program_options::options_description & _optionDescription,
                   const boost::program_options::options_description & _optionHidden,
                   const SubOptionOptions & _subOptionOptions) const
{
 report_subcommand_help(false, "");
}

std::vector<std::string> 
SubCmd::process_arguments( po::variables_map& vm,
                           const SubCmdOptions& _options,
                           bool validate_arguments) const
{
  po::options_description all_options("All Options");
  all_options.add(m_options.all_options);
  all_options.add(m_options.hidden_options);

  try {
    po::command_line_parser parser(_options);
    return XBU::process_arguments(vm, parser, all_options, m_options.all_positionals, validate_arguments);
  } catch(boost::program_options::error& e) {
    std::cerr << boost::format("ERROR: %s\n") % e.what();
    printHelp();
    throw xrt_core::error(std::errc::operation_canceled);
  }
}

std::vector<std::string> 
SubCmd::process_arguments( po::variables_map& vm,
                           const SubCmdOptions& _options,
                           const po::options_description& common_options,
                           const po::options_description& hidden_options,
                           const po::positional_options_description& positionals,
                           const SubOptionOptions& suboptions,
                           bool validate_arguments) const
{
  po::options_description all_options("All Options");
  all_options.add(common_options);
  all_options.add(hidden_options);

  try {
    po::command_line_parser parser(_options);
    return XBU::process_arguments(vm, parser, all_options, positionals, validate_arguments);
  } catch(boost::program_options::error& e) {
    std::cerr << boost::format("ERROR: %s\n") % e.what();
    printHelp(common_options, hidden_options, suboptions);
    throw xrt_core::error(std::errc::operation_canceled);
  }
}


void 
SubCmd::conflictingOptions( const boost::program_options::variables_map& _vm, 
                            const std::string &_opt1, const std::string &_opt2) const
{
  if ( _vm.count(_opt1.c_str())  
       && !_vm[_opt1.c_str()].defaulted() 
       && _vm.count(_opt2.c_str()) 
       && !_vm[_opt2.c_str()].defaulted()) {
    std::string errMsg = boost::str(boost::format("Mutually exclusive options: '%s' and '%s'") % _opt1 % _opt2);
    throw std::logic_error(errMsg);
  }
}
