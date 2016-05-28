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
#include "solace/optional.hpp"


namespace Solace {

/* Fixed-length raw data buffer/memory view.
 * Buffer is a very thin abstruction on top of raw memory address -
 * it remembers memory block size and deallocates memory if it owned it.
 *
 * Buffer has value semantic and gives users random access to the undelying memory.
 * For stream semantic please @see ByteBuffer
 */
struct Buffer {
    typedef uint32  size_type;
    typedef byte    value_type;

    typedef value_type&         reference;
    typedef const value_type&   const_reference;

    typedef value_type*         iterator;
    typedef const value_type*   const_iterator;

    /**
     * Wrap already allocated memory pointer into the buffer object.
     *
     * @param data Pointer the memory to wrap
     * @param size Size in bytes of the allocated memory
     * @param copyData If true - a memory to store the data will be allocated and
     * the content of the data will be copied. Created buffer will be responsible for managing new memory.
     * If false - new buffer will be acting as a pointer and it's caller's responsibility to despose of the memory.
     *
     * @return Buffer object wrapping the memory address given
     */
    static Buffer wrap(void* data, size_type size, bool copyData = false) {
        return {size, reinterpret_cast<byte*>(data), copyData};
    }

    static Buffer wrap(char* data, size_type size, bool copyData = false) {
        return {size, reinterpret_cast<byte*>(data), copyData};
    }

    static Buffer wrap(byte* data, size_type size, bool copyData = false) {
        return {size, reinterpret_cast<byte*>(data), copyData};
    }

    explicit Buffer(size_type newSize);

    Buffer(Buffer&& rhs):
                _size(rhs._size),
                _ownsData(rhs._ownsData),
                _data(rhs._data)
    {
        // Stuff up rhs so it won't be destructed
        rhs._size = 0;
        rhs._ownsData = false;
        rhs._data = 0;
    }

    Buffer(const Buffer& rhs);

    Buffer(size_type size, void* data, bool copyData = false);

    /** Deallocate memory */
    ~Buffer();


    Buffer& swap(Buffer& rhs) noexcept {
        std::swap(_size, rhs._size);
        std::swap(_ownsData, rhs._ownsData);
        std::swap(_data, rhs._data);

        return (*this);
    }

    Buffer& operator= (Buffer&& rhs) noexcept {
        return swap(rhs);
    }

    Buffer& operator= (const Buffer& rhs) noexcept {
        Buffer(rhs).swap(*this);

        return *this;
    }

    bool equals(const Buffer& other) const noexcept {
        if ((&other == this) ||
            ((_size == other._size) && (_data == other._data))) {
            return true;
        }

        if (_size != other._size)
            return false;

        for (size_type i = 0; i < _size; ++i) {
            if (_data[i] != other._data[i])
                return false;
        }

        return true;
    }

    bool operator== (const Buffer& rhv) const noexcept {
        return equals(rhv);
    }

    bool operator!= (const Buffer& rhv) const noexcept {
        return !equals(rhv);
    }

    bool empty() const noexcept {
        return ((!_data) || (_size == 0));
    }

    /**
     * @return The size of this finite collection
     */
    size_type size() const noexcept { return _size; }

    /**
     * Return iterator to beginning of the collection
     * @return iterator to beginning of the collection
     */
    const_iterator begin() const {
        return _data;
    }

    iterator begin() {
        return _data;
    }

    /**
     * Return iterator to end of the collection
     * @return iterator to end of the collection
     */
    const_iterator end() const {
        return _data + _size;
    }

    iterator end() {
        return _data + _size;
    }

    value_type first() const noexcept { return _data[0]; }
    value_type last()  const noexcept { return _data[_size - 1]; }

    reference  operator[] (size_type index) { return _data[index]; }
    value_type operator[] (size_type index) const { return _data[index]; }

    byte* data() const noexcept { return _data; }
    bool isOwner() const noexcept { return _ownsData; }

    Buffer slice(size_type from, size_type to, bool copyData) const;

private:
    size_type   _size;
    bool        _ownsData;
    byte*       _data;
};



/**
 * A byte storage with stream access semantic.
 * One can read and write into the byte buffer and this operations will advane current position.
 *
 */
class ByteBuffer {
public:

    typedef Buffer              Storage;
    typedef Storage::size_type  size_type;

public:

    /**
     * Construct a byte buffer of the given size
     * @param initialSize Fixed size of the byte buffer
     */
    explicit ByteBuffer(size_type initialSize) :
            _position(0),
            _limit(initialSize),
            _mark(),
            _storage(initialSize)
    {
    }

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
            _mark(other._mark),
            _storage(std::move(other._storage))
    {
    }


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

    size_type remaining() const noexcept { return limit() - position(); }

    bool hasRemaining() const noexcept { return remaining() > 0; }

    ByteBuffer& reset();


    // Read content back
    size_type read(byte* bytes, size_type count);
    size_type read(size_type offset, byte* bytes, size_type count) const;

    byte get();
    byte get(size_type position) const;

    ByteBuffer& operator<< (char c);

    ByteBuffer& write(byte* bytes, size_type count);
    ByteBuffer& write(const char* bytes, size_type count);

    byte* data() const noexcept {
        return _storage.data();
    }

    byte* dataPositiong() const noexcept {
        return _storage.data() + position();
    }

private:

    size_type           _position;
    size_type           _limit;
    Optional<size_type> _mark;

    Storage _storage;
};

}  // End of namespace Solace
#endif  // SOLACE_BYTEBUFFER_HPP
