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
 * libSolace: Base interface for all classes with collection semantic
 *	@file		solace/iterable.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	@brief		Defines interface tag for all iterable types
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_ITERABLE_HPP
#define SOLACE_ITERABLE_HPP

#include <functional>
#include <algorithm>

namespace Solace {

/**
* Base interface for all classes that has collection semantics
*
* Any class implementing iterable trait should publicly inherit this interface
*/
template <
        typename C,  // Type of the collection
        typename T   // Type of the item in the collection
>
class Iterable {
public:

    virtual ~Iterable() noexcept = default;

    virtual const C& forEach(const std::function<void(const T&)> &f) const = 0;

};

}  // namespace Solace
#endif  // SOLACE_ITERABLE_HPP
