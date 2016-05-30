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
 * libSolace
 *	@file		version.cpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#include "solace/version.hpp"

#include "solace/libsolace_config.h"		// Defines compile time version


#include <fmt/format.h>
#include <regex>


#define SOLACE_VERSION_MAJOR 0
#define SOLACE_VERSION_MINOR 0
#define SOLACE_VERSION_BUILD 1

using Solace::String;
using Solace::Version;


static const String ComponentSeparator(".");
static const String PreSeparator("-");
static const String BuildSeparator("+");


Version Solace::getBuildVersion() {
	return Version( SOLACE_VERSION_MAJOR,
					SOLACE_VERSION_MINOR,
					SOLACE_VERSION_BUILD);
}


bool Version::operator > (const Version& rhv) const {
    if (majorNumber > rhv.majorNumber) {
        return true;
    } else if (majorNumber < rhv.majorNumber) {
        return false;
    } else {
        if (minorNumber > rhv.minorNumber) return true;
        else if (minorNumber < rhv.minorNumber) return false;
        else
            return patchNumber > rhv.patchNumber;
    }
}


String Version::toString() const {
    // FIXME(abbyssoul): Using format is an overkill in this case - a simple String concat should do!
    auto s1 = fmt::format("{1}{0}{2}{0}{3}", ComponentSeparator.to_str(), majorNumber, minorNumber, patchNumber);

    auto s2 = (preRelease.empty())
              ? String::Empty
              : PreSeparator.concat(preRelease);

    auto s3 = build.empty()
        ? String::Empty
        : BuildSeparator.concat(build);

    return String::join(String::Empty, {s1, s2, s3});
}


Version Version::parse(const String& str) {
    // FIXME(abbyssoul): Should return Error result in case of invalid string format
    const std::regex versionRegexp("(\\d+)\\.(\\d+)\\.(\\d+)(?:-([A-Za-z0-9\\-\\.]+))?(?:\\+([A-Za-z0-9\\-\\.]+))?");

    std::cmatch capture;
    if (std::regex_match(str.c_str(), capture, versionRegexp)) {
        const value_type majorVersion = std::stoul(capture[1].str());
        const value_type minorVersion = std::stoul(capture[2].str());
        const value_type patchVersion = std::stoul(capture[3].str());
        const String preRelease = (capture[4].length() > 0) ? capture[4].str() : String::Empty;
        const String build = (capture[5].length() > 0) ? capture[5].str() : String::Empty;

        return Version(majorVersion, minorVersion, patchVersion, preRelease, build);
    }

    return Version();
}
