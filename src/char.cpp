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
 * libSolace
 *	@file		char.cpp
 *	@brief		Implementation of Unicode character type
*******************************************************************************/
#include "solace/char.hpp"

#include <cstdio>
#include <cctype>
#include <cstring>


using Solace::byte;
using Solace::Char;
using Solace::MemoryView;


const Char Char::Eof {static_cast<value_type>(EOF)};


Char::Char(value_type codePoint) noexcept
	: _value{codePoint}
{
// TODO(abbyssoul): Encoding::Utf8::encode(_bytes, codePoint);
}

Char::Char(char c) {
    memset(_bytes, 0, sizeof(_bytes));
    _bytes[0] = static_cast<byte>(c);
}


Char::Char(MemoryView bytes) {
    // TODO(abbyssoul): Assert::notNull(bytes);
    // TODO(abbyssoul): Assert::indexInRange(count, 0, max_bytes);

    memset(_bytes, 0, sizeof(_bytes));
	for (MemoryView::size_type i = 0; i < bytes.size(); ++i) {
        _bytes[i] = bytes[i];
    }
}


Char& Char::swap(Char& rhs) noexcept {
    std::swap(_value, rhs._value);

    return *this;
}


Char::size_type Char::getBytesCount() const noexcept {
    size_type len = 0;

    while (len < max_bytes && _bytes[len]) {
        ++len;
    }

    return len;
}


const char* Char::c_str() const noexcept {
    return reinterpret_cast<const char *>(&(_bytes[0]));
}


MemoryView Char::getBytes() const {
    return wrapMemory(_bytes, getBytesCount());
}


bool Char::isDigit() const noexcept {
    return isdigit(_value);
}


bool Char::isXDigit() const noexcept {
    return isxdigit(_value);
}


bool Char::isLetter() const noexcept {
    return isalpha(_value);
}


bool Char::isLetterOrDigit() const noexcept {
    return isalnum(_value);
}


bool Char::isUpperCase() const noexcept {
    return isupper(_value);
}


bool Char::isLowerCase() const noexcept {
    return islower(_value);
}


bool Char::isWhitespace() const noexcept {
    return isspace(_value);
}


bool Char::isSymbol() const noexcept {
    return (isLetterOrDigit() || ((_bytes[0] == '_') && (_bytes[1] == 0)));
}


bool Char::isFirstSymbol() const noexcept {
    return (isLetter() || ((_bytes[0] == '_') && (_bytes[1] == 0)));
}


bool Char::isNewLine() const noexcept {
    return ((_bytes[0] == '\n') && (_bytes[1] == 0));
}


bool Char::isCntrl() const noexcept {
    return iscntrl(_value);
}


bool Char::isGraphical() const noexcept {
    return isgraph(_value);
}


bool Char::isPrintable() const noexcept {
    return isprint(_value);
}


bool Char::isPunctuation() const noexcept {
    return ispunct(_value);
}


Char Char::toLower() const {
    return static_cast<value_type>(tolower(_value));
}


Char Char::toUpper() const {
    return static_cast<value_type>(toupper(_value));
}
