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
 * libSolace: Base interface for all classes that can be converted to the string
 *	@file		solace/iformattable.h
 *	@author		$LastChangedBy: $
 *	@date		$LastChangedDate: $
 *	@brief		Interface of a class that can be converted into a string
 *	ID:			$Id: $
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IFORMATTABLE_HPP
#define SOLACE_IFORMATTABLE_HPP

namespace Solace {

// Forward declaration, as it is not really instantiated
class String;

/**
 * Base interface for all classes that can be converted into a string.
*/
class IFormattable {
public:

	virtual ~IFormattable() = default;

	/**
	 * Return string representation of the object
     * @return String 'representation' of this object
	 */
	virtual String toString() const = 0;
};

}  // namespace Solace
#endif  // SOLACE_IFORMATTABLE_HPP
