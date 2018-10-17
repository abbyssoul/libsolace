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
 * libSolace: atom
 *	@file		solace/atom.hpp
 ******************************************************************************/
#pragma once
#ifndef SOLACE_ATOM_HPP
#define SOLACE_ATOM_HPP

#include "solace/types.hpp"


namespace Solace {


/**
 * Atoms value type
 */
enum class AtomValue : uint64_t {
    /// @cond PRIVATE
    _dirty_little_hack = 1337
    /// @endcond
};

namespace detail {
uint64 atom_val(const char* cstr, uint64 interim = 0xF);
}

/// Creates an atom from given string literal.
template <size_t Size>
AtomValue atom(char const (&str)[Size]) {
    // last character is the NULL terminator
    static_assert(Size <= 11, "only 10 characters are allowed");

    return static_cast<AtomValue>(detail::atom_val(str));
}

}  // End of namespace Solace
#endif  // SOLACE_ATOM_HPP
