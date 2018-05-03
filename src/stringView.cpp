/*
*  Copyright 2018 Ivan Ryabov
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
 *	@file		solace/stringView.cpp
 *	@brief		Implementation of string view class.
 ******************************************************************************/
#include "solace/stringView.hpp"

#include <cstring>  // strlen
#include <regex>


using namespace Solace;


StringView::StringView(const char* data) :
    StringView(assertNotNull(data), data ? strlen(data) : 0)
{
}


bool
StringView::equals(StringView str) const noexcept {
    if (size() != str.size())
        return false;

    for (size_type i = 0; i < size(); ++i) {
        if (_data[i] != str._data[i])
            return false;
    }

    return true;
}


StringView
StringView::substring(size_type from) const {
    assertIndexInRange(from, 0, size() + 1);

    const size_type newSize = _size - from;

    return {_data + from, newSize};
}


StringView
StringView::substring(size_type from, size_type len) const {
    assertIndexInRange(from,  0,  size() + 1, "StringView::substring(from) const");
    assertIndexInRange(from + len,   from,  size() + 1, "StringView::substring(len) const");

    return {_data + from, len};
}


Optional<StringView::size_type>
StringView::indexOf(const value_type& ch, size_type fromIndex) const {
    assertIndexInRange(fromIndex,  0,  size() + 1, "StringView::indexOf() const");

    for (; fromIndex < size(); ++fromIndex) {
        if (_data[fromIndex] == ch) {
            return Optional<size_type>::of(fromIndex);
        }
    }

    return None();
}

Optional<StringView::size_type>
StringView::indexOf(const StringView& str, size_type fromIndex) const {
    assertIndexInRange(fromIndex,  0,  size() + 1, "StringView::indexOf() const");

    if (size() < str.size()) {
        return None();
    }

    for (; fromIndex + str.size() < size() + 1; ++fromIndex) {
        if (_data[fromIndex] == str._data[0] &&
            substring(fromIndex, str.length()) == str) {
            return Optional<size_type>::of(fromIndex);
        }
    }

    return None();
}



Optional<StringView::size_type>
StringView::lastIndexOf(const value_type& ch, size_type fromIndex) const {
    assertIndexInRange(fromIndex,  0,  size() + 1, "StringView::lastIndexOf() const");

    Optional<size_type> result;
    for (; fromIndex < size(); ++fromIndex) {
        if (_data[fromIndex] == ch) {
            result = Optional<size_type>::of(fromIndex);
        }
    }

    return result;
}

Optional<StringView::size_type>
StringView::lastIndexOf(const StringView& str, size_type fromIndex) const {
    assertIndexInRange(fromIndex,  0,  size() + 1, "StringView::lastIndexOf() const");

    if (size() < str.size()) {
        return None();
    }

    Optional<size_type> result;
    for (; fromIndex + str.size() < size() + 1; ++fromIndex) {
        if (_data[fromIndex] == str._data[0] &&
            substring(fromIndex, str.length()) == str) {
            result = Optional<size_type>::of(fromIndex);
        }
    }

    return result;
}


bool
StringView::startsWith(StringView prefix) const noexcept {
    if (size() < prefix.size()) {
        return false;
    }

//    return strncmp(_data, str._data, str.size()) == 0;
    return (substring(0, prefix.length()) == prefix);
}


bool
StringView::endsWith(StringView suffix) const noexcept {
    if (size() < suffix.size()) {
        return false;
    }

//    return strncmp(_data + (size() - str.size()), str._data, str.size());
    return (substring(size() - suffix.size(), suffix.length()) == suffix);
}


StringView
StringView::trim() const {
    size_type fromIndex = 0;

    while (fromIndex < length() && Char::isWhitespace(_data[fromIndex])) {
        ++fromIndex;
    }

    size_type toIndex = length();
    while (fromIndex + 1 < toIndex && Char::isWhitespace(_data[toIndex - 1])) {
        --toIndex;
    }

    return substring(fromIndex, toIndex - fromIndex);
}


uint64
StringView::hashCode() const noexcept {
    uint64 result = 0;
    const uint64 prime = 31;
    for (size_t i = 0; i < _size; ++i) {
        result = _data[i] + (result * prime);
    }

    return result;
}


Array<StringView>
StringView::split(value_type delim) const {

    size_type delimCount = 0;
    for (const auto c : *this){
        if (c == delim) {
            ++delimCount;
        }
    }

    Array<StringView> result(delimCount + 1);
    Array<StringView>::size_type i = 0;
    size_type to = 0, from = 0;
    for (; to < size(); ++to) {
        if (_data[to] == delim) {
            result[i++] = substring(from, to - from);
            from = to + 1;
        }
    }

    result[i++] = substring(from, size() - from);


    return result;
}

Array<StringView>
StringView::split(const StringView& delim) const {

    const auto delimLength = delim.length();
    size_type delimCount = 0;

    for (size_type i = 0; i < size() && delimLength + i <= size(); ++i){
        if (substring(i, delimLength) == delim) {
            ++delimCount;
            i += delimLength - 1;
        }
    }

    Array<StringView> result(delimCount + 1);
    Array<StringView>::size_type i = 0;
    size_type to = 0, from = 0;
    for (; to < size() && delimLength + to <= size(); ++to) {
        if (substring(to, delimLength) == delim) {
            result[i++] = substring(from, to - from);
            to += delimLength - 1;
            from = to + 1;
        }
    }

    result[i++] = substring(from, size() - from);


    return result;
}
