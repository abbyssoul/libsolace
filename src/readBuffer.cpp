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
 *	@file		readBuffer.cpp
 *	@brief		Implementation of ReadBuffer
 ******************************************************************************/
#include "solace/readBuffer.hpp"


#include <cstring>  // memcpy


using namespace Solace;


Result<void, Error>
ReadBuffer::limit(size_type newLimit) {
    if (capacity() < newLimit) {
        return Err<Error>(String("OverflowError: limit(): new limit is greater then capacity."));
    }

    _limit = newLimit;

    return Ok();
}


Result<void, Error>
ReadBuffer::position(size_type newPosition) {
    if (limit() < newPosition) {
        return Err<Error>(String("OverflowError: position(): value pass the buffer end."));
    }

    _position = newPosition;

    return Ok();
}


Result<void, Error>
ReadBuffer::advance(size_type increment) {
    if (remaining() < increment) {
        return Err<Error>(String("OverflowError: advance(): move pass the buffer end."));
    }

    _position += increment;

    return Ok();
}


Result<byte, Error>
ReadBuffer::get() {
    if (remaining() < 1) {
        return Err<Error>(String("OverflowError: get(): no data remained in the buffer"));
    }

    return Ok(_storage.view()[_position++]);
}

Result<byte, Error>
ReadBuffer::get(size_type pos) const {
    if (limit() <= pos) {
        return Err<Error>(String("OverflowError: get(pos): offset outside of the buffer"));
    }

    return Ok(_storage.view()[pos]);
}


Result<void, Error>
ReadBuffer::read(MemoryView& dest, size_type bytesToRead) {
    if (dest.size() < bytesToRead) {
        return Err<Error>(String("OverflowError: read(dest, size): destination buffer is too small"));
    }

    return read(dest.dataAddress(), bytesToRead);
}


Result<void, Error>
ReadBuffer::read(void* dest, size_type bytesToRead) {
    if (remaining() < bytesToRead) {
        return Err<Error>(String("UnderflowError: read(dest, size): not enough data in the buffer"));
    }

    const void* srcAddr = _storage.view().dataAddress(_position);
    memmove(dest, srcAddr, bytesToRead);
    _position += bytesToRead;

    return Ok();
}


Result<void, Error>
ReadBuffer::read(size_type offset, MemoryView& dest, size_type bytesToRead) const {
    if (dest.size() < bytesToRead) {
        return Err<Error>(String("OverflowError: read(dest, size): destination buffer is too small"));
    }

    if (limit() < (offset + bytesToRead)) {
        return Err<Error>(String("Overflow Error: byte to read"));
    }

    const void* srcAddr = _storage.view().dataAddress(offset);
    memmove(dest.dataAddress(), srcAddr, bytesToRead);

    return Ok();
}


Result<void, Error>
ReadBuffer::readLE(uint16& value) {
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
ReadBuffer::readLE(uint32& value) {
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
ReadBuffer::readLE(uint64& value) {
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
ReadBuffer::readBE(uint16& value) {
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
ReadBuffer::readBE(uint32& value) {
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
ReadBuffer::readBE(uint64& value) {
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
