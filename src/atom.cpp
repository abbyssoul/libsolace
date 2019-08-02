/*
*  Copyright 2017 Ivan Ryabov
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
 *	@file		atom.cpp
 *	@brief		Implementation of Atom type.
 ******************************************************************************/
#include "solace/atom.hpp"

using namespace Solace;


namespace {

// encodes ASCII characters to 6bit encoding
constexpr unsigned char kEncodingTable[] = {
/*       ..0 ..1 ..2 ..3 ..4 ..5 ..6 ..7 ..8 ..9 ..A ..B ..C ..D ..E ..F  */
/* 0..*/  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* 1..*/  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* 2..*/  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* 3..*/  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 0,  0,  0,  0,  0,  0,
/* 4..*/  0, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
/* 5..*/ 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,  0,  0,  0,  0, 37,
/* 6..*/  0, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52,
/* 7..*/ 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,  0,  0,  0,  0,  0};

/*
// decodes 6bit characters to ASCII
constexpr char kDecodingTable[] = " 0123456789"
                                  "ABCDEFGHIJKLMNOPQRSTUVWXYZ_"
                                  "abcdefghijklmnopqrstuvwxyz";


constexpr uint64 nextInterim(uint64 current, size_t char_code) {
    return (current << 6) | kEncodingTable[(char_code <= 0x7F) ? char_code : 0];
}
*/

}  // namespace

/*
uint64
atomVal(const char* cstr, uint64 interim) noexcept {
    return (*cstr == '\0')
            ? interim
            : atomVal(cstr + 1, nextInterim(interim, static_cast<size_t>(*cstr)));
}
*/

Result<AtomValue, ParseError>
Solace::tryParseAtom(StringView str) noexcept {
	// last character is the NULL terminator
	constexpr auto kMaxLiteralSize = sizeof(std::uintmax_t);
	if (str.size() > kMaxLiteralSize) {
		return ParseError{};
	}

	return Ok(static_cast<AtomValue>(detail::wrap(str.data(), str.size())));
}

