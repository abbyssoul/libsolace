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
 *	@file		byteReader.cpp
 *	@brief		Implementation of Byte Reader
 ******************************************************************************/
#include "solace/byteReader.hpp"
#include "solace/posixErrorDomain.hpp"

#include <cstring>  // memmove

using namespace Solace;


Result<void, Error>
ByteReader::limit(size_type newLimit) noexcept {
    if (capacity() < newLimit) {
		return makeError(SystemErrors::Overflow, "ByteReader::limit()");
    }

    _limit = newLimit;

    return Ok();
}


Result<void, Error>
ByteReader::position(size_type newPosition) noexcept {
    if (limit() < newPosition) {
		return makeError(SystemErrors::Overflow, "ByteReader::position()");
    }

    _position = newPosition;

    return Ok();
}


Result<void, Error>
ByteReader::advance(size_type increment) noexcept {
    if (remaining() < increment) {
		return makeError(SystemErrors::Overflow, "ByteReader::advance()");
    }

    _position += increment;

    return Ok();
}


Result<byte, Error>
ByteReader::get() noexcept {
    if (remaining() < 1) {
		return makeError(SystemErrors::Overflow, "ByteReader::get()");
    }

    return Ok(_storage.view()[_position++]);
}

Result<byte, Error>
ByteReader::get(size_type pos) const noexcept {
    if (limit() <= pos) {
		return makeError(SystemErrors::Overflow, "ByteReader::get(position)");
    }

    return Ok(_storage.view()[pos]);
}


Result<void, Error>
ByteReader::read(MutableMemoryView dest, size_type bytesToRead) noexcept {
    if (dest.size() < bytesToRead) {
		return makeError(SystemErrors::Overflow, "ByteReader::read()");
    }

    return read(dest.dataAddress(), bytesToRead);
}


Result<void, Error>
ByteReader::read(void* dest, size_type bytesToRead) noexcept {
    if (remaining() < bytesToRead) {
		return makeError(SystemErrors::Overflow, "ByteReader::read()");
    }

    const void* srcAddr = _storage.view().dataAddress(_position);
    memmove(dest, srcAddr, bytesToRead);
    _position += bytesToRead;

    return Ok();
}


Result<void, Error>
ByteReader::read(size_type offset, MutableMemoryView dest, size_type bytesToRead) const noexcept {
    if (dest.size() < bytesToRead) {
		return makeError(SystemErrors::Overflow, "ByteReader::read()");
    }

    if (limit() < (offset + bytesToRead)) {
		return makeError(SystemErrors::Overflow, "ByteReader::read()");
    }

    const void* srcAddr = _storage.view().dataAddress(offset);
    memmove(dest.dataAddress(), srcAddr, bytesToRead);

    return Ok();
}


Result<void, Error>
ByteReader::readLE(uint16& value) noexcept {
    constexpr auto valueSize = sizeof(value);
    return read(&value, valueSize)
            .then([&]() {
                if (isBigendian()) {
                    auto result = value;
                    byte const* v = reinterpret_cast<byte*>(&result);
                    byte* const p = reinterpret_cast<byte*>(&value);
                    p[0] = v[valueSize - 1];
                    p[1] = v[valueSize - 2];
                }
            });
}

Result<void, Error>
ByteReader::readLE(uint32& value) noexcept {
    constexpr auto valueSize = sizeof(value);
    return read(&value, valueSize)
            .then([&]() {
                if (isBigendian()) {
                    auto result = value;
                    const byte* v = reinterpret_cast<byte*>(&result);
                    byte* const p = reinterpret_cast<byte*>(&value);
                    p[0] = v[valueSize - 1];
                    p[1] = v[valueSize - 2];
                    p[2] = v[valueSize - 3];
                    p[3] = v[valueSize - 4];
                }
            });
}


Result<void, Error>
ByteReader::readLE(uint64& value) noexcept {
    constexpr auto valueSize = sizeof(value);
    return read(&value, valueSize)
            .then([&]() {
                if (isBigendian()) {
                    auto result = value;
                    const byte* v = reinterpret_cast<byte*>(&result);
                    byte* const p = reinterpret_cast<byte*>(&value);
                    p[0] = v[valueSize - 1];
                    p[1] = v[valueSize - 2];
                    p[2] = v[valueSize - 3];
                    p[3] = v[valueSize - 4];
                    p[4] = v[valueSize - 5];
                    p[5] = v[valueSize - 6];
                    p[6] = v[valueSize - 7];
                    p[7] = v[valueSize - 8];
                }
            });
}


Result<void, Error>
ByteReader::readBE(uint16& value) noexcept {
    constexpr auto valueSize = sizeof(value);
    return read(&value, valueSize)
            .then([&]() {
                if (!isBigendian()) {
                    auto result = value;
                    const byte* v = reinterpret_cast<byte*>(&result);
                    byte* const p = reinterpret_cast<byte*>(&value);
                    p[0] = v[valueSize - 1];
                    p[1] = v[valueSize - 2];
                }
            });
}

Result<void, Error>
ByteReader::readBE(uint32& value) noexcept {
    constexpr auto valueSize = sizeof(value);
    return read(&value, valueSize)
            .then([&]() {
                if (!isBigendian()) {
                    auto result = value;
                    const byte* v = reinterpret_cast<byte*>(&result);
                    byte* const p = reinterpret_cast<byte*>(&value);
                    p[0] = v[valueSize - 1];
                    p[1] = v[valueSize - 2];
                    p[2] = v[valueSize - 3];
                    p[3] = v[valueSize - 4];
                }
    });
}

Result<void, Error>
ByteReader::readBE(uint64& value) noexcept {
    constexpr auto valueSize = sizeof(value);
    return read(&value, valueSize)
            .then([&]() {
                if (!isBigendian()) {
                    auto result = value;
                    const byte* v = reinterpret_cast<byte*>(&result);
                    byte* const p = reinterpret_cast<byte*>(&value);
                    p[0] = v[valueSize - 1];
                    p[1] = v[valueSize - 2];
                    p[2] = v[valueSize - 3];
                    p[3] = v[valueSize - 4];
                    p[4] = v[valueSize - 5];
                    p[5] = v[valueSize - 6];
                    p[6] = v[valueSize - 7];
                    p[7] = v[valueSize - 8];
                }
    });
}
