/*
*  Copyright 2018 Ivan Ryabov
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*/
/*******************************************************************************
 * @file: cli/helpPrinter.cpp
 *
 *  Created by soultaker on 18/04/18.
*******************************************************************************/

#include "solace/cli/commandlineParser.hpp"
#include "solace/cli/commandlineUtils.hpp"
#include "solace/parseUtils.hpp"
#include "solace/path.hpp"


#include <iomanip>
#include <iostream>
#include <sstream>  // std::stringstream

#include <utility>


using namespace Solace;
using namespace Solace::cli;


void
HelpFormatter::operator() (StringView name,
                           const StringView& desc,
                           const Array<CommandlineParser::Option>& options,
                           const CommandlineParser::CommandDict& commands
                           ) {
    _output << "Usage: " << name;  // Path::parse(c.argv[0]).getBasename();

    if (!options.empty()) {
        _output << " [options]";
    }

    if (!commands.empty()) {
        _output << " <command>";
    }

    _output << std::endl;
    _output << desc << std::endl;

    if (!options.empty()) {
        _output << "Options:" << std::endl;

        for (const auto& opt : options) {
            formatOption(_optionsPrefix, opt);
        }
    }

    if (!commands.empty()) {
        _output << "Commands:" << std::endl;

        for (const auto& cmd : commands) {
            formatCommand(cmd.first, cmd.second);
        }
    }
}


void HelpFormatter::formatOption(char prefixChar, const CommandlineParser::Option& option) {
    std::stringstream s;
    s << "  ";

    bool chained = false;
    for (const auto& optName : option.names()) {
        if (chained) {
            s << ", ";
        }

        if (optName.length() == 1) {
            s << prefixChar << optName;
        } else {
            s << prefixChar << prefixChar << optName;
        }

        chained = true;
    }

    _output << std::left << std::setw(26) << s.str() << option.description() << std::endl;
}


void HelpFormatter::formatCommand(const String& name, const CommandlineParser::Command& cmd) {
    _output << "  " << std::left << std::setw(14)
            << name << cmd.description() << std::endl;
}



void
VersionPrinter::operator() (std::ostream& dest) {
    dest << _canonicalAppName << " " << _version << std::endl;
}
