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
 * libSolace: Unit type - void of functional programming
 *	@file		solace/uuid.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_UNIT_HPP
#define SOLACE_UNIT_HPP


#include "solace/types.hpp"


namespace Solace {

/**
 * Unit type is a "void" of functinal programming
 *
 * Unlike void in C++ an object of type Unit can actually be created.
 * Such object is identical to all other instances of this type.
 *
 * Having value of Unit type allows to have containers of this type such as
 * @see Result<Unit, Error> that otherwise are impossible
 */
struct Unit {

    bool operator== (const Unit& /*rhs*/) const noexcept { return true; }
    bool operator!= (const Unit& /*rhs*/) const noexcept { return false; }
};


constexpr Unit unit {};

}  // End of namespace Solace
#endif  // SOLACE_UNIT_HPP

