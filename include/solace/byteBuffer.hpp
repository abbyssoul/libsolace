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
 * libSolace: Byte buffer
 *	@file		solace/byteBuffer.hpp
 *	@author		$LastChangedBy: $
 *	@date		$LastChangedDate: $
 *	@brief		Byte buffer
 *	ID:			$Id: $
 ******************************************************************************/
#pragma once
#ifndef SOLACE_BYTEBUFFER_HPP
#define SOLACE_BYTEBUFFER_HPP

#include "solace/readBuffer.hpp"


namespace Solace {

/**
 * A byte storage with stream access semantic.
 * One can read and write into the byte buffer and this operations will advane current position.
 * FIXME: Implementation must be based on ReadBuffer
 */
class ByteBuffer: public ReadBuffer {
public:

    using ReadBuffer::size_type;

public:

    /** Construct an empty buffer of size zero */
    ByteBuffer() noexcept = default;


    ByteBuffer(const ByteBuffer& other) = delete;
    ByteBuffer& operator= (const ByteBuffer&) = delete;


    /**
     * Construct the byte buffer by moving content from the other buffer
     * @param other Other buffer to take over from
     */
    ByteBuffer(ByteBuffer&& other) :
        ReadBuffer(std::move(other))
    {}

    ByteBuffer(MemoryBuffer& buffer) :
        ReadBuffer(buffer)
    {}

    ByteBuffer(MemoryBuffer&& buffer) :
        ReadBuffer(std::move(buffer))
    {}

    /**
     * Construct the byte buffer from the memory view object
     * @param other Other buffer to copy data from
     */
    ByteBuffer(const MemoryView& memView) :
        ReadBuffer(memView)
    {}

    ByteBuffer(MemoryView&& memView) :
        ReadBuffer(std::move(memView))
    {}


    ByteBuffer& swap(ByteBuffer& rhs) noexcept {
        ReadBuffer::swap(rhs);

        return *this;
    }


    ByteBuffer& operator= (ByteBuffer&& rhs) noexcept {
        return swap(rhs);
    }

    /**
     * Set the limit to the capacity and the position to zero.
     */
    ByteBuffer& clear() noexcept {
        _position = 0;
        _limit = capacity();

        return *this;
    }

    /**
     * Set the limit to the current position and then sets the position to zero.
     */
    ByteBuffer& flip() noexcept {
        _limit = _position;
        _position = 0;

        return *this;
    }

    /**
     * Leave the limit unchanged and sets the position to zero.
     */
    ByteBuffer& rewind() noexcept {
        ReadBuffer::rewind();

        return *this;
    }

    /**
     * Write given raw bytes into this buffer.
     * @param data Raw bytes data to write.
     * @return Refernce to this for luency.
     * @note Exception is thrown if given data exceed buffer capacity.
     */
    ByteBuffer& write(const ImmutableMemoryView& data) {
        return write(data.dataAddress(), data.size());
    }

    /**
     * Write given raw bytes into this buffer.
     * @param data Raw bytes data to write.
     * @param bytesToWrite Number of bytes to write from data into this buffer.
     * @return Refernce to this for luency.
     * @note Exception is thrown if bytesToWrite exceed buffer capacity.
     */
    ByteBuffer& write(const ImmutableMemoryView& data, size_type bytesToWrite);

    ByteBuffer& write(const void* bytes, size_type count);
    ByteBuffer& write(const byte* bytes, size_type count) {
        return write(reinterpret_cast<const void*>(bytes), count);
    }

    ByteBuffer& write(const char* bytes, size_type count) {
        return write(reinterpret_cast<const void*>(bytes), count);
    }

    using ReadBuffer::viewRemaining;
    using ReadBuffer::viewWritten;

    MemoryView viewRemaining();

    MemoryView viewWritten();


    // Endianess aware write methods
    ByteBuffer& writeLE(int8 value)  { return write(&value, sizeof(value)); }
    ByteBuffer& writeLE(uint8 value) { return write(&value, sizeof(value)); }
    ByteBuffer& writeLE(int16 value) { return writeLE(static_cast<uint16>(value)); }
    ByteBuffer& writeLE(uint16 value);
    ByteBuffer& writeLE(int32 value) { return writeLE(static_cast<uint32>(value)); }
    ByteBuffer& writeLE(uint32 value);
    ByteBuffer& writeLE(int64 value) { return writeLE(static_cast<uint64>(value)); }
    ByteBuffer& writeLE(uint64 value);

    ByteBuffer& writeBE(int8 value)  { return write(&value, sizeof(value)); }
    ByteBuffer& writeBE(uint8 value) { return write(&value, sizeof(value)); }
    ByteBuffer& writeBE(int16 value) { return writeBE(static_cast<uint16>(value)); }
    ByteBuffer& writeBE(uint16 value);
    ByteBuffer& writeBE(int32 value) { return writeBE(static_cast<uint32>(value)); }
    ByteBuffer& writeBE(uint32 value);
    ByteBuffer& writeBE(int64 value) { return writeBE(static_cast<uint64>(value)); }
    ByteBuffer& writeBE(uint64 value);
};



inline
ByteBuffer& operator<< (ByteBuffer& dest, char c)     { return dest.write(&c, sizeof(char));   }
inline
ByteBuffer& operator<< (ByteBuffer& dest, int8 c)     { return dest.write(&c, sizeof(int8));   }
inline
ByteBuffer& operator<< (ByteBuffer& dest, uint8 c)    { return dest.write(&c, sizeof(uint8));  }
inline
ByteBuffer& operator<< (ByteBuffer& dest, int16 c)    { return dest.write(&c, sizeof(int16));  }
inline
ByteBuffer& operator<< (ByteBuffer& dest, uint16 c)   { return dest.write(&c, sizeof(uint16)); }
inline
ByteBuffer& operator<< (ByteBuffer& dest, int32 c)    { return dest.write(&c, sizeof(int32));  }
inline
ByteBuffer& operator<< (ByteBuffer& dest, uint32 c)   { return dest.write(&c, sizeof(uint32)); }
inline
ByteBuffer& operator<< (ByteBuffer& dest, int64 c)    { return dest.write(&c, sizeof(int64));  }
inline
ByteBuffer& operator<< (ByteBuffer& dest, uint64 c)   { return dest.write(&c, sizeof(uint64)); }
inline
ByteBuffer& operator<< (ByteBuffer& dest, float32 c)  { return dest.write(&c, sizeof(float32));}
inline
ByteBuffer& operator<< (ByteBuffer& dest, float64 c)  { return dest.write(&c, sizeof(float64));}


inline void swap(ByteBuffer& lhs, ByteBuffer& rhs) noexcept {
    lhs.swap(rhs);
}

}  // End of namespace Solace
#endif  // SOLACE_BYTEBUFFER_HPP
