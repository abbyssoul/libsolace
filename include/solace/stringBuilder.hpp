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
 ******************************************************************************/
#pragma once
#ifndef SOLACE_STRINGBUILDER_HPP
#define SOLACE_STRINGBUILDER_HPP


#include "solace/byteWriter.hpp"
#include "solace/string.hpp"


namespace Solace {

/**
 * String Builder to build a string if you really need mutable strings
 * FIXME(abbyssoul): This is not yet implemented.
 */
class StringBuilder {
public:
    using size_type = String::size_type;

	static size_type measure(StringView::value_type) noexcept { return 1; }
	static size_type measure(StringView value) noexcept { return value.size(); }

	static size_type measureFormatted(uint16) noexcept;
	static size_type measureFormatted(uint32) noexcept;
	static size_type measureFormatted(uint64) noexcept;
	static size_type measureFormatted(int16) noexcept;
	static size_type measureFormatted(int32) noexcept;
	static size_type measureFormatted(int64) noexcept;

	template<size_t N>
	static constexpr size_type measure(char const (&SOLACE_UNUSED(str))[N]) noexcept {
		return N;
	}


public:
    //!< Not copy-able
    StringBuilder(StringBuilder const&) = delete;
    StringBuilder& operator= (const StringBuilder&) = delete;

    /** Initialize a new instance of StringBuilder with a given storage */
    StringBuilder(MutableMemoryView&& buffer) noexcept
        : _buffer{std::move(buffer)}
    {}

    /** Initialize a new instance of StringBuilder with a given storage */
    StringBuilder(MemoryResource&& buffer) noexcept
        : _buffer{std::move(buffer)}
    {}

    /** Initialize a new instance of StringBuilder with a given storage and initial string value.*/
    StringBuilder(MutableMemoryView&& buffer, StringView str);

    /** Initialize a new instance of StringBuilder with a given storage and initial string value.*/
    StringBuilder(MemoryResource&& buffer, StringView str);


    //!< Move construct string builder instance.
    StringBuilder(StringBuilder&& s) noexcept = default;

    StringBuilder& operator= (StringBuilder&& rhs) noexcept {
        return swap(rhs);
    }


    StringBuilder& swap(StringBuilder& rhs) noexcept {
        using std::swap;

        swap(_buffer, rhs._buffer);

        return *this;
    }


public:

	StringBuilder& append(char c);
    StringBuilder& append(Char c);
	StringBuilder& append(uint16 value);
	StringBuilder& append(uint32 value);
	StringBuilder& append(uint64 value);

	StringBuilder& append(StringView str);
    StringBuilder& append(String const& str) {
        return append(str.view());
    }

    StringBuilder& appendFormat(StringView fmt) { return append(fmt); }


	StringView substring(size_type from, size_type to) const;
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

	bool empty() const;
    size_type length() const noexcept;
	size_type remaining() const noexcept {
		return narrow_cast<size_type>(_buffer.remaining());
	}

    StringView view() const noexcept;

	/** View resulting string */
	StringView toString() const {
		return view();
	}

	String build();

private:

    ByteWriter  _buffer;
};


inline void swap(StringBuilder& lhs, StringBuilder& rhs) noexcept {
    lhs.swap(rhs);
}

}  // namespace Solace
#endif  // SOLACE_STRINGBUILDER_HPP
