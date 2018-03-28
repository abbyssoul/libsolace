/*
*  Copyright 2016 Ivan Ryabov
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
 * libSolace: Command line parser utilities
 *	@file		solace/framework/commandlineUtils.hpp
 *	@author		$LastChangedBy: $
 *	@date		$LastChangedDate: $
 *	@brief		Command line parser helpers
 *	ID:			$Id: $
 ******************************************************************************/
#pragma once
#ifndef SOLACE_FRAMEWORK_COMMANDLINEUTILS_HPP
#define SOLACE_FRAMEWORK_COMMANDLINEUTILS_HPP

#include "solace/types.hpp"
#include "solace/string.hpp"
#include "solace/optional.hpp"
#include "solace/version.hpp"

#include <ostream>


namespace Solace { namespace Framework {

/**
 * A helper class to print application version string into an ouput stream.
 * This helper follows GNU '--version' effect description minus copyright string.
 * See https://www.gnu.org/prep/standards/standards.html#g_t_002d_002dversion for more details.
 *
 * The handler is intended to respond to '--version' option to print name and version to standard output and then exit.
 * Following options will be ignored after this handler has been invoked.
 *
 * The output line is meant to be easy to parse and contains the canonical name for of the program:
 *   my_app 1.3.21-release
 *
 * The program’s name should be provided to the construcor and it is not recommended to use argv[0] as
 * it is possible for the same program to be called via different file names / links.
 *
 * GNU standards also recommend that if a program is a part of a package, mention the package name in parentheses:
 *   my_app (My Package) 1.4.32-release
 *
 */
class VersionPrinter {
public:

    VersionPrinter(const char* canonicalAppName, const Version& v, std::ostream& output) :
        _canonicalAppName(canonicalAppName),
        _version(v),
        _output(output)
    {
    }

    Optional<Error> operator() (CommandlineParser::Context& c) {
        _output << _canonicalAppName << " " << _version << std::endl;

        c.stopParsing();
        return None();
    }

private:
    const char*     _canonicalAppName;
    Version         _version;

    std::ostream&   _output;
};


/**
 *
 */
class HelpFormatter {
public:

    HelpFormatter(std::ostream& output):
        _output(output)
    {}

    Optional<Error> operator() (CommandlineParser::Context& c);

protected:

    void formatOption(char prefixChar, const CommandlineParser::Option& option);
    void formatCommand(const String& name, const CommandlineParser::Command& cmd);

private:
    std::ostream& _output;
};


}  // End of namespace Framework
}  // End of namespace Solace
#endif  // SOLACE_FRAMEWORK_COMMANDLINEUTILS_HPP
