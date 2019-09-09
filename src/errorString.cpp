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
 *	@file		errorString.cpp
 *	@brief		Implementation details of ErrorString
 ******************************************************************************/

#include "solace/details/string_utils.hpp"

#include <cstdlib>
#include <cstring>
#include <cstdio>  // snprintf etc
#include <inttypes.h>  // Platform-independent format

#include <algorithm>  // std::min


namespace Solace {
namespace details {

ErrorString::~ErrorString() noexcept {
	if (_message) {
		free(const_cast<char*>(_message));
		_message = nullptr;
	}
}



ErrorString::ErrorString(StringView view)
	: _size{view.size()}
	, _message{static_cast<value_type*>(calloc(view.size() + 1, 1))}
{
	memcpy(_message, view.data(), _size);
}



StringWriter::size_type
StringWriter::measure(uint32 value) noexcept { return narrow_cast<size_type>(snprintf(nullptr, 0, "%" PRIu32, value)); }
StringWriter::size_type
StringWriter::measure(uint64 value) noexcept { return narrow_cast<size_type>(snprintf(nullptr, 0, "%" PRIu64, value)); }

StringWriter::size_type
StringWriter::measure(int32 value) noexcept { return narrow_cast<size_type>(snprintf(nullptr, 0, "%" PRId32, value)); }
StringWriter::size_type
StringWriter::measure(int64 value) noexcept { return narrow_cast<size_type>(snprintf(nullptr, 0, "%" PRId64, value)); }



StringWriter::StringWriter(size_type memSize) noexcept
	: _size{memSize}
	, _offset{0}
	, _buffer{static_cast<details::ErrorString::value_type*>(calloc(memSize + 1, 1))}
{
	if (!_buffer) {
		_size = 0;
	}
}

StringWriter&
StringWriter::append(StringView data) noexcept {
	size_type const dataCopied = std::min(data.size(), remaining());
	memcpy(currentBuffer(), data.data(), dataCopied);
	_offset += dataCopied;

	return *this;
}

StringWriter&
StringWriter::appendFormated(uint32 value) noexcept {
	_offset += snprintf(currentBuffer(), remaining(), "%" PRIu32, value); return *this;
}

StringWriter&
StringWriter::appendFormated(int32 value) noexcept {
	_offset += snprintf(currentBuffer(), remaining(), "%" PRId32, value); return *this;
}

StringWriter&
StringWriter::appendFormated(uint64 value) noexcept {
	_offset += snprintf(currentBuffer(), remaining(), "%" PRIu64, value);
	return *this;
}

StringWriter&
StringWriter::appendFormated(int64 value) noexcept {
	_offset += snprintf(currentBuffer(), remaining(), "%" PRId64, value);
	return *this;
}

StringWriter&
StringWriter::append(const char* value) noexcept {
	_offset += snprintf(currentBuffer(), remaining(), "%s", value); return *this;
}

}  // namespace details
}  // namespace Solace
