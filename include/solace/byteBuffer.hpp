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
 *	@brief		Byte write buffer
 *	ID:			$Id: $
 ******************************************************************************/
#pragma once
#ifndef SOLACE_BYTEBUFFER_HPP
#define SOLACE_BYTEBUFFER_HPP

#include "solace/types.hpp"
#include "solace/memoryView.hpp"


namespace Solace {

/**
 * A byte storage with stream access semantic.
 * One can read and write into the byte buffer and this operations will advane current position.
 *
 */
class ByteBuffer {
public:

    typedef MemoryView          Storage;
    typedef Storage::size_type  size_type;

public:

    /**
     * Default destructor
     */
    ~ByteBuffer() = default;

    /** Construct an empty buffer of size zero */
    ByteBuffer() noexcept {
    }


    ByteBuffer(const ByteBuffer& other) = delete;
    ByteBuffer& operator= (const ByteBuffer&) = delete;


    /**
     * Construct the byte buffer by moving content from the other buffer
     * @param other Other buffer to take over from
     */
    ByteBuffer(ByteBuffer&& other) :
        _storage(std::move(other._storage)),
        _position(other._position),
        _limit(_storage.size())
    {
    }

    /**
     * Construct the byte buffer from the memory view object
     * @param other Other buffer to copy data from
     */
    ByteBuffer(MemoryView&& memView) :
        _storage(std::move(memView)),
        _position(0),
        _limit(_storage.size())
    {
    }


    ByteBuffer& swap(ByteBuffer& rhs) noexcept {
        using std::swap;

        swap(_storage, rhs._storage);
        swap(_position, rhs._position);
        swap(_limit, rhs._limit);

        return *this;
    }


    ByteBuffer& operator= (ByteBuffer&& rhs) noexcept {
        return swap(rhs);
    }


    /**
     * Check if the bute buffer is empty, that is no data has been written
     * @return True if position is zero and buffer has no data.
     */
    bool empty() const noexcept {
        return (_position == 0);
    }

    /**
     * Set the limit to the capacity and the position to zero.
     */
    ByteBuffer& clear() {
        _position = 0;
        _limit = capacity();

        return *this;
    }

    /**
     * Set the limit to the current position and then sets the position to zero.
     */
    ByteBuffer& flip() {
        _limit = _position;
        _position = 0;

        return *this;
    }

    /**
     * Leave the limit unchanged and sets the position to zero.
     */
    ByteBuffer& rewind() {
        _position = 0;

        return *this;
    }


    /** Returns this buffer's capacity.
     * A buffer's capacity is the number of elements it contains.
     * The capacity of a buffer is never negative and never changes.
     * @return Fixed number of bytes this buffer was created to store.
     */
    size_type capacity() const noexcept { return _storage.size(); }

    /** Return data read/write limit of this buffer.
     * For write buffer this is the maximum number of bytes this buffer can hold.
     * For read buffer this is number of bytes that can be read from this buffer.
     * @note Buffer limit is always less of equal to buffer's capacity.
     * @return Number of bytes that can be read/written to the buffer
     */
    size_type limit() const noexcept { return _limit; }

    /** Limit number of bytes that can be read/written to this buffer.

     * @note Buffer limit is always less of equal to buffer's capacity.
     * @return A reference to this this buffer.
     *
     * @throws IllegalArgumentException if attempt is made to set limit to more then this buffer capacity.
     */
    ByteBuffer& limit(size_type newLimit);

    /**
     * Set position back to the previously saved mark
     * @return Reference to this buffer.
     */
    ByteBuffer& reset(size_type savedMark) {
        return position(savedMark);
    }

    /**
     * Get current position in the buffer.
     * It can be stored to later return to it using @see reset
     * @return Current position in the buffer
     */
    size_type position() const noexcept { return _position; }

    /**
     * Set current position to the given one.
     *
     * @return Reference to this buffer for fluent interface.
     * @note It is illigal to set position beyond the limit(), and exception will be raised in that case.
     */
    ByteBuffer& position(size_type newPosition);

    /**
     * Increment current position by the given amount.
     * @param increment Amount to advance current position by.
     * @return Reference to this buffer for fluent interface.
     * @note It is illigal to advance position beyond the limit(), and exception will be raised in that case.
     */
    ByteBuffer& advance(size_type increment);

    /**
     * Get remaining number of bytes in the buffer (Up to the limit)
     * @return Remaining number of bytes in the buffer.
     */
    size_type remaining() const noexcept { return limit() - position(); }

    /**
     * Check if there are bytes left in the buffer (Up to the limit)
     * @return True if there are still some data before the limit is reached.
     */
    bool hasRemaining() const noexcept { return remaining() > 0; }

    /**
     * Read a single byte from the buffer
     * @return One byte read from the buffer
     */
    byte get();

    /**
     * Get a single byte from the buffer in the given position
     * @return One byte read from the buffer
     * @note This operation does not advance current position
     */
    byte get(size_type position) const;

    // Read content back
    ByteBuffer& read(MemoryView& from)  {
        return read(from, from.size());
    }

    ByteBuffer& read(MemoryView& from, size_type bytesToRead);

    ByteBuffer& read(void* bytes, size_type count);
    ByteBuffer& read(byte* bytes, size_type count) {
        return read(reinterpret_cast<void*>(bytes), count);
    }
    ByteBuffer& read(char* bytes, size_type count) {
        return read(reinterpret_cast<void*>(bytes), count);
    }
    const ByteBuffer& read(size_type offset, byte* bytes, size_type count) const;
    const ByteBuffer& read(size_type offset, MemoryView& from, size_type count) const;
    const ByteBuffer& read(size_type offset, MemoryView& from) const  {
        return read(offset, from, from.size());
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

    ImmutableMemoryView viewRemaining() const {
        return _storage.slice(position(), limit());
    }

    MemoryView viewRemaining() {
        return _storage.slice(position(), limit());
    }

    ImmutableMemoryView viewWritten() const {
        return _storage.slice(0, position());
    }

    MemoryView viewWritten() {
        return _storage.slice(0, position());
    }

    ByteBuffer& operator<< (char c)     { return write(&c, sizeof(char));   }
    ByteBuffer& operator<< (int8 c)     { return write(&c, sizeof(int8));   }
    ByteBuffer& operator<< (uint8 c)    { return write(&c, sizeof(uint8));  }
    ByteBuffer& operator<< (int16 c)    { return write(&c, sizeof(int16));  }
    ByteBuffer& operator<< (uint16 c)   { return write(&c, sizeof(uint16)); }
    ByteBuffer& operator<< (int32 c)    { return write(&c, sizeof(int32));  }
    ByteBuffer& operator<< (uint32 c)   { return write(&c, sizeof(uint32)); }
    ByteBuffer& operator<< (int64 c)    { return write(&c, sizeof(int64));  }
    ByteBuffer& operator<< (uint64 c)   { return write(&c, sizeof(uint64)); }
    ByteBuffer& operator<< (float32 c)  { return write(&c, sizeof(float32));}
    ByteBuffer& operator<< (float64 c)  { return write(&c, sizeof(float64));}

    ByteBuffer& operator>> (char& c)     { return read(&c, sizeof(char));   }
    ByteBuffer& operator>> (int8& c)     { return read(&c, sizeof(int8));   }
    ByteBuffer& operator>> (uint8& c)    { return read(&c, sizeof(uint8));  }
    ByteBuffer& operator>> (int16& c)    { return read(&c, sizeof(int16));  }
    ByteBuffer& operator>> (uint16& c)   { return read(&c, sizeof(uint16)); }
    ByteBuffer& operator>> (int32& c)    { return read(&c, sizeof(int32));  }
    ByteBuffer& operator>> (uint32& c)   { return read(&c, sizeof(uint32)); }
    ByteBuffer& operator>> (int64& c)    { return read(&c, sizeof(int64));  }
    ByteBuffer& operator>> (uint64& c)   { return read(&c, sizeof(uint64)); }
    ByteBuffer& operator>> (float32& c)  { return read(&c, sizeof(float32)); }
    ByteBuffer& operator>> (float64& c)  { return read(&c, sizeof(float64)); }

    // Endianess aware read/write
    ByteBuffer& writeLE(int8 value)  { return *this << value; }
    ByteBuffer& writeLE(uint8 value) { return *this << value; }
    ByteBuffer& writeLE(int16 value) { return writeLE(static_cast<uint16>(value)); }
    ByteBuffer& writeLE(uint16 value);
    ByteBuffer& writeLE(int32 value) { return writeLE(static_cast<uint32>(value)); }
    ByteBuffer& writeLE(uint32 value);
    ByteBuffer& writeLE(int64 value) { return writeLE(static_cast<uint64>(value)); }
    ByteBuffer& writeLE(uint64 value);

    ByteBuffer& readLE(int8& value)  { return *this >> value; }
    ByteBuffer& readLE(uint8& value) { return *this >> value; }
    ByteBuffer& readLE(int16& value) { return readLE(reinterpret_cast<uint16&>(value)); }
    ByteBuffer& readLE(uint16& value);
    ByteBuffer& readLE(int32& value) { return readLE(reinterpret_cast<uint32&>(value)); }
    ByteBuffer& readLE(uint32& value);
    ByteBuffer& readLE(int64& value) { return readLE(reinterpret_cast<uint64&>(value)); }
    ByteBuffer& readLE(uint64& value);

    ByteBuffer& writeBE(int8 value)  { return *this << value; }
    ByteBuffer& writeBE(uint8 value) { return *this << value; }
    ByteBuffer& writeBE(int16 value) { return writeBE(static_cast<uint16>(value)); }
    ByteBuffer& writeBE(uint16 value);
    ByteBuffer& writeBE(int32 value) { return writeBE(static_cast<uint32>(value)); }
    ByteBuffer& writeBE(uint32 value);
    ByteBuffer& writeBE(int64 value) { return writeBE(static_cast<uint64>(value)); }
    ByteBuffer& writeBE(uint64 value);

    ByteBuffer& readBE(int8& value)  { return *this >> value; }
    ByteBuffer& readBE(uint8& value) { return *this >> value; }
    ByteBuffer& readBE(int16& value) { return readBE(reinterpret_cast<uint16&>(value)); }
    ByteBuffer& readBE(uint16& value);
    ByteBuffer& readBE(int32& value) { return readBE(reinterpret_cast<uint32&>(value)); }
    ByteBuffer& readBE(uint32& value);
    ByteBuffer& readBE(int64& value) { return readBE(reinterpret_cast<uint64&>(value)); }
    ByteBuffer& readBE(uint64& value);

private:

    Storage             _storage;

    size_type           _position;
    size_type           _limit;

};

inline void swap(ByteBuffer& lhs, ByteBuffer& rhs) noexcept {
    lhs.swap(rhs);
}

}  // End of namespace Solace
#endif  // SOLACE_BYTEBUFFER_HPP
