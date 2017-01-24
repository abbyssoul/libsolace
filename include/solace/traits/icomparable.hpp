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
 * libSolace: Base class for all classes with comparable semantic
 *	@file		solace/traits/icomparable.hpp
 *	@author		soultaker
 *	@date		Created on: 22 Apr 2015
 *	@brief		Base exectpion types used by core library
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_TRAITS_ICOMPARABLE_HPP
#define SOLACE_TRAITS_ICOMPARABLE_HPP


namespace Solace {

/**
* Base interface for all classes that implement 'equals'
* Any class implementing comparable 'trait' should inherit from this class
*/
template <typename T>
class IComparable {
public:

    virtual ~IComparable() noexcept = default;

    /**
     * Compare if this object is 'equal' to the give.
     * It is derived class responisbility to define symantics of 'equal' as
     * default implementation does only reference comparison.
     *
     * @param rhv Object to compare this instance to.
     * @return True if objcets are 'equal'
     */
    virtual bool equals(const T& rhv) const noexcept {
        return (this == &rhv);
    }

    /**
     * Overloaded operator for syntactic sugar. @see equals
     * @param rhv The object to compare this instance to.
     * @return True if this object is equal to the given
     */
    bool operator== (const T& rhv) const noexcept {
        return equals(rhv);
    }

    /**
     * Overloaded operator for syntactic sugar. @see equals
     * @param rhv The object to compare this instance to.
     * @return True if this object is NOT equal to the given.
     */
    bool operator!= (const T& rhv) const noexcept {
        return !equals(rhv);
    }
};

}  // namespace Solace
#endif  // SOLACE_TRAITS_ICOMPARABLE_HPP
