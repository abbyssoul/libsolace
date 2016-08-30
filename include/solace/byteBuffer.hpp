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
#include "solace/optional.hpp"  // FIXME(abbyssoul): Consider removal


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
     * Construct the byte buffer by copeing content of the given buffer
     * @param other Other buffer to copy data from
     */
    ByteBuffer(const ByteBuffer& other) :
            _position(other._position),
            _limit(other._limit),
            _mark(other._mark),
            _storage(other._storage)
    {
    }

    /**
     * Construct the byte buffer by moving content from the other buffer
     * @param other Other buffer to take over from
     */
    ByteBuffer(ByteBuffer&& other) :
            _position(other._position),
            _limit(other._limit),
            _mark(std::move(other._mark)),
            _storage(std::move(other._storage))
    {
    }

    /**
     * Construct the byte buffer from the memory view object
     * @param other Other buffer to copy data from
     */
    ByteBuffer(const MemoryView& memView) :
            _position(0),
            _limit(memView.size()),
            _mark(),
            _storage(memView)
    {
    }

    /**
     * Construct the byte buffer from the memory view object
     * @param other Other buffer to copy data from
     */
    ByteBuffer(MemoryView&& memView) :
            _position(0),
            _limit(memView.size()),
            _mark(),
            _storage(std::move(memView))
    {
    }

    /**
     * Default destructor
     */
    ~ByteBuffer() = default;


    ByteBuffer& swap(ByteBuffer& rhs) noexcept {
        std::swap(_position, rhs._position);
        std::swap(_limit, rhs._limit);
        std::swap(_mark, rhs._mark);
        _storage.swap(rhs._storage);

        return *this;
    }

    ByteBuffer& operator= (const ByteBuffer& rhs) noexcept {
        ByteBuffer(rhs).swap(*this);

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
        return _position == 0;
    }

    /**
     * Set the limit to the capacity and the position to zero.
     */
    ByteBuffer& clear() {
        _position = 0;
        _mark = Optional<size_type>::none();
        _limit = capacity();

        return *this;
    }

    /**
     * Set the limit to the current position and then sets the position to zero.
     */
    ByteBuffer& flip() {
        _mark = Optional<size_type>::none();
        _limit = _position;
        _position = 0;

        return *this;
    }

    /**
     * Leave the limit unchanged and sets the position to zero.
     */
    ByteBuffer& rewind() {
        _mark = Optional<size_type>::none();
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

    ByteBuffer& mark() {
        _mark = Optional<size_type>::of(_position);

        return *this;
    }

    size_type position() const noexcept { return _position; }

    ByteBuffer& position(size_type newPosition);

    ByteBuffer& advance(size_type newPosition);

    size_type remaining() const noexcept { return limit() - position(); }

    bool hasRemaining() const noexcept { return remaining() > 0; }

    ByteBuffer& reset();

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
    ByteBuffer& read(MemoryView& buffer)  {
        return read(buffer, buffer.size());
    }

    ByteBuffer& read(MemoryView& buffer, ByteBuffer::size_type bytesToRead);

    ByteBuffer& read(void* bytes, size_type count);
    ByteBuffer& read(byte* bytes, size_type count) {
        return read(reinterpret_cast<void*>(bytes), count);
    }
    ByteBuffer& read(char* bytes, size_type count) {
        return read(reinterpret_cast<void*>(bytes), count);
    }
    const ByteBuffer& read(size_type offset, byte* bytes, size_type count) const;
    const ByteBuffer& read(size_type offset, MemoryView* bytes) const;
    const ByteBuffer& read(size_type offset, MemoryView* bytes, size_type count) const;


    ByteBuffer& write(const MemoryView& memView) {
        return write(memView.dataAddress(), memView.size());
    }

    ByteBuffer& write(const MemoryView& buffer, size_type bytesToWrite);

    ByteBuffer& write(const void* bytes, size_type count);
    ByteBuffer& write(const byte* bytes, size_type count);
    ByteBuffer& write(const char* bytes, size_type count) {
        return write(reinterpret_cast<const byte*>(bytes), count);
    }

    MemoryView viewRemaining() const {
        return _storage.slice(position(), limit());
    }

    MemoryView viewWritten() const {
        return _storage.slice(0, position());
    }

    ByteBuffer& operator<< (char c)     { return write(&c, sizeof(char)); }
    ByteBuffer& operator<< (byte c)     { return write(&c, sizeof(byte)); }
    ByteBuffer& operator<< (int8 c)     { return write(&c, sizeof(int8)); }
    ByteBuffer& operator<< (int16 c)    { return write(&c, sizeof(int16)); }
    ByteBuffer& operator<< (int32 c)    { return write(&c, sizeof(int32)); }
    ByteBuffer& operator<< (int64 c)    { return write(&c, sizeof(int64)); }
    ByteBuffer& operator<< (float32 c)  { return write(&c, sizeof(float32)); }
    ByteBuffer& operator<< (float64 c)  { return write(&c, sizeof(float64)); }

    ByteBuffer& operator>> (char& c)     { return read(&c, sizeof(char)); }
    ByteBuffer& operator>> (int8& c)     { return read(&c, sizeof(int8)); }
    ByteBuffer& operator>> (int16& c)    { return read(&c, sizeof(int16)); }
    ByteBuffer& operator>> (int32& c)    { return read(&c, sizeof(int32)); }
    ByteBuffer& operator>> (int64& c)    { return read(&c, sizeof(int64)); }
    ByteBuffer& operator>> (float32& c)  { return read(&c, sizeof(float32)); }
    ByteBuffer& operator>> (float64& c)  { return read(&c, sizeof(float64)); }


private:

    size_type           _position;
    size_type           _limit;
    Optional<size_type> _mark;

    Storage             _storage;
};

}  // End of namespace Solace
#endif  // SOLACE_BYTEBUFFER_HPP
