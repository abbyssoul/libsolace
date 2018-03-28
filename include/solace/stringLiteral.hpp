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
 * libSolace: String literal
 *	@file		solace/stringLiteral.hpp
 *	@brief		Static Immutable Unicode String Literal
 ******************************************************************************/
#pragma once
#ifndef SOLACE_STRINGLITERAL_HPP
#define SOLACE_STRINGLITERAL_HPP

#include "solace/char.hpp"


namespace Solace {

/** Immutable Unicode String Literal
 * StringLiteral is a wrapper of C-string defined in place in the code.
 * Naturally it is immutable.
 */
struct StringLiteral {

    /// String size_type is intentionally small to disallow long strings.
    typedef uint16      size_type;

    typedef Char        value_type;

    template<size_t N>
    constexpr StringLiteral(const char (&str)[N]) :
        _size(N - 1),
        _data(&str[0])
    {
    }

    constexpr size_type size() const noexcept {
        return _size;
    }

    constexpr const char *c_str() const noexcept {
        return _data;
    }

    const size_type _size;
    const char *_data = nullptr;
};


}  // namespace Solace
#endif  // SOLACE_STRINGLITERAL_HPP
