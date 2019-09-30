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
 * libSolace: atom type
 *	@file		solace/atom.hpp
 ******************************************************************************/
#pragma once
#ifndef SOLACE_ATOM_HPP
#define SOLACE_ATOM_HPP

#include "solace/types.hpp"
#include "solace/stringView.hpp"
#include "solace/result.hpp"

#include <type_traits>  // std::is_integral
#include <climits>


namespace Solace {


/**
 * Atoms value type
 */
enum class AtomValue : std::uintmax_t {
    /// @cond PRIVATE
    _dirty_little_hack = 1337
    /// @endcond
};


namespace detail {

template <typename T = std::uintmax_t>
constexpr std::enable_if_t<std::is_integral<T>::value, T>
wrap(char const* const str, std::size_t len = sizeof(T)) noexcept {
	constexpr auto N = sizeof(T);
    T n {};
	std::size_t i{};
	while (i < N && i < len && str[i]) {
        n = (n << CHAR_BIT) | str[i++];
    }

	return (i == 0)
			? 0
			: (n << (N - i) * CHAR_BIT);
}

template <typename T>
std::enable_if_t<std::is_integral<T>::value>
unwrap(const T n, char *const buffer) noexcept {
    constexpr auto N = sizeof(T);
    constexpr auto lastbyte = static_cast<char>(~0);
    for (std::size_t i = 0UL; i < N; ++i) {
        buffer[i] = ((n >> (N - i - 1) * CHAR_BIT) & lastbyte);
    }

    buffer[N] = '\0';
}

}  // namespace detail


/// Creates an atom value from a given short string literal.
template <size_t Size>
[[nodiscard]]
constexpr AtomValue atom(char const (&str)[Size]) noexcept {
    // last character is the NULL terminator
    constexpr auto kMaxLiteralSize = sizeof(std::uintmax_t);
    static_assert(Size <= kMaxLiteralSize, "String literal too long");

    return static_cast<AtomValue>(detail::wrap(str));
}

inline
void atomToString(AtomValue a, char *const buffer) noexcept {
    detail::unwrap<std::uintmax_t>(static_cast<std::uintmax_t>(a), buffer);
}


struct ParseError {};

Result<AtomValue, ParseError>
tryParseAtom(StringView str) noexcept;

}  // End of namespace Solace
#endif  // SOLACE_ATOM_HPP
