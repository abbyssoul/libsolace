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


#include "solace/byteBuffer.hpp"
#include "solace/string.hpp"


namespace Solace {

/**
 * String Builder to build a string if you really need mutable strings
 */
class StringBuilder : public IFormattable {
public:
    typedef String::size_type	size_type;

public:

    /** Initialize a new instance of StringBuilder with a given storage */
    StringBuilder(const ByteBuffer& buffer);

    /** New StringBuilder with the given storage and initial content */
    StringBuilder(const ByteBuffer& buffer, const char* str);

    /** New StringBuilder with the given storage and initial content */
    StringBuilder(const ByteBuffer& buffer, const String& str);

    /** New StringBuilder with the given storage and initial content */
    StringBuilder(const ByteBuffer& buffer, std::initializer_list<String> s);

    //!< Move construct string builder
    StringBuilder(StringBuilder&& s);

	virtual ~StringBuilder() = default;

public:

	StringBuilder& append(char c);
    StringBuilder& append(const char* c);
    StringBuilder& append(const Char& c);
    StringBuilder& append(const IFormattable& f);
	StringBuilder& append(const String& str);

    StringBuilder& appendFormat(const char* fmt) {	return append(fmt); }
	StringBuilder& appendFormat(const String& fmt) { return append(fmt); }

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

	bool endsWith(const String& suffix) const;
	bool endsWith(const Char& suffix) const;

	size_type length() const;
	bool empty() const;

    /** From IFromattable */
	String toString() const override;

private:

    ByteBuffer  _buffer;
};

}  // namespace Solace
#endif  // SOLACE_STRINGBUILDER_HPP
