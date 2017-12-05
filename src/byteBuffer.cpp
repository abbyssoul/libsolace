/*
*  Copyright 2017 Ivan Ryabov
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
 *	@file		byteBuffer.cpp
 *	@author		$LastChangedBy: soultaker $
 *	@date		$LastChangedDate$
 *	@brief		Implementation of ByteBuffer
 *	ID:			$Id$
 ******************************************************************************/
#include "solace/byteBuffer.hpp"
#include "solace/exception.hpp"


#include <cstring>  // memcpy


using namespace Solace;



MemoryView ByteBuffer::viewRemaining() {
    auto destAddr = _storage.dataAddress(position());

    return wrapMemory(const_cast<byte*>(destAddr), remaining());
//    return _storage.slice(position(), limit());
}


MemoryView ByteBuffer::viewWritten() {
    auto destAddr = _storage.dataAddress();
    return wrapMemory(const_cast<byte*>(destAddr), position());
//    return _storage.slice(0, position());
}


ByteBuffer& ByteBuffer::write(const ImmutableMemoryView& data, size_type bytesToWrite) {
    if (data.size() < bytesToWrite) {
         raise<OverflowException>("bytesToWrite", bytesToWrite, 0, data.size());
    }

    return write(data.dataAddress(), bytesToWrite);
}

ByteBuffer& ByteBuffer::write(const void* data, size_type count) {
    if (count == 0) {
        return *this;
    }

    if (remaining() < count) {
         raise<OverflowException>(position() + count, position(), remaining());
    }

    const auto pos = position();
    auto destAddr = _storage.dataAddress(pos);
    void* dest = reinterpret_cast<void*>(const_cast<ReadBuffer::Storage::value_type*>(destAddr));
    memmove(dest, data, count);

    advance(count);

    return *this;
}


ByteBuffer& ByteBuffer::writeLE(uint16 value) {
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


ByteBuffer& ByteBuffer::writeLE(uint32 value) {
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


ByteBuffer& ByteBuffer::writeLE(uint64 value) {
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


ByteBuffer& ByteBuffer::writeBE(uint16 value) {
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


ByteBuffer& ByteBuffer::writeBE(uint32 value) {
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


ByteBuffer& ByteBuffer::writeBE(uint64 value) {
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
