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
#include "solace/exception.hpp"


#include <cstring>  // memcpy


using namespace Solace;


ReadBuffer&
ReadBuffer::limit(size_type newLimit) {
    if (capacity() < newLimit) {
        raise<IllegalArgumentException>("newLimit");
    }

    _limit = newLimit;

    return *this;
}


ReadBuffer&
ReadBuffer::position(size_type newPosition) {
    if (limit() < newPosition) {
        raise<IllegalArgumentException>("newPosition");
    }

    _position = newPosition;

    return *this;
}


ReadBuffer&
ReadBuffer::advance(size_type increment) {
    if (remaining() < increment) {
        raise<IllegalArgumentException>("positionIncrement");
    }

    _position += increment;

    return *this;
}


byte
ReadBuffer::get() {
    if (remaining() < 1) {
        raise<OverflowException>(_position + 1, _position, _limit);
    }

    return _storage[_position++];
}

byte
ReadBuffer::get(size_type pos) const {
    if (limit() <= pos) {
        raise<IllegalArgumentException>("pos");
    }

    return _storage[pos];
}


ReadBuffer&
ReadBuffer::read(MemoryView& dest, size_type bytesToRead) {
    if (dest.size() < bytesToRead) {
        raise<IllegalArgumentException>("bytesToRead");
    }

    return read(dest.dataAddress(), bytesToRead);
}


ReadBuffer&
ReadBuffer::read(void* dest, size_type count) {
    if (remaining() < count) {
        raise<OverflowException>("Reading 'count' bytes will overflow",
                                 _position + count,
                                 _position,
                                 _limit);
    }

    const void* srcAddr = _storage.dataAddress(_position);
    memmove(dest, srcAddr, count);
    _position += count;

    return (*this);
}


const ReadBuffer&
ReadBuffer::read(size_type offset, byte* dest, size_type bytesToRead) const {
    if (_limit < (offset + bytesToRead)) {
        raise<OverflowException>(offset + bytesToRead, 0, _limit);
    }

    const void* srcAddr = _storage.dataAddress(offset);
    memmove(dest, srcAddr, bytesToRead);

    return (*this);
}


const ReadBuffer&
ReadBuffer::read(size_type offset, MemoryView& dest, size_type bytesToRead) const {
    if (dest.size() < bytesToRead) {
        raise<IllegalArgumentException>("bytesToRead");
    }

    return read(offset, dest.dataAddress(), bytesToRead);
}


ReadBuffer& ReadBuffer::readLE(uint16& value) {
    constexpr auto valueSize = sizeof(value);
    read(&value, valueSize);

    if (isBigendian()) {
        auto result = value;
        const byte* v = reinterpret_cast<byte*>(&result);
        byte* const p = reinterpret_cast<byte*>(&value);
        p[0] = v[valueSize - 1];
        p[1] = v[valueSize - 2];
    }

    return (*this);
}

ReadBuffer& ReadBuffer::readLE(uint32& value) {
    constexpr auto valueSize = sizeof(value);
    read(&value, valueSize);

    if (isBigendian()) {
        auto result = value;
        const byte* v = reinterpret_cast<byte*>(&result);
        byte* const p = reinterpret_cast<byte*>(&value);
        p[0] = v[valueSize - 1];
        p[1] = v[valueSize - 2];
        p[2] = v[valueSize - 3];
        p[3] = v[valueSize - 4];
    }

    return (*this);
}

ReadBuffer& ReadBuffer::readLE(uint64& value) {
    constexpr auto valueSize = sizeof(value);
    read(&value, valueSize);

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

    return (*this);
}


ReadBuffer& ReadBuffer::readBE(uint16& value) {
    constexpr auto valueSize = sizeof(value);
    read(&value, valueSize);

    if (!isBigendian()) {
        auto result = value;
        const byte* v = reinterpret_cast<byte*>(&result);
        byte* const p = reinterpret_cast<byte*>(&value);
        p[0] = v[valueSize - 1];
        p[1] = v[valueSize - 2];
    }

    return (*this);
}

ReadBuffer& ReadBuffer::readBE(uint32& value) {
    constexpr auto valueSize = sizeof(value);
    read(&value, valueSize);

    if (!isBigendian()) {
        auto result = value;
        const byte* v = reinterpret_cast<byte*>(&result);
        byte* const p = reinterpret_cast<byte*>(&value);
        p[0] = v[valueSize - 1];
        p[1] = v[valueSize - 2];
        p[2] = v[valueSize - 3];
        p[3] = v[valueSize - 4];
    }

    return (*this);
}

ReadBuffer& ReadBuffer::readBE(uint64& value) {
    constexpr auto valueSize = sizeof(value);
    read(&value, valueSize);

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

    return (*this);
}
