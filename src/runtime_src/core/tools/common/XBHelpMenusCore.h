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

#ifndef __XBHelpMenusCore_h_
#define __XBHelpMenusCore_h_

// Include files
// Please keep these to the bare minimum

#include <string>
#include <vector>
#include <utility> // Pair template
#include <boost/program_options.hpp>

// ----------------------- T Y P E D E F S -----------------------------------

namespace XBUtilities {

  typedef struct usage_options {
    boost::program_options::options_description options;
    std::string description;
    usage_options() :
      description("")
      {}
    usage_options(const boost::program_options::options_description& _options,
                  const std::string& _description = "") : 
      options(_options),
      description(_description)
      {}
  } usage_options;

  typedef struct command_options {
    boost::program_options::options_description all_options;
    boost::program_options::options_description hidden_options;
    boost::program_options::positional_options_description all_positionals;
    std::vector<struct usage_options> usage_paths;
  } command_options;

  // Temporary color objects until the supporting color library becomes available
  namespace ec {
    class fgcolor
      {
      public:
        fgcolor(uint8_t _color) : m_color(_color) {};
        std::string string() const { return "\033[38;5;" + std::to_string(m_color) + "m"; }
        static const std::string reset() { return "\033[39m"; };
        friend std::ostream& operator <<(std::ostream& os, const fgcolor & _obj) { return os << _obj.string(); }
    
    private:
      uint8_t m_color;
    };

    class bgcolor
      {
      public:
        bgcolor(uint8_t _color) : m_color(_color) {};
        std::string string() const { return "\033[48;5;" + std::to_string(m_color) + "m"; }
        static const std::string reset() { return "\033[49m"; };
        friend std::ostream& operator <<(std::ostream& os, const bgcolor & _obj) { return  os << _obj.string(); }

    private:
      uint8_t m_color;
    };
  }

  // ------ Colors ---------------------------------------------------------
  static const uint8_t FGC_HEADER          = 3;  
  static const uint8_t FGC_HEADER_BODY     = 111;
  static const uint8_t FGC_USAGE_BODY      = 252;
  static const uint8_t FGC_OPTION          = 65; 
  static const uint8_t FGC_OPTION_BODY     = 111;
  static const uint8_t FGC_SUBCMD          = 140;
  static const uint8_t FGC_SUBCMD_BODY     = 111;
  static const uint8_t FGC_POSITIONAL      = 140;
  static const uint8_t FGC_POSITIONAL_BODY = 111;
  static const uint8_t FGC_OOPTION         = 65; 
  static const uint8_t FGC_OOPTION_BODY    = 70; 
  static const uint8_t FGC_EXTENDED_BODY   = 70; 
  // ------ Miscellanous Variables -------------------------------------
  static const unsigned int maxColumnWidth = 100;

  void 
  report_subcommand_help( const std::string &_executableName,
                          const std::string &_subCommand,
                          const std::string &_description, 
                          const std::string &_extendedHelp,
                          const boost::program_options::options_description & _optionDescription,
                          const boost::program_options::options_description &_optionHidden,
                          const boost::program_options::positional_options_description & _positionalDescription,
                          const boost::program_options::options_description &_globalOptions,
                          bool removeLongOptDashes = false,
                          const std::string& customHelpSection = "");

  void 
  report_subcommand_help( const std::string &_executableName,
                          const std::string &_subCommand,
                          const std::string &_description, 
                          const std::string &_extendedHelp,
                          const command_options& options,
                          const boost::program_options::options_description &_globalOptions,
                          bool removeLongOptDashes = false,
                          const std::string& customHelpSection = "");

  void 
  report_option_help( const std::string & _groupName, 
                      const boost::program_options::options_description& _optionDescription,
                      const boost::program_options::positional_options_description & _positionalDescription,
                      bool _bReportParameter = true,
                      bool removeLongOptDashes = false);

  std::string 
  create_usage_string(const usage_options& usage,
                      const boost::program_options::positional_options_description & positionals,
                      bool removeLongOptDashes = false);

  std::vector<std::string>
  process_arguments( boost::program_options::variables_map& vm,
                     boost::program_options::command_line_parser& parser,
                     const boost::program_options::options_description& options,
                     const boost::program_options::positional_options_description& positionals,
                     bool validate_arguments);

  bool
  isPositional( const std::string &_name, 
                const boost::program_options::positional_options_description & _pod);

  std::string 
  create_option_format_name(const boost::program_options::option_description * _option,
                            bool _reportParameter = true,
                            bool removeLongOptDashes = false);
};

#endif
