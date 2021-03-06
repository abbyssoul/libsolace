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
 *	@file		stringBuilder.cpp
 *	@brief		Implementation of StringBuilder
 ******************************************************************************/
#include "solace/stringBuilder.hpp"
#include "solace/byteReader.hpp"

#include <cstdio>  // snprintf etc
#include <inttypes.h>


using namespace Solace;

StringBuilder::size_type
nonNegative(int bytesWritten) noexcept {
	return (bytesWritten > 0)
			? narrow_cast<StringBuilder::size_type>(bytesWritten)
			: 0;
}

void maybeAdvance(ByteWriter& dest, MemoryView::size_type capacity, int bytesWritten) {
	if (bytesWritten > 0 &&
		static_cast<MemoryView::size_type>(bytesWritten) < capacity) {
		dest.advance(static_cast<ByteWriter::size_type>(bytesWritten));
	}
}

template<typename T>
void safeFormat(ByteWriter& dest, char const* fmt, T value) {
	auto buffer = dest.viewRemaining();
	auto const capacity = buffer.size();
	auto const bytesWritten = snprintf(buffer.begin(), capacity, fmt, value);

	if (bytesWritten > 0 &&
		bytesWritten < capacity) {
		dest.advance(static_cast<ByteWriter::size_type>(bytesWritten));
	}
}

StringBuilder::size_type
StringBuilder::measureFormatted(uint16 value) noexcept { return nonNegative(snprintf(nullptr, 0, "%" PRIu16, value)); }
StringBuilder::size_type
StringBuilder::measureFormatted(uint32 value) noexcept { return nonNegative(snprintf(nullptr, 0, "%" PRIu32, value)); }
StringBuilder::size_type
StringBuilder::measureFormatted(uint64 value) noexcept { return nonNegative(snprintf(nullptr, 0, "%" PRIu64, value)); }

StringBuilder::size_type
StringBuilder::measureFormatted(int16 value) noexcept { return nonNegative(snprintf(nullptr, 0, "%" PRId16, value)); }
StringBuilder::size_type
StringBuilder::measureFormatted(int32 value) noexcept { return nonNegative(snprintf(nullptr, 0, "%" PRId32, value)); }
StringBuilder::size_type
StringBuilder::measureFormatted(int64 value) noexcept {	return nonNegative(snprintf(nullptr, 0, "%" PRId64, value));
}



StringBuilder::StringBuilder(MutableMemoryView&& buffer, StringView str)
	: StringBuilder{mv(buffer)}
{
    _buffer.write(str.view());
}

StringBuilder::StringBuilder(MemoryResource&& buffer, StringView str)
	: StringBuilder{mv(buffer)}
{
    _buffer.write(str.view());
}

StringBuilder& StringBuilder::append(char c) {
    _buffer.write(c);

	return *this;
}

StringBuilder& StringBuilder::append(Char c) {
	return append(StringView{c.c_str(), c.getBytesCount()});
}

StringBuilder& StringBuilder::append(StringView cstr) {
    _buffer.write(cstr.view());

	return *this;
}


StringBuilder&
StringBuilder::append(uint16 value) {
	auto buffer = _buffer.viewRemaining();
	auto const capacity = buffer.size();
	auto pos = static_cast<char*>(buffer.dataAddress());
	maybeAdvance(_buffer, capacity, snprintf(pos, capacity, "%" PRIu16, value));

	return *this;
}

StringBuilder&
StringBuilder::append(uint32 value) {
	auto buffer = _buffer.viewRemaining();
	auto const capacity = buffer.size();
	auto pos = static_cast<char*>(buffer.dataAddress());
	maybeAdvance(_buffer, capacity, snprintf(pos, capacity, "%" PRIu32, value));

	return *this;
}

StringBuilder&
StringBuilder::append(uint64 value) {
	auto buffer = _buffer.viewRemaining();
	auto const capacity = buffer.size();
	auto pos = static_cast<char*>(buffer.dataAddress());
	maybeAdvance(_buffer, capacity, snprintf(pos, capacity, "%" PRIu64, value));

	return *this;
}


StringView
StringBuilder::view() const noexcept {
	return StringView{_buffer.viewWritten()};
}


bool StringBuilder::empty() const {
    return (_buffer.position() == 0);
}

StringBuilder::size_type
StringBuilder::length() const noexcept {
    return _buffer.position();
}


StringView
StringBuilder::substring(size_type from, size_type to) const {
    auto const data = _buffer.viewWritten();
	return data.slice(from, to);
}


Optional<StringBuilder::size_type>
StringBuilder::indexOf(Char const& ch, size_type fromIndex) const {
    // TODO(abbyssoul): Check for index out of range

    MutableMemoryView::value_type buffer[Char::max_bytes];
    auto b = wrapMemory(buffer);

    ByteReader reader(_buffer.viewWritten());
    for (size_type i = fromIndex; i + ch.getBytesCount() < reader.limit(); ++i) {
		b.fill(0);
		reader.read(i, b.slice(0, ch.getBytesCount()));

        if (ch.equals(b)) {
            return Optional<size_type>(i);
        }
    }

    return none;
}


String
StringBuilder::build() {
	auto const resultStringSize = length();  // Note we need to store size temporerely as buffer gets moved out.
	return { _buffer.moveResource(), resultStringSize };
}
