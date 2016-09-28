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
 * @file: char.cpp
 *      @author: soultaker
 *
 * Created on: 21 Jul 2014
*******************************************************************************/
#include <solace/char.hpp>

#include <cstdio>
#include <cctype>
#include <cstring>
#include <utility>

using Solace::byte;
using Solace::Char;
using Solace::MemoryView;


const Char Char::Eof {static_cast<value_type>(EOF)};


Char::Char(const value_type codePoint) noexcept :
        _value(codePoint)
{
// TODO(abbyssoul): Encoding::Utf8::encode(_bytes, codePoint);
}


Char::Char(byte c) {
    memset(_bytes, 0, sizeof(_bytes));
    _bytes[0] = c;
}


Char::Char(char c) {
    memset(_bytes, 0, sizeof(_bytes));
    _bytes[0] = static_cast<byte>(c);
}


Char::Char(const MemoryView& bytes) {
    // TODO(abbyssoul): Assert::notNull(bytes);
    // TODO(abbyssoul): Assert::indexInRange(count, 0, max_bytes);

    memset(_bytes, 0, sizeof(_bytes));
    for (size_t i = 0; i < bytes.size(); ++i) {
        _bytes[i] = bytes[i];
    }
}


Char::Char(Char&& c) noexcept : _value(std::move(c._value)) {
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


const MemoryView Char::getBytes() const {
    return wrapMemory(const_cast<byte*>(_bytes), getBytesCount());
}


bool Char::isDigit() const {
    return isdigit(_value);
}


bool Char::isXDigit() const {
    return isxdigit(_value);
}


bool Char::isLetter() const {
    return isalpha(_value);
}


bool Char::isLetterOrDigit() const {
    return isalnum(_value);
}


bool Char::isUpperCase() const {
    return isupper(_value);
}


bool Char::isLowerCase() const {
    return islower(_value);
}


bool Char::isWhitespace() const {
    return isspace(_value);
}


bool Char::isSymbol() const {
    return (isLetterOrDigit() || ((_bytes[0] == '_') && (_bytes[1] == 0)));
}


bool Char::isFirstSymbol() const {
    return (isLetter() || ((_bytes[0] == '_') && (_bytes[1] == 0)));
}


bool Char::isNewLine() const {
    return ((_bytes[0] == '\n') && (_bytes[1] == 0));
}


bool Char::isCntrl() const {
    return iscntrl(_value);
}


bool Char::isGraphical() const {
    return isgraph(_value);
}


bool Char::isPrintable() const {
    return isprint(_value);
}


bool Char::isPunctuation() const {
    return ispunct(_value);
}


Char Char::toLower() const {
    return static_cast<value_type>(tolower(_value));
}


Char Char::toUpper() const {
    return static_cast<value_type>(toupper(_value));
}
