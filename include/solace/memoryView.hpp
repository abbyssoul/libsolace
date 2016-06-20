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
 * libSolace: MemoryView
 *	@file		solace/memoryView.hpp
 *	@author		$LastChangedBy: $
 *	@date		$LastChangedDate: $
 *	@brief		MemoryView object
 *	ID:			$Id: $
 ******************************************************************************/
#pragma once
#ifndef SOLACE_MEMORYVIEW_HPP
#define SOLACE_MEMORYVIEW_HPP

#include "solace/types.hpp"


namespace Solace {

/* Fixed-length raw data buffer/memory view.
 * This is a very thin abstruction on top of raw memory address -
 * it remembers memory block size and deallocates memory only if it ownes it.
 *
 * Buffer has value semantic and gives users random access to the undelying memory.
 * For the stream semantic please @see ByteBuffer
 */
struct MemoryView {
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
     * @return MemoryView object wrapping the memory address given
     */
    static MemoryView wrap(void* data, size_type size, bool copyData = false) {
        return {size, reinterpret_cast<byte*>(data), copyData};
    }

    static MemoryView wrap(char* data, size_type size, bool copyData = false) {
        return {size, reinterpret_cast<byte*>(data), copyData};
    }

    static MemoryView wrap(byte* data, size_type size, bool copyData = false) {
        return {size, reinterpret_cast<byte*>(data), copyData};
    }

public:

    explicit MemoryView(size_type newSize);

    MemoryView(MemoryView&& rhs):
                _size(rhs._size),
                _ownsData(rhs._ownsData),
                _data(rhs._data)
    {
        // Stuff up rhs so it won't be destructed
        rhs._size = 0;
        rhs._ownsData = false;
        rhs._data = 0;
    }

    MemoryView(const MemoryView& rhs);

    MemoryView(size_type size, void* data, bool copyData = false);

    /** Deallocate memory */
    ~MemoryView();

    MemoryView& swap(MemoryView& rhs) noexcept {
        std::swap(_size, rhs._size);
        std::swap(_ownsData, rhs._ownsData);
        std::swap(_data, rhs._data);

        return (*this);
    }

    MemoryView& operator= (MemoryView&& rhs) noexcept {
        return swap(rhs);
    }

    MemoryView& operator= (const MemoryView& rhs) noexcept {
        MemoryView(rhs).swap(*this);

        return *this;
    }

    bool equals(const MemoryView& other) const noexcept {
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

    bool operator== (const MemoryView& rhv) const noexcept {
        return equals(rhv);
    }

    bool operator!= (const MemoryView& rhv) const noexcept {
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

    MemoryView slice(size_type from, size_type to, bool copyData) const;

private:
    size_type   _size;
    bool        _ownsData;
    byte*       _data;
};

}  // End of namespace Solace
#endif  // SOLACE_MEMORYVIEW_HPP
