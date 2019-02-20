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
#include "solace/posixErrorDomain.hpp"

#include "solace/libsolace_config.hpp"		// Defines compile time version

#include <string>       // std::to_string


#define SOLACE_VERSION_MAJOR 0
#define SOLACE_VERSION_MINOR 2
#define SOLACE_VERSION_BUILD 1



using namespace Solace;


const StringView::value_type Version::NumberSeparator{'.'};
const StringView::value_type Version::ReleaseSeparator{'-'};
const StringView::value_type Version::BuildSeparator{'+'};


Version
Solace::getBuildVersion() noexcept {
    return Version{ SOLACE_VERSION_MAJOR,
					SOLACE_VERSION_MINOR,
                    SOLACE_VERSION_BUILD};
}


bool
Version::operator > (Version const& rhv) const noexcept {
    if (majorNumber > rhv.majorNumber) {
        return true;
    } else if (majorNumber < rhv.majorNumber) {
        return false;
    } else {
        if (minorNumber > rhv.minorNumber) {
            return true;
        } else if (minorNumber < rhv.minorNumber) {
            return false;
        } else {
            return patchNumber > rhv.patchNumber;
        }
    }
}


String
Version::toString() const {
    auto const majorString = std::to_string(majorNumber);
    auto const minorString = std::to_string(minorNumber);
    auto const patchString = std::to_string(patchNumber);

    auto s1 = makeStringJoin(NumberSeparator,
                               StringView(majorString.data(), majorString.size()),
                               StringView(minorString.data(), minorString.size()),
                               StringView(patchString.data(), patchString.size()));

    auto s2 = (preRelease.empty())
              ? makeString(StringLiteral{""})
              : makeString(ReleaseSeparator, preRelease);

    auto s3 = build.empty()
        ? makeString(StringLiteral{""})
        : makeString(BuildSeparator, build);

    return makeString(s1, s2, s3);
}


Result<Version, Error>
Version::parse(StringView str) noexcept {
    value_type majorVersion;
    value_type minorVersion;
    value_type patchVersion;

    StringView afterPatch;
    StringView::size_type splitIndex = 0;
    str.split(NumberSeparator, [&](StringView split) {
        if (splitIndex == 0) {
            majorVersion = std::strtoul(split.data(), nullptr, 10);
            ++splitIndex;
        } else if (splitIndex == 1) {
            minorVersion = std::strtoul(split.data(), nullptr, 10);
            ++splitIndex;
        } else if (splitIndex == 2) {
            char* patchEnd;
            patchVersion = std::strtoul(split.data(), &patchEnd, 10);

            if (patchEnd != str.end()) {
                ptrdiff_t dist = str.end() - patchEnd;
                afterPatch = StringView(patchEnd,
                                        narrow_cast<StringView::size_type>(dist));
            }

            ++splitIndex;
        }
    });

    if (splitIndex < 3) {
        return Err(makeError(BasicError::InvalidInput, "Version::parse()"));
    }


    String preRelease;
    String build;

    if (!afterPatch.empty()) {
        splitIndex = 0;
        afterPatch.split(BuildSeparator, [&](StringView split) {
            if (splitIndex == 0) {
                preRelease = makeString(split.trim(ReleaseSeparator));
            } else if (splitIndex == 1) {
                build = makeString(split);
            }

            ++splitIndex;
        });

        if (splitIndex > 2) {
            return Err(makeError(BasicError::InvalidInput, "Version::parse()"));
        }
    }

    return Ok<Version>({majorVersion, minorVersion, patchVersion,
                        std::move(preRelease), std::move(build)});
}
