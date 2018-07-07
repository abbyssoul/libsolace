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
 *	@file		solace/string.cpp
 *	@author		$LastChangedBy: soultaker $
 *	@date		$LastChangedDate: 2009-11-18 02:28:25 +0300 (Wed, 18 Nov 2009) $
 *	@brief		Definition of extended String from APRIL.
 *	ID:			$Id: aprilString.cpp 179 2009-11-17 23:28:25Z soultaker $
 ******************************************************************************/
#include "solace/string.hpp"

#include <cstring>  // va_list
#include <regex>


using namespace Solace;



const String String::Empty(std::string(""));
const String TRUE_STRING("true");
const String FALSE_STRING("false");


String::String(const char* data) :
    _str(assertNotNull(data)) {
}

String::String(const char* data, size_type dataLength): _str(data, dataLength) {
}

String::String(const ImmutableMemoryView& buffer):
    _str(reinterpret_cast<const char*>(buffer.dataAddress()), buffer.size()) {

}

String::String(StringView view) :
        _str(view.data(), view.size())
{
}


String& String::swap(String& rhs) noexcept {
    _str.swap(rhs._str);

    return *this;
}

bool String::empty() const  noexcept {
    return _str.empty();
}

String::size_type String::length() const noexcept {
    return static_cast<size_type>(_str.length());
}

String::size_type String::size() const noexcept {
    return static_cast<size_type>(_str.length());
}

bool String::equals(const String& v) const noexcept {
    return (_str == v._str);
}

bool String::equals(const char* v) const {
    return _str.compare(v) == 0;
}

int String::compareTo(const String& other) const {
    return _str.compare(other._str);
}

int String::compareTo(const char* other) const {
    return _str.compare(other);
}

String::value_type String::charAt(size_type index) const {
    // FIXME: Properly handle UTF-8 String
    return Char(_str[index]);
}

Optional<String::size_type> String::indexOf(const String& str, size_type fromIndex) const {
    const auto index = _str.find(str._str, fromIndex);

    return (index != std::string::npos)
            ? Optional<size_type>::of(index)
            : Optional<size_type>::none();
}

Optional<String::size_type> String::indexOf(const value_type& ch, size_type fromIndex) const {
    const auto index = _str.find(ch.getValue(), fromIndex);

    return (index != std::string::npos)
            ? Optional<size_type>::of(index)
            : Optional<size_type>::none();
}

Optional<String::size_type> String::indexOf(const char* str, size_type fromIndex) const {
    const auto index = _str.find(str, fromIndex);

    return (index != std::string::npos)
            ? Optional<size_type>::of(index)
            : Optional<size_type>::none();
}

Optional<String::size_type> String::lastIndexOf(const String& str, size_type fromIndex) const {
    const auto thisSize = size();

    std::string::size_type index = fromIndex;
    std::string::size_type lastIndex = std::string::npos;
    while ((index < thisSize) &&
           (index = _str.find(str._str, index)) != std::string::npos)
    {
        lastIndex = index;
        index += str._str.length();
    }

    return (lastIndex != std::string::npos)
            ? Optional<size_type>::of(lastIndex)
            : Optional<size_type>::none();
}

Optional<String::size_type> String::lastIndexOf(const value_type& ch, size_type fromIndex) const {
    const auto thisSize = size();

    const auto chSize = ch.getBytesCount();
    std::string::size_type index = fromIndex;
    std::string::size_type lastIndex = std::string::npos;
    while ((index < thisSize) &&
           (index = _str.find(ch.c_str(), index, chSize)) != std::string::npos) {
        lastIndex = index;
        index += chSize;
    }

    return (lastIndex != std::string::npos)
            ? Optional<size_type>::of(lastIndex)
            : Optional<size_type>::none();
}

Optional<String::size_type> String::lastIndexOf(const char* str, size_type fromIndex) const {
    const auto thisSize = size();
    const auto strLen = strlen(str);

    std::string::size_type index = fromIndex;
    auto lastIndex = std::string::npos;
    while ((index < thisSize) &&
           (index = _str.find(str, index)) != std::string::npos) {
        lastIndex = index;
        index += strLen;
    }

    return (lastIndex != std::string::npos)
            ? Optional<size_type>::of(lastIndex)
            : Optional<size_type>::none();
}

String String::concat(const String& str) const {
    return String(_str + str._str);
}

String String::concat(const char* str) const {
    return String(_str + str);
}

String String::replace(const value_type& what, const value_type& with) const {
    std::string subject(_str);
    std::string::size_type pos = 0;

    const Char::size_type whatSize = what.getBytesCount();
    const Char::size_type withSize = with.getBytesCount();

    while ((pos = subject.find(what.c_str(), pos, whatSize)) != std::string::npos) {
        subject.replace(pos, what.getBytesCount(), with.c_str());
        pos += withSize;
    }

    return subject;
}

String String::replace(const String& what, const String& by) const {
    std::string subject(_str);
    std::string::size_type pos = 0;

    while ((pos = subject.find(what._str, pos)) != std::string::npos) {
        subject.replace(pos, what._str.length(), by._str);
        pos += by._str.length();
    }

    return { subject };
}

Array<String> String::split(const String& expr) const {
    std::regex splitBy(expr._str);

    std::smatch m;
    std::string s = _str;

    std::vector<String> splits;
    while (std::regex_search(s, m, splitBy)) {
        splits.emplace_back(m.prefix());
        s = m.suffix().str();
    }

    if (m.ready()) {
        splits.emplace_back(s);
    }

    return splits.empty()
            ? Array<String>({*this})
            : Array<String>(splits);
}


String String::substring(size_type from, size_type len) const {
    return String(_str.substr(from, len));
}

String String::substring(size_type from) const {
    return String(_str.substr(from));
}


String String::trim() const {

    size_type firstNonWhitespace = 0;
    while (firstNonWhitespace < _str.size() &&
            Char::isWhitespace(_str[firstNonWhitespace])) {
        ++firstNonWhitespace;
    }

    // This string is whitespaces only
    if (firstNonWhitespace == _str.size())
        return Empty;

    size_type lastNonWhitespace = _str.size() - 1;
    while (lastNonWhitespace > firstNonWhitespace &&
           Char::isWhitespace(_str[lastNonWhitespace])) {
        --lastNonWhitespace;
    }

    return substring(firstNonWhitespace, lastNonWhitespace - firstNonWhitespace + 1);
}

String String::toLowerCase() const {
    std::string res;

    res.reserve(_str.size());
    for (auto c : _str) {
        auto lower = Char::toLower(c);
        res.push_back(lower.c_str()[0]);  // FIXME(abbyssoul): Non-UTF8 compatible code
    }

    return res;
}

String String::toUpperCase() const {
    std::string res;

    res.reserve(_str.size());
    for (auto c : _str) {
        auto lower = Char::toUpper(c);
        res.push_back(lower.c_str()[0]);  // FIXME(abbyssoul): Non-UTF8 compatible code
    }

    return res;
}

bool String::startsWith(const String& prefix) const {
    if (empty())
        return prefix.empty();

    if (prefix.empty())
        return empty();

    if (prefix.size() > size())
        return false;

    return std::strncmp(_str.data(), prefix._str.data(), prefix.size()) == 0;
}

bool String::startsWith(const value_type& prefix) const {
    return empty() ? false :
            Char::equals(prefix, _str[0]);
}

bool String::endsWith(const String& suffix) const {
    if (empty())
        return suffix.empty();

    if (suffix.empty())
        return empty();

    if (suffix.size() > size())
        return false;

    return std::strncmp(_str.data() + (_str.size() - suffix.size()), suffix._str.data(), suffix.size()) == 0;
}

bool String::endsWith(const value_type& suffix) const {
    return empty() ? false :
           Char::equals(suffix, _str[_str.size() - 1]);
}

uint64 String::hashCode() const {
    return std::hash<std::string>()(_str);
}

const char* String::c_str() const {
    return _str.c_str();
}

String String::join(const String& by, std::initializer_list<String> list) {
    std::string buffer;
    size_type total_size = 0;
    for (auto& s : list) {
        total_size += s.size();
    }

    if (list.size() > 1) {
        total_size += (list.size() - 1)*by.size();
    }
    buffer.reserve(total_size);

    size_type i = 0;
    for (auto& s : list) {
        buffer.append(s._str);
        if (++i < list.size())
            buffer.append(by._str);
    }

    return String(buffer);
}

/** Return jointed string from the given collection */
String
String::join(const StringView& by, const Array<String>& list) {
    std::string buffer;
    size_type total_size = 0;
    for (auto& s : list) {
        total_size += s.size();
    }

    if (list.size() > 1) {
        total_size += (list.size() - 1) * by.size();
    }
    buffer.reserve(total_size);

    size_type i = 0;
    for (auto& s : list) {
        buffer.append(s._str);
        if (++i < list.size()) {
            buffer.append(by.data(), by.size());
        }
    }

    return String(buffer);
}


//const String& String::forEach(const std::function<void(const value_type&)>& f) const {
//    for (auto c : _str) {
//        f(c);
//    }

//    return (*this);
//}


/** Return String representation of boolen value **/
String String::valueOf(bool value) {
    return (value)
            ? TRUE_STRING
            : FALSE_STRING;
}


String String::valueOf(String value) {
    return value;
}

String String::valueOf(int32 val) {
    return std::to_string(val);
}

String String::valueOf(int64 val) {
    return std::to_string(val);
}

String String::valueOf(uint32 val) {
    return std::to_string(val);
}

String String::valueOf(uint64 val) {
    return std::to_string(val);
}

String String::valueOf(float32 val) {
    return std::to_string(val);
}

String String::valueOf(float64 val) {
    return std::to_string(val);
}
