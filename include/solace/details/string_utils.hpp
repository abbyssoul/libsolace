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
 *	@file		solace/details/string_utils.hpp
 *  @brief		Implemenetation details for error string.
 * Note: Not to be included directly.
 ******************************************************************************/
#pragma once
#ifndef SOLACE_DETAILS_STRING_UTILS_HPP
#define SOLACE_DETAILS_STRING_UTILS_HPP

#include "solace/stringView.hpp"

namespace Solace {
namespace details {

/**
 * A short and limited string to be used in error handling.
 * Short string owns the memory it is given.
 */
struct ErrorString {

	using value_type = StringView::value_type;
	using size_type = StringView::size_type;

	~ErrorString() noexcept;

	ErrorString(ErrorString&& rhs) noexcept
		: _size{rhs.size()}
		, _message{exchange(rhs._message, nullptr)}
	{}

	ErrorString& operator= (ErrorString&& rhs) noexcept {
		_size = exchange(rhs._size, 0);
		_message = exchange(rhs._message, nullptr);

		return *this;
	}

	ErrorString(StringView view);

	ErrorString(char* data, size_type size) noexcept
		: _size{size}
		, _message{data}
	{}

	size_type size() const noexcept { return _size; }

	value_type const* c_str() const noexcept { return _message; }
	StringView view() const noexcept { return {_message, _size}; }

private:

	size_type		_size;
	// Owned memory
	value_type*		_message;
};



struct StringWriter {
	using size_type = details::ErrorString::size_type;
	using value_type = details::ErrorString::value_type;

	static size_type measure(StringView value) noexcept { return value.size(); }

	static size_type measure(int32 value) noexcept;
	static size_type measure(int64 value) noexcept;

	static size_type measure(uint32 value) noexcept;
	static size_type measure(uint64 value) noexcept;

	template<size_t N>
	static constexpr size_type measure(char const (&SOLACE_UNUSED(str))[N]) noexcept {  // NOLINT(whitespace/parens)
		return N;
	}


	StringWriter(size_type memSize) noexcept;

	StringWriter& append(StringView data) noexcept;

	StringWriter& appendFormated(uint32 value) noexcept;
	StringWriter& appendFormated(int32 value) noexcept;
	StringWriter& appendFormated(uint64 value) noexcept;
	StringWriter& appendFormated(int64 value) noexcept;

	StringWriter& append(const char* value) noexcept;


	size_type remaining() const noexcept { return _size - _offset; }
	value_type* currentBuffer() const noexcept { return _buffer + _offset; }

	details::ErrorString build() noexcept {
		_size = 0;
		return {exchange(_buffer, nullptr), exchange(_offset, 0)};
	}

	size_type		_size;
	size_type		_offset;
	value_type*		_buffer;
};
}  // namespace details
}  // namespace Solace
#endif  // SOLACE_DETAILS_STRING_UTILS_HPP
