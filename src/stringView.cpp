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

#include <cstring>      // strlen
#include <algorithm>    // std::min


using namespace Solace;


StringView::StringView(const char* data) noexcept
    : StringView((data != nullptr)
               ? narrow_cast<size_type>(std::strlen(data))
               : 0,
               data)
{
}


bool
StringView::equals(StringView str) const noexcept {
    auto const thisLen = size();
    if (thisLen != str.size()) {
        return false;
    }

    for (size_type i = 0; i < thisLen; ++i) {
        if (_data[i] != str._data[i]) {
            return false;
        }
    }

    return true;
}


int
StringView::compareTo(StringView x) const noexcept {
    auto const thisSize = size();
    auto const xSize = x.size();

    if (thisSize > xSize) {
        return charAt(xSize - 1);
    } else if (thisSize < xSize) {
        return -x.charAt(xSize - 1);
    } else {  // thisSize == xSize
        return strncmp(_data, x._data, thisSize);
    }

}


StringView
StringView::substring(size_type from, size_type to) const noexcept {
    auto const thisSize = size();

    from = std::min(from, thisSize);
    size_type const newSize = to - from;
    size_type const maxSize = thisSize - from;

    return {std::min(maxSize, newSize), _data + from};
}


Optional<StringView::size_type>
StringView::indexOf(value_type ch, size_type fromIndex) const noexcept {
    auto const thisSize = size();
    if (thisSize < fromIndex) {
        return none;
    }

    for (; fromIndex < thisSize; ++fromIndex) {
        if (_data[fromIndex] == ch) {
            return Optional<size_type>(fromIndex);
        }
    }

    return none;
}


Optional<StringView::size_type>
StringView::indexOf(StringView str, size_type fromIndex) const noexcept {
    auto const thisSize = size();
    auto const strSize = str.size();

    if (thisSize < fromIndex) {
        return none;
    }

    if (thisSize < strSize) {
        return none;
    }

    if (thisSize < fromIndex + strSize) {
        return none;
    }

    for (; fromIndex + strSize < thisSize + 1; ++fromIndex) {
        if (_data[fromIndex] == str._data[0] &&
            str.equals(substring(fromIndex, fromIndex + strSize))) {

            return Optional<size_type>(fromIndex);
        }
    }

    return none;
}



Optional<StringView::size_type>
StringView::lastIndexOf(value_type ch, size_type fromIndex) const noexcept {
    auto const thisSize = size();
    if (thisSize < fromIndex) {
        return none;
    }

    Optional<size_type> result;
    for (; fromIndex < thisSize; ++fromIndex) {
        if (_data[fromIndex] == ch) {
            result = Optional<size_type>(fromIndex);
        }
    }

    return result;
}

Optional<StringView::size_type>
StringView::lastIndexOf(StringView str, size_type fromIndex) const noexcept {
    auto const thisSize = size();
    auto const strSize = str.size();

    if ((thisSize < fromIndex) || (thisSize < strSize) || (thisSize < fromIndex + strSize)) {
        return none;
    }

    Optional<size_type> result;
    for (; fromIndex + strSize < thisSize + 1; ++fromIndex) {
        if (_data[fromIndex] == str._data[0] &&
            str.equals(substring(fromIndex, fromIndex + strSize))) {
            result = Optional<size_type>(fromIndex);
        }
    }

    return result;
}


bool
StringView::startsWith(StringView prefix) const noexcept {
    auto const prefixSize = prefix.size();
    if (size() < prefixSize) {
        return false;
    }

    return prefix.equals(substring(0, prefixSize));
}


bool
StringView::endsWith(StringView suffix) const noexcept {
    auto const thisSize = size();
    auto const suffixSize = suffix.size();
    if (thisSize < suffixSize) {
        return false;
    }

//    return strncmp(_data + (size() - str.size()), str._data, str.size());
    return suffix.equals(substring(thisSize - suffixSize));
}


StringView
StringView::trim() const noexcept {
    size_type fromIndex = 0;

    while (fromIndex < length() && Char::isWhitespace(_data[fromIndex])) {
        ++fromIndex;
    }

    size_type toIndex = length();
    while (fromIndex + 1 < toIndex && Char::isWhitespace(_data[toIndex - 1])) {
        --toIndex;
    }

    return substring(fromIndex, toIndex);
}

StringView
StringView::trim(value_type delim) const noexcept {
    size_type fromIndex = 0;

    while (fromIndex < length() && _data[fromIndex] == delim) {
        ++fromIndex;
    }

    size_type toIndex = length();
    while (fromIndex + 1 < toIndex && _data[toIndex - 1] == delim) {
        --toIndex;
    }

    return substring(fromIndex, toIndex);
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
