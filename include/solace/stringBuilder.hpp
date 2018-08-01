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
 * libSolace: StringBuilder class
 *	@file		solace/stringBuilder.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	@brief		Defines StringBuilder
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_STRINGBUILDER_HPP
#define SOLACE_STRINGBUILDER_HPP


#include "solace/byteWriter.hpp"
#include "solace/string.hpp"

#include "solace/traits/iformattable.hpp"


namespace Solace {

/**
 * String Builder to build a string if you really need mutable strings
 * FIXME(abbyssoul): This is not yet implemented.
 */
class StringBuilder {
public:
    using size_type = String::size_type;

public:

     ~StringBuilder() = default;

    /** Initialize a new instance of StringBuilder with a given storage */
    StringBuilder(MutableMemoryView&& buffer) :
        _buffer(std::move(buffer))
    {}

    /** Initialize a new instance of StringBuilder with a given storage */
    StringBuilder(MemoryBuffer&& buffer) :
        _buffer(std::move(buffer))
    {}

    /** Initialize a new instance of StringBuilder with a given storage and initial string value.*/
    StringBuilder(MutableMemoryView&& buffer, StringView str);

    /** Initialize a new instance of StringBuilder with a given storage and initial string value.*/
    StringBuilder(MemoryBuffer&& buffer, StringView str);

    //!< Move construct string builder
    StringBuilder(const StringBuilder&) = delete;

    //!< Move construct string builder instance.
    StringBuilder(StringBuilder&& s) noexcept = default;

    StringBuilder& operator= (StringBuilder&& rhs) noexcept {
        return swap(rhs);
    }

    StringBuilder& operator= (const StringBuilder&) = delete;

    StringBuilder& swap(StringBuilder& rhs) noexcept {
        using std::swap;

        swap(_buffer, rhs._buffer);

        return *this;
    }


public:

	StringBuilder& append(char c);
    StringBuilder& append(Char c);
    StringBuilder& append(StringView str);

    StringBuilder& append(String const& str) {
        return append(str.view());
    }

    StringBuilder& append(IFormattable const& str) {
        return append(str.toString());
    }

    StringBuilder& appendFormat(StringView fmt) { return append(fmt); }

//        template<typename T>
//        StringBuilder& appendFormat(const String& fmt, T&& value) {
//            return append(Format(fmt).formatArg(std::forward<T>(value)));
//        }

//        template<typename T>
//        StringBuilder& appendFormat(char* fmt, T&& value) {
//            return append(Format(fmt).formatArg(std::forward<T>(value)));
//        }

//        template<typename T>
//        StringBuilder& appendFormat(char16_t* fmt, T&& value) {
//            return append(Format(fmt).formatArg(std::forward<T>(value)));
//        }

//        template<typename T>
//        StringBuilder& appendFormat(char32_t* fmt, T&& value) {
//            return append(Format(fmt).formatArg(std::forward<T>(value)));
//        }

//        template<typename T, typename... Args>
//        StringBuilder& appendFormat(const String& fmt, T&& value, Args... args) {
//            return append(Format(fmt).formatArg(std::forward<T>(value), args...));
//        }

//        template<typename T, typename... Args>
//        StringBuilder& appendFormat(const char* fmt, T&& value, Args... args) {
//            return append(Format(fmt).formatArg(std::forward<T>(value), args...));
//        }

//        template<typename T, typename... Args>
//        StringBuilder& appendFormat(const char16_t* fmt, T&& value, Args... args) {
//            return append(Format(fmt).formatArg(std::forward<T>(value), args...));
//        }

//        template<typename T, typename... Args>
//        StringBuilder& appendFormat(const char32_t* fmt, T&& value, Args... args) {
//            return append(Format(fmt).formatArg(std::forward<T>(value), args...));
//        }

    String substring(size_type from, size_type to) const;
	StringBuilder& clear();
	StringBuilder& erase(size_type position, size_type count = 1);
	StringBuilder& operator= (const String& str);
	Char operator[] (size_type index) const;

    Optional<size_type> indexOf(const String& str, size_type fromIndex = 0) const;
    Optional<size_type> indexOf(const Char& ch, size_type fromIndex = 0) const;
    Optional<size_type> indexOf(const char* str, size_type fromIndex = 0) const;
    Optional<size_type> lastIndexOf(const String& str, size_type fromIndex = 0) const;
    Optional<size_type> lastIndexOf(const Char& ch, size_type fromIndex = 0) const;
    Optional<size_type> lastIndexOf(const char* str, size_type fromIndex = 0) const;

    size_type replace(const Char& what, const Char& with);
	size_type replace(const String& what, const String& by);

    size_type length() const noexcept;
	bool empty() const;

    StringView view() const noexcept;

    /** Get resulting string */
    String toString() const;

private:

    ByteWriter  _buffer;
};


inline void swap(StringBuilder& lhs, StringBuilder& rhs) noexcept {
    lhs.swap(rhs);
}

}  // namespace Solace
#endif  // SOLACE_STRINGBUILDER_HPP
