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
#include "XBHelpMenusCore.h"

#include "XBUtilitiesCore.h"

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
bool
XBUtilities::isPositional(const std::string &_name, 
             const boost::program_options::positional_options_description & _pod)
{
  // Look through the list of positional arguments
  for (unsigned int index = 0; index < _pod.max_total_count(); ++index) {
    if ( _name.compare(_pod.name_for_position(index)) == 0) {
      return true;
    }
  }
  return false;
}

std::string 
XBUtilities::create_usage_string( const usage_options& usage,
                                  bool removeLongOptDashes)
{
  const static int SHORT_OPTION_STRING_SIZE = 2;
  std::stringstream buffer;

  auto &options = usage.options.options();

  // Gather up the short simple flags
  {
    bool firstShortFlagFound = false;
    for (const auto & option : options) {
      // Get the option name
      std::string optionDisplayName = option->canonical_display_name(po::command_line_style::allow_dash_for_short);

      // See if we have a long flag
      if (optionDisplayName.size() != SHORT_OPTION_STRING_SIZE)
        continue;

      // We are not interested in any arguments
      if (option->semantic()->max_tokens() > 0)
        continue;

      // This option shouldn't be required
      if (option->semantic()->is_required()) 
        continue;

      if (!firstShortFlagFound) {
        buffer << " [-";
        firstShortFlagFound = true;
      }

      buffer << optionDisplayName[1];
    }

    if (firstShortFlagFound) 
      buffer << "]";
  }

   
  // Gather up the long simple flags (flags with no short versions)
  {
    for (const auto & option : options) {
      // Get the option name
      std::string optionDisplayName = option->canonical_display_name(po::command_line_style::allow_dash_for_short);

      // See if we have a short flag
      if (optionDisplayName.size() == SHORT_OPTION_STRING_SIZE)
        continue;

      // We are not interested in any arguments
      if (option->semantic()->max_tokens() > 0)
        continue;

      // This option shouldn't be required
      if (option->semantic()->is_required()) 
        continue;

      
      const std::string completeOptionName = removeLongOptDashes ? option->long_name() : 
				option->canonical_display_name(po::command_line_style::allow_long);
      buffer << boost::format(" [%s]") % completeOptionName;
    }
  }

  // Gather up the options with arguments
  for (const auto & option : options) {
    // Skip if there are no arguments
    if (option->semantic()->max_tokens() == 0)
      continue;

    // This option shouldn't be required
    if (option->semantic()->is_required()) 
      continue;

    std::string completeOptionName = option->canonical_display_name(po::command_line_style::allow_dash_for_short);
    
    // See if we have a long flag
    if (completeOptionName.size() != SHORT_OPTION_STRING_SIZE)
      continue;

    buffer << boost::format(" [%s arg]") % completeOptionName;
  }

  // Gather up the options with arguments (options with no short versions)
  for (const auto & option : options) {
    // Skip if there are no arguments
    if (option->semantic()->max_tokens() == 0)
      continue;

    // This option shouldn't be required
    if (option->semantic()->is_required()) 
      continue;

    const std::string optionDisplayName = option->canonical_display_name(po::command_line_style::allow_dash_for_short);

    // See if we have a short flag
    if (optionDisplayName.size() == SHORT_OPTION_STRING_SIZE)
      continue;

    const std::string completeOptionName = removeLongOptDashes ? option->long_name() : 
      option->canonical_display_name(po::command_line_style::allow_long);
      buffer << boost::format(" [%s arg]") % completeOptionName;
  }

  // Gather up the required options with arguments
  for (const auto & option : options) {
    // Skip if there are no arguments
    if (option->semantic()->max_tokens() == 0)
      continue;

    // This option is required
    if (!option->semantic()->is_required()) 
      continue;

    std::string completeOptionName = option->canonical_display_name(po::command_line_style::allow_dash_for_short);

    // We don't wish to have positional options
    if ( ::isPositional(completeOptionName, usage.positionals) ) {
      continue;
    }

    buffer << boost::format(" %s arg") % completeOptionName;
  }

  // Report the positional arguments
  for (const auto & option : options) {
    std::string completeOptionName = option->canonical_display_name(po::command_line_style::allow_dash_for_short);
    if ( ! ::isPositional(completeOptionName, usage.positionals) ) {
      continue;
    }

    buffer << " " << completeOptionName;
  }

  buffer << " " << usage.description;

  return buffer.str();
}

std::string 
XBUtilities::create_option_format_name(const boost::program_options::option_description * _option,
                          bool _reportParameter,
                          bool removeLongOptDashes)
{
  if (_option == nullptr) 
    return "";

  std::string optionDisplayName = _option->canonical_display_name(po::command_line_style::allow_dash_for_short);

  // Determine if we really got the "short" name (might not exist and a long was returned instead)
  if (!optionDisplayName.empty() && optionDisplayName[0] != '-')
    optionDisplayName.clear();

  // Get the long name (if it exists)
  std::string longName = _option->canonical_display_name(po::command_line_style::allow_long);
  if ((longName.size() > 2) && (longName[0] == '-') && (longName[1] == '-')) {
    if (!optionDisplayName.empty()) 
      optionDisplayName += ", ";

    optionDisplayName += removeLongOptDashes ? _option->long_name() : longName;
  }

  if (_reportParameter && !_option->format_parameter().empty()) 
    optionDisplayName += " " + _option->format_parameter();

  return optionDisplayName;
}

void 
XBUtilities::report_subcommand_help( const std::string &_executableName,
                        const std::string &_subCommand,
                        const std::string &_description, 
                        const std::string &_extendedHelp,
                        const command_options& _options,
                        const boost::program_options::options_description &_globalOptions,
                        bool removeLongOptDashes,
                        const std::string& customHelpSection)
{
  // Formatting color parameters
  // Color references: https://en.wikipedia.org/wiki/ANSI_escape_code
  const std::string fgc_header      = XBUtilities::is_escape_codes_disabled() ? "" : ec::fgcolor(FGC_HEADER).string();
  const std::string fgc_headerBody  = XBUtilities::is_escape_codes_disabled() ? "" : ec::fgcolor(FGC_HEADER_BODY).string();
  const std::string fgc_poption      = XBUtilities::is_escape_codes_disabled() ? "" : ec::fgcolor(FGC_POSITIONAL).string();
  const std::string fgc_poptionBody  = XBUtilities::is_escape_codes_disabled() ? "" : ec::fgcolor(FGC_POSITIONAL_BODY).string();
  const std::string fgc_usageBody   = XBUtilities::is_escape_codes_disabled() ? "" : ec::fgcolor(FGC_USAGE_BODY).string();
  const std::string fgc_extendedBody = XBUtilities::is_escape_codes_disabled() ? "" : ec::fgcolor(FGC_EXTENDED_BODY).string();
  const std::string fgc_reset       = XBUtilities::is_escape_codes_disabled() ? "" : ec::fgcolor::reset();

  // -- Command description
  {
    static const std::string key = "DESCRIPTION: ";
    auto formattedString = XBU::wrap_paragraphs(_description, static_cast<unsigned int>(key.size()), maxColumnWidth - static_cast<unsigned int>(key.size()), false);
    boost::format fmtHeader(fgc_header + "\n" + key + fgc_headerBody + "%s\n" + fgc_reset);
    if ( !formattedString.empty() )
      std::cout << fmtHeader % formattedString;
  }

  // -- Command usage
  std::cout << "\n";
  for (const auto& usage_path : _options.usage_paths) {
    const std::string usage = XBU::create_usage_string(usage_path, removeLongOptDashes);
    boost::format fmtUsage(fgc_header + "USAGE: " + fgc_usageBody + "%s %s%s\n" + fgc_reset);
    std::cout << fmtUsage % _executableName % _subCommand % usage;
  }

  // -- Add positional arguments
  boost::format fmtOOSubPositional(fgc_poption + "  %-15s" + fgc_poptionBody + " - %s\n" + fgc_reset);
  for (auto option : _options.all_options.options()) {
    if ( !::isPositional( option->canonical_display_name(po::command_line_style::allow_dash_for_short),
                          _options.all_positionals))  {
      continue;
    }

    std::string optionDisplayFormat = create_option_format_name(option.get(), false);
    unsigned int optionDescTab = 33;
    auto formattedString = XBU::wrap_paragraphs(option->description(), optionDescTab, maxColumnWidth, false);

    std::string completeOptionName = option->canonical_display_name(po::command_line_style::allow_dash_for_short);
    std::cout << fmtOOSubPositional % ("<" + option->long_name() + ">") % formattedString;
  }


  // -- Options
  report_option_help("OPTIONS", _options.all_options, _options.all_positionals, false, removeLongOptDashes);

  // -- Custom Section
  std::cout << customHelpSection << "\n";

  // -- Global Options
  report_option_help("GLOBAL OPTIONS", _globalOptions, _options.all_positionals, false);

  if (XBU::getShowHidden()) 
    report_option_help("OPTIONS (Hidden)", _options.hidden_options, _options.all_positionals, false);

  // Extended help
  {
    boost::format fmtExtHelp(fgc_extendedBody + "\n  %s\n" +fgc_reset);
    auto formattedString = XBU::wrap_paragraphs(_extendedHelp, 2, maxColumnWidth, false);
    if (!formattedString.empty()) 
      std::cout << fmtExtHelp % formattedString;
  }
}

void
XBUtilities::report_option_help( const std::string & _groupName, 
                                 const boost::program_options::options_description& _optionDescription,
                                 const boost::program_options::positional_options_description & _positionalDescription,
                                 bool _bReportParameter,
                                 bool removeLongOptDashes)
{
  // Formatting color parameters
  // Color references: https://en.wikipedia.org/wiki/ANSI_escape_code
  const std::string fgc_header     = XBUtilities::is_escape_codes_disabled() ? "" : ec::fgcolor(FGC_HEADER).string();
  const std::string fgc_optionName = XBUtilities::is_escape_codes_disabled() ? "" : ec::fgcolor(FGC_OPTION).string();
  const std::string fgc_optionBody = XBUtilities::is_escape_codes_disabled() ? "" : ec::fgcolor(FGC_OPTION_BODY).string();
  const std::string fgc_reset      = XBUtilities::is_escape_codes_disabled() ? "" : ec::fgcolor::reset();

  // Determine if there is anything to report
  if (_optionDescription.options().empty())
    return;

  // Report option group name (if defined)
  boost::format fmtHeader(fgc_header + "\n%s:\n" + fgc_reset);
  if ( !_groupName.empty() )
    std::cout << fmtHeader % _groupName;

  // Report the options
  boost::format fmtOption(fgc_optionName + "  %-18s " + fgc_optionBody + "- %s\n" + fgc_reset);
  for (auto & option : _optionDescription.options()) {
    if ( ::isPositional( option->canonical_display_name(po::command_line_style::allow_dash_for_short),
                         _positionalDescription) )  {
      continue;
    }

    std::string optionDisplayFormat = create_option_format_name(option.get(), _bReportParameter, removeLongOptDashes);
    unsigned int optionDescTab = 23;
    auto formattedString = XBU::wrap_paragraphs(option->description(), optionDescTab, maxColumnWidth - optionDescTab, false);
    std::cout << fmtOption % optionDisplayFormat % formattedString;
  }
}

void 
XBUtilities::report_subcommand_help( const std::string &_executableName,
                                     const std::string &_subCommand,
                                     const std::string &_description, 
                                     const std::string &_extendedHelp,
                                     const boost::program_options::options_description &_optionDescription,
                                     const boost::program_options::options_description &_optionHidden,
                                     const boost::program_options::positional_options_description & _positionalDescription,
                                     const boost::program_options::options_description &_globalOptions,
                                     bool removeLongOptDashes,
                                     const std::string& customHelpSection)
{
 usage_options usage("");
 usage.options.add(_optionDescription);
 usage.positionals = _positionalDescription;
 command_options command;
 command.all_options.add(_optionDescription);
 command.hidden_options.add(_optionHidden);
 command.all_positionals = _positionalDescription;
 command.usage_paths.push_back(usage);
 report_subcommand_help(_executableName, _subCommand, _description, _extendedHelp, command, _globalOptions, removeLongOptDashes, customHelpSection);
}

std::vector<std::string>
XBUtilities::process_arguments( po::variables_map& vm,
                                po::command_line_parser& parser,
                                const po::options_description& options,
                                const po::positional_options_description& positionals,
                                bool validate_arguments
                                )
{
  // Add unregistered "option"" that will catch all extra positional arguments
  po::options_description all_options(options);
  all_options.add_options()("__unreg", po::value<std::vector<std::string> >(), "Holds all unregistered options");
  po::positional_options_description all_positionals(positionals);
  all_positionals.add("__unreg", -1);

  // Parse the given options and hold onto the results
  auto parsed_options = parser.options(all_options).positional(all_positionals).allow_unregistered().run();
  
  if (validate_arguments) {
    // This variables holds options denoted with a '-' or '--' that were not registered
    const auto unrecognized_options = po::collect_unrecognized(parsed_options.options, po::exclude_positional);
    // Parse out all extra positional arguments from the boost results
    // This variable holds arguments that do not have a '-' or '--' that did not have a registered positional space
    std::vector<std::string> extra_positionals;
    for (const auto& option : parsed_options.options) {
      if (boost::equals(option.string_key, "__unreg"))
        // Each option is a vector even though most of the time they contain only one element
        for (const auto& bad_option : option.value)
          extra_positionals.push_back(bad_option);
    }

    // Throw an exception if we have any unknown options or extra positionals
    if (!unrecognized_options.empty() || !extra_positionals.empty()) {
      std::string error_str;
      error_str.append("Unrecognized arguments:\n");
      for (const auto& option : unrecognized_options)
        error_str.append(boost::str(boost::format("  %s\n") % option));
      for (const auto& option : extra_positionals)
        error_str.append(boost::str(boost::format("  %s\n") % option));
      throw boost::program_options::error(error_str);
    }
  }

  // Parse the options into the variable map
  // If an exception occurs let it bubble up and be handled elsewhere
  po::store(parsed_options, vm);
  po::notify(vm);

  // Return all the unrecognized arguments for use in a lower level command if needed
  return po::collect_unrecognized(parsed_options.options, po::include_positional);
}
