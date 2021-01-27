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
 *	@file		byteWriter.cpp
 *	@brief		Implementation of ByteWriter
 ******************************************************************************/
#include "solace/byteWriter.hpp"
#include "solace/posixErrorDomain.hpp"

#include <cstring>  // memmove


using namespace Solace;


Result<void, Error>
ByteWriter::limit(size_type newLimit) noexcept {
    if (capacity() < newLimit) {
		return makeError(SystemErrors::Overflow, "ByteWriter::limit()");
    }

    _limit = newLimit;

    return Ok();
}


Result<void, Error>
ByteWriter::position(size_type newPosition) noexcept {
    if (limit() < newPosition) {
		return makeError(SystemErrors::Overflow, "ByteWriter::position()");
    }

    _position = newPosition;

    return Ok();
}


Result<void, Error>
ByteWriter::advance(size_type increment) noexcept {
    if (remaining() < increment) {
		return makeError(SystemErrors::Overflow, "ByteWriter::advance()");
    }

    _position += increment;

    return Ok();
}


Result<void, Error>
ByteWriter::write(MemoryView::MemoryAddress srcAddr, size_type count) noexcept {
    if (count == 0) {  // It's always ok to write 0 bytes :)
        return Ok();
    }

	auto const pos = position();
	auto destView = _storage.view().slice(pos, pos + count);
	if (destView.size() < count) {  // Check we have enough space in the resulting slice
		return makeError(SystemErrors::Overflow, "ByteWriter::write()");
	}

	std::memmove(destView.dataAddress(), srcAddr, count);

    return advance(count);
}


Result<void, Error>
ByteWriter::writeLE(uint16 value) noexcept {
    constexpr auto valueSize = sizeof(value);
    auto result = value;

    if (isBigendian()) {
        const byte* v = reinterpret_cast<byte*>(&value);
        byte* const p = reinterpret_cast<byte*>(&result);
        p[0] = v[valueSize - 1];
        p[1] = v[valueSize - 2];
    }

    return write(&result, valueSize);
}


Result<void, Error>
ByteWriter::writeLE(uint32 value) noexcept {
    constexpr auto valueSize = sizeof(value);
    auto result = value;

    if (isBigendian()) {
        const byte* v = reinterpret_cast<byte*>(&value);
        byte* const p = reinterpret_cast<byte*>(&result);
        p[0] = v[valueSize - 1];
        p[1] = v[valueSize - 2];
        p[2] = v[valueSize - 3];
        p[3] = v[valueSize - 4];
    }

    return write(&result, valueSize);
}


Result<void, Error>
ByteWriter::writeLE(uint64 value) noexcept {
    constexpr auto valueSize = sizeof(value);
    auto result = value;

    if (isBigendian()) {
        const byte* v = reinterpret_cast<byte*>(&value);
        byte* const p = reinterpret_cast<byte*>(&result);
        p[0] = v[valueSize - 1];
        p[1] = v[valueSize - 2];
        p[2] = v[valueSize - 3];
        p[3] = v[valueSize - 4];
        p[4] = v[valueSize - 5];
        p[5] = v[valueSize - 6];
        p[6] = v[valueSize - 7];
        p[7] = v[valueSize - 8];
    }

    return write(&result, valueSize);
}


Result<void, Error>
ByteWriter::writeBE(uint16 value) noexcept {
    constexpr auto valueSize = sizeof(value);
    auto result = value;

    if (!isBigendian()) {
        const byte* v = reinterpret_cast<byte*>(&value);
        byte* const p = reinterpret_cast<byte*>(&result);
        p[0] = v[1];
        p[1] = v[0];
    }

    return write(&result, valueSize);
}


Result<void, Error>
ByteWriter::writeBE(uint32 value) noexcept {
    constexpr auto valueSize = sizeof(value);
    auto result = value;

    if (!isBigendian()) {
        const byte* v = reinterpret_cast<byte*>(&value);
        byte* const p = reinterpret_cast<byte*>(&result);
        p[0] = v[valueSize - 1];
        p[1] = v[valueSize - 2];
        p[2] = v[valueSize - 3];
        p[3] = v[valueSize - 4];
    }

    return write(&result, valueSize);
}


Result<void, Error>
ByteWriter::writeBE(uint64 value) noexcept {
    constexpr auto valueSize = sizeof(value);
    auto result = value;

    if (!isBigendian()) {
        const byte* v = reinterpret_cast<byte*>(&value);
        byte* const p = reinterpret_cast<byte*>(&result);
        p[0] = v[valueSize - 1];
        p[1] = v[valueSize - 2];
        p[2] = v[valueSize - 3];
        p[3] = v[valueSize - 4];
        p[4] = v[valueSize - 5];
        p[5] = v[valueSize - 6];
        p[6] = v[valueSize - 7];
        p[7] = v[valueSize - 8];
    }

    return write(&result, valueSize);
}
