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
 * libSolace: Semantic versioning version class
 *	@file		solace/version.hpp
 *	@brief		Semantic Version class implementation
 ******************************************************************************/
#pragma once
#ifndef SOLACE_VERSION_HPP
#define SOLACE_VERSION_HPP

#include "solace/string.hpp"
#include "solace/result.hpp"
#include "solace/error.hpp"


namespace Solace {

/** Immutable Semantic Version class.
 * Represents a Version following semantic versioning specification
 *
 * Given a version number MAJOR.MINOR.PATCH, increment the:
 *
 *   MAJOR version when you make incompatible API changes,
 *   MINOR version when you add functionality in a backwards-compatible manner, and
 *   PATCH version when you make backwards-compatible bug fixes.
 *
 *   Additional labels for pre-release and build metadata are available as
 *   extensions to the MAJOR.MINOR.PATCH format.
 *
 * Please @see http://semver.org/ for more details
 */
class Version {
public:
    using value_type = uint32;

    static const StringView::value_type NumberSeparator;
    static const StringView::value_type ReleaseSeparator;
    static const StringView::value_type BuildSeparator;

public:
	//!< The major version, to be incremented on incompatible changes.
	value_type		majorNumber{};
	//!< The minor version, to be incremented when functionality is added in a backwards-compatible manner.
	value_type		minorNumber{};
	//!< The patch version, to be incremented when backwards-compatible bug fixes are made.
	value_type		patchNumber{};
	//!< The pre-release version identifier, if one exists.
	String			preRelease;
	//!< The build metadata, ignored when determining version precedence.
	String			build;

public:

    /**
     * Construct the version object from a string representation.
     * @return Parsed object or an error.
     */
    static Result<Version, Error>
    parse(StringView value) noexcept;

    static Result<Version, Error>
    parse(String const& value) noexcept {
        return parse(value.view());
    }


public:

	/** Empty version constructor */
	constexpr Version() noexcept = default;

	/** Construct the version object by specifying only numeric components */
	constexpr Version(value_type aMajor, value_type aMinor, value_type aPatch) noexcept
        : majorNumber{aMajor}
        , minorNumber{aMinor}
        , patchNumber{aPatch}
	{}

    /** Construct the version object by specifying all components */
    Version(value_type aMajor, value_type aMinor, value_type aPatch, StringLiteral aPre)
        : majorNumber{aMajor}
        , minorNumber{aMinor}
        , patchNumber{aPatch}
        , preRelease{makeString(aPre)}
        , build{}
    {}

	// cppcheck-suppress passedByValue
	Version(value_type aMajor, value_type aMinor, value_type aPatch, String aPre)
        : majorNumber{aMajor}
        , minorNumber{aMinor}
        , patchNumber{aPatch}
		, preRelease{mv(aPre)}
        , build{}
    {}

	/** Construct the version object by specifying all components */
    // cppcheck-suppress passedByValue
    Version(value_type aMajor, value_type aMinor, value_type aPatch, String aPre, String aBuild)
        : majorNumber{aMajor}
        , minorNumber{aMinor}
        , patchNumber{aPatch}
		, preRelease{mv(aPre)}
		, build{mv(aBuild)}
	{}

    /** Construct the version object by specifying all components */
    // cppcheck-suppress passedByValue
    Version(value_type aMajor, value_type aMinor, value_type aPatch, StringLiteral aPre, StringLiteral aBuild)
        : majorNumber{aMajor}
        , minorNumber{aMinor}
        , patchNumber{aPatch}
        , preRelease{makeString(aPre)}
        , build{makeString(aBuild)}
    {}

public:

    bool operator> (Version const& rhv) const noexcept;

	bool operator< (Version const& rhv) const noexcept {
		return !(operator >(rhv));
	}

    /*constexpr*/ bool equals(Version const& rhv) const noexcept {
        return ((majorNumber == rhv.majorNumber)
				&& (minorNumber == rhv.minorNumber)
				&& (patchNumber == rhv.patchNumber)
				&& (preRelease.equals(rhv.preRelease)));
	}


    /*constexpr*/ Version& swap(Version& rhs) noexcept {
        using std::swap;

        swap(majorNumber, rhs.majorNumber);
        swap(minorNumber, rhs.minorNumber);
        swap(patchNumber, rhs.patchNumber);
        swap(preRelease, rhs.preRelease);
        swap(build, rhs.build);

        return (*this);
    }


    /**
     * String representation of this object.
     * @return String representing this version object.
     */
    String toString() const;
};


/**
 * Get build version of the linked library.
 * @return Build version of libsolace we are linked against.
 */
Version getBuildVersion() noexcept;


inline
void swap(Version& lhs, Version& rhs) noexcept {
    lhs.swap(rhs);
}

inline
bool operator!= (Version const& lhs, Version const& rhv) noexcept {
    return !lhs.equals(rhv);
}

inline
bool operator== (Version const& lhs, Version const& rhv) noexcept {
    return lhs.equals(rhv);
}


}  // namespace Solace
#endif  // SOLACE_VERSION_HPP
