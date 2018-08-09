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

#include "solace/cli/parser.hpp"
#include "solace/cli/utils.hpp"
#include "solace/parseUtils.hpp"
#include "solace/path.hpp"

#include "solace/output_utils.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>  // std::stringstream

#include <utility>


using namespace Solace;
using namespace Solace::cli;


namespace {
void formatOption(std::ostream& output, char prefixChar, Parser::Option const& option) {
    std::stringstream s;
    s << "  ";

    bool chained = false;
    for (auto const& optName : option.names()) {
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

    output << std::left << std::setw(26) << s.str() << option.description() << std::endl;
}


void formatCommand(std::ostream& output, StringView name, Parser::Command const& cmd) {
    output << "  " << std::left << std::setw(14)
            << name << cmd.description() << std::endl;
}

}  // namespace


void
HelpFormatter::operator() (std::ostream& output,
                           StringView name,
                           Parser::Command const& cmd
                           ) {
    output << "Usage: " << name;  // Path::parse(c.argv[0]).getBasename();

    if (!cmd.options().empty()) {
        output << " [options]";
    }

    if (!cmd.arguments().empty()) {
        for (auto const& arg : cmd.arguments()) {
            output << " [" << arg.name() <<"]";
        }
    }


    if (!cmd.commands().empty()) {
        output << " <command>";
    }

    output << std::endl;
    output << cmd.description() << std::endl;

    if (!cmd.options().empty()) {
        output << "Options:" << std::endl;

        for (auto const& opt : cmd.options()) {
            formatOption(output, _optionsPrefix, opt);
        }
    }

    if (!cmd.commands().empty()) {
        output << "Commands:" << std::endl;

        for (auto const& subcmd : cmd.commands()) {
            formatCommand(output, subcmd.first, subcmd.second);
        }
    }
}



void
VersionPrinter::operator() (std::ostream& output) {
    output << _canonicalAppName << " " << _version << std::endl;
}
