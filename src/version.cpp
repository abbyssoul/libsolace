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
 ******************************************************************************/
#include "solace/version.hpp"
#include "solace/posixErrorDomain.hpp"
#include "solace/stringBuilder.hpp"

#include "solace/libsolace_config.hpp"		// Defines compile time version

#include <cstdlib>


#define SOLACE_VERSION_MAJOR 0
#define SOLACE_VERSION_MINOR 3
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
	auto const versionSize = StringBuilder::measureFormatted(majorNumber)
			+ StringBuilder::measureFormatted(minorNumber)
			+ StringBuilder::measureFormatted(patchNumber)
			+ 2 * StringBuilder::measure(NumberSeparator);

	auto const releaseStringSize = (preRelease.empty() ?  0: StringBuilder::measure(ReleaseSeparator) + preRelease.size());
	auto const buildStringSize = (build.empty() ? 0: StringBuilder::measure(BuildSeparator) + build.size());

	auto const bufferSize = versionSize + releaseStringSize + buildStringSize + 1;
	auto sb = StringBuilder{getSystemHeapMemoryManager().allocate(bufferSize)};
	sb.append(majorNumber)
			.append(NumberSeparator)
			.append(minorNumber)
			.append(NumberSeparator)
			.append(patchNumber);

	if (!preRelease.empty()) {
		sb.append(ReleaseSeparator)
				.append(preRelease);
	}

	if (!build.empty()) {
		sb.append(BuildSeparator)
				.append(build);
	}

	return sb.build();
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
		return makeError(BasicError::InvalidInput, "Version::parse()");
    }


    String preRelease;
    String build;

    if (!afterPatch.empty()) {
        splitIndex = 0;

		Optional<Error> maybeError;
		afterPatch.split(BuildSeparator, [&](StringView split) {
			if (maybeError)  // Short-circuit in case of an error
				return;

            if (splitIndex == 0) {
				auto maybePrerelease = makeString(split.trim(ReleaseSeparator));
				if (!maybePrerelease) {
					maybeError = maybePrerelease.moveError();
				} else {
					preRelease = maybePrerelease.moveResult();
				}
            } else if (splitIndex == 1) {
				auto maybeBuild = makeString(split);
				if (!maybeBuild) {
					maybeError = maybeBuild.moveError();
				} else {
					build = maybeBuild.moveResult();
				}

            }

            ++splitIndex;
        });

		if (maybeError) {
			return maybeError.move();
		}

        if (splitIndex > 2) {
			return makeError(BasicError::InvalidInput, "Version::parse()");
        }
    }

	return Ok<Version>({majorVersion, minorVersion, patchVersion,
						mv(preRelease), mv(build)});
}
