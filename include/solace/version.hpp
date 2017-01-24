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
 *	@author		$LastChangedBy: $
 *	@date		$LastChangedDate: $
 *	@brief		Semantic Version class implementation
 *	ID:			$Id: $
 ******************************************************************************/
#pragma once
#ifndef SOLACE_VERSION_HPP
#define SOLACE_VERSION_HPP


#include "solace/traits/iformattable.hpp"
#include "solace/traits/icomparable.hpp"
#include "solace/string.hpp"


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
class Version :	public IComparable<Version>,
                public IFormattable
{
public:
	typedef uint64 		        value_type;

	//!< The major version, to be incremented on incompatible changes.
	value_type 			majorNumber;
	//!< The minor version, to be incremented when functionality is added in a backwards-compatible manner.
	value_type 			minorNumber;
	//!< The patch version, to be incremented when backwards-compatible bug fixes are made.
	value_type			patchNumber;
	//!< The pre-release version identifier, if one exists.
	String				preRelease;
	//!< The build metadata, ignored when determining version precedence.
	String				build;

public:

    /**
     * Construct the version object from a string representation
     */
    static Version parse(const String& value);


public:

	/** Empty version constructor */
	Version() :	majorNumber(0), minorNumber(0),	patchNumber(0), preRelease(), build()
	{}

	/** Construct the version object by specifying only numeric components */
	Version(value_type aMajor, value_type aMinor, value_type aPatch) :
			majorNumber(aMajor), minorNumber(aMinor), patchNumber(aPatch),
			preRelease(), build()
	{}

    /** Construct the version object by specifying all components */
    Version(value_type aMajor, value_type aMinor, value_type aPath, const String& aPre):
            majorNumber(aMajor), minorNumber(aMinor), patchNumber(aPath),
			preRelease(aPre), build()
    {}

	/** Construct the version object by specifying all components */
	Version(value_type aMajor, value_type aMinor, value_type aPath, const String& aPre, const String& aBuild) :
				majorNumber(aMajor), minorNumber(aMinor), patchNumber(aPath),
				preRelease(aPre), build(aBuild)
	{}

	/** Copy-constructor */
	Version(const Version& v) :
		majorNumber(v.majorNumber), minorNumber(v.minorNumber), patchNumber(v.patchNumber),
        preRelease(v.preRelease), build(v.build)
	{}

	/** Move-constructor */
	Version(Version&& v) :
		majorNumber(std::move(v.majorNumber)),
		minorNumber(std::move(v.minorNumber)),
		patchNumber(std::move(v.patchNumber)),
        preRelease(std::move(v.preRelease)), build(std::move(v.build))
	{}

    virtual ~Version() noexcept = default;

public:

	bool operator> (const Version& rhv) const;

	bool operator< (const Version& rhv) const {
		return !(operator >(rhv));
	}

	//!< from IComparable
	bool equals(const Version& rhv) const noexcept override {
        return ((majorNumber == rhv.majorNumber)
				&& (minorNumber == rhv.minorNumber)
				&& (patchNumber == rhv.patchNumber)
				&& (preRelease.equals(rhv.preRelease)));
	}

    using IComparable::operator!=;
    using IComparable::operator==;

	Version& swap(Version& rhs) noexcept {
        std::swap(majorNumber, rhs.majorNumber);
        std::swap(minorNumber, rhs.minorNumber);
        std::swap(patchNumber, rhs.patchNumber);
        std::swap(preRelease, rhs.preRelease);
        std::swap(build, rhs.build);

        return (*this);
    }

    Version& operator= (const Version& rhs) noexcept {
        Version(rhs).swap(*this);

        return *this;
    }

    Version& operator= (Version&& rhs) noexcept {
        return swap(rhs);
    }

    //!< @see IFormattable
	String toString() const override;
};


Version getBuildVersion();


}  // namespace Solace
#endif  // SOLACE_VERSION_HPP
