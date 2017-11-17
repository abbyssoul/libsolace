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
 * libSolace: Read Buffer
 *	@file		solace/readBuffer.hpp
 *	@brief		Read only byte buffer
 ******************************************************************************/
#pragma once
#ifndef SOLACE_READBUFFER_HPP
#define SOLACE_READBUFFER_HPP

#include "solace/immutableMemoryView.hpp"
#include "solace/memoryView.hpp"


namespace Solace {

/**
 * A byte storage with stream access semantic.
 * One can read and write into the byte buffer and this operations will advane current position.
 *
 */
class ReadBuffer {
public:

    typedef ImmutableMemoryView Storage;
    typedef Storage::size_type  size_type;

public:

    /** Construct an empty buffer of size zero */
    ReadBuffer() noexcept :
        _position(0),
        _limit(0),
        _storage()
    {}


    ReadBuffer(const ReadBuffer& other) = delete;
    ReadBuffer& operator= (const ReadBuffer&) = delete;


    /**
     * Construct the byte buffer by moving content from the other buffer
     * @param other Other buffer to take over from
     */
    ReadBuffer(ReadBuffer&& other) :
        _position(other._position),
        _limit(other._storage.size()),
        _storage(std::move(other._storage))
    {}


    /**
     * Construct the byte buffer from the memory view object
     * @param other Other buffer to copy data from
     */
    ReadBuffer(ImmutableMemoryView&& memView) :
        _position(0),
        _limit(memView.size()),
        _storage(std::move(memView))
    {
    }


    ReadBuffer& swap(ReadBuffer& rhs) noexcept {
        using std::swap;

        swap(_position, rhs._position);
        swap(_limit, rhs._limit);
        swap(_storage, rhs._storage);

        return *this;
    }


    ReadBuffer& operator= (ReadBuffer&& rhs) noexcept {
        return swap(rhs);
    }


    /**
     * Leave the limit unchanged and sets the position to zero.
     */
    ReadBuffer& rewind() noexcept {
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
    ReadBuffer& limit(size_type newLimit);

    /**
     * Set position back to the previously saved mark
     * @return Reference to this buffer.
     */
    ReadBuffer& reset(size_type savedMark) {
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
    ReadBuffer& position(size_type newPosition);

    /**
     * Increment current position by the given amount.
     * @param increment Amount to advance current position by.
     * @return Reference to this buffer for fluent interface.
     * @note It is illigal to advance position beyond the limit(), and exception will be raised in that case.
     */
    ReadBuffer& advance(size_type increment);

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

    /**
     * Read data form this buffer and store it into a given destantion.
     * @param dest Buffer to store read data into.
     * @return Reference to this for fluency.
     */
    ReadBuffer& read(MemoryView& dest) {
        return read(dest, dest.size());
    }

    ReadBuffer& read(MemoryView& dest, size_type bytesToRead);

    ReadBuffer& read(void* dest, size_type count);
    ReadBuffer& read(byte* dest, size_type count) {
        return read(reinterpret_cast<void*>(dest), count);
    }
    ReadBuffer& read(char* dest, size_type count) {
        return read(reinterpret_cast<void*>(dest), count);
    }
    const ReadBuffer& read(size_type offset, byte* dest, size_type bytesToRead) const;
    const ReadBuffer& read(size_type offset, MemoryView& dest, size_type bytesToRead) const;
    const ReadBuffer& read(size_type offset, MemoryView& dest) const  {
        return read(offset, dest, dest.size());
    }

    ImmutableMemoryView viewRemaining() const {
        return _storage.slice(position(), limit());
    }

    ImmutableMemoryView viewWritten() const {
        return _storage.slice(0, position());
    }

    // Endianess aware read/write
    ReadBuffer& readLE(int8& value)  { return read(&value, sizeof(int8)); }
    ReadBuffer& readLE(uint8& value) { return read(&value, sizeof(uint8)); }
    ReadBuffer& readLE(int16& value) { return readLE(reinterpret_cast<uint16&>(value)); }
    ReadBuffer& readLE(uint16& value);
    ReadBuffer& readLE(int32& value) { return readLE(reinterpret_cast<uint32&>(value)); }
    ReadBuffer& readLE(uint32& value);
    ReadBuffer& readLE(int64& value) { return readLE(reinterpret_cast<uint64&>(value)); }
    ReadBuffer& readLE(uint64& value);

    ReadBuffer& readBE(int8& value)  { return read(&value, sizeof(int8)); }
    ReadBuffer& readBE(uint8& value) { return read(&value, sizeof(uint8)); }
    ReadBuffer& readBE(int16& value) { return readBE(reinterpret_cast<uint16&>(value)); }
    ReadBuffer& readBE(uint16& value);
    ReadBuffer& readBE(int32& value) { return readBE(reinterpret_cast<uint32&>(value)); }
    ReadBuffer& readBE(uint32& value);
    ReadBuffer& readBE(int64& value) { return readBE(reinterpret_cast<uint64&>(value)); }
    ReadBuffer& readBE(uint64& value);

protected:

    size_type           _position;
    size_type           _limit;

    Storage             _storage;

};


inline
ReadBuffer& operator>> (ReadBuffer& src, char& c)     { return src.read(&c, sizeof(char));   }
inline
ReadBuffer& operator>> (ReadBuffer& src, int8& c)     { return src.read(&c, sizeof(int8));   }
inline
ReadBuffer& operator>> (ReadBuffer& src, uint8& c)    { return src.read(&c, sizeof(uint8));  }
inline
ReadBuffer& operator>> (ReadBuffer& src, int16& c)    { return src.read(&c, sizeof(int16));  }
inline
ReadBuffer& operator>> (ReadBuffer& src, uint16& c)   { return src.read(&c, sizeof(uint16)); }
inline
ReadBuffer& operator>> (ReadBuffer& src, int32& c)    { return src.read(&c, sizeof(int32));  }
inline
ReadBuffer& operator>> (ReadBuffer& src, uint32& c)   { return src.read(&c, sizeof(uint32)); }
inline
ReadBuffer& operator>> (ReadBuffer& src, int64& c)    { return src.read(&c, sizeof(int64));  }
inline
ReadBuffer& operator>> (ReadBuffer& src, uint64& c)   { return src.read(&c, sizeof(uint64)); }
inline
ReadBuffer& operator>> (ReadBuffer& src, float32& c)  { return src.read(&c, sizeof(float32)); }
inline
ReadBuffer& operator>> (ReadBuffer& src, float64& c)  { return src.read(&c, sizeof(float64)); }

inline
void swap(ReadBuffer& lhs, ReadBuffer& rhs) noexcept {
    lhs.swap(rhs);
}

}  // End of namespace Solace
#endif  // SOLACE_READBUFFER_HPP
