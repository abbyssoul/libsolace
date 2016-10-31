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
#include "solace/assert.hpp"


#include <functional>


namespace Solace {

/* Fixed-length raw data buffer/memory view.
 * This is a very thin abstruction on top of raw memory address -
 * it remembers memory block size and deallocates memory only if it ownes it.
 *
 * Buffer has value semantic and gives users random access to the undelying memory.
 * For the stream semantic please @see ByteBuffer
 */
class MemoryView {
public:
    typedef uint32              size_type;
    typedef byte                value_type;

    typedef value_type&         reference;
    typedef const value_type&   const_reference;

    typedef value_type*         iterator;
    typedef const value_type*   const_iterator;

public:

    MemoryView() noexcept;

    MemoryView(MemoryView&& rhs) noexcept;

    /** Deallocate memory.. maybe */
    ~MemoryView();

    MemoryView& swap(MemoryView& rhs) noexcept;

    MemoryView& operator= (MemoryView&& rhs) noexcept {
        return swap(rhs);
    }

    bool equals(const MemoryView& other) const noexcept {
        if ((&other == this) ||
            ((_size == other._size) && (_dataAddress == other._dataAddress))) {
            return true;
        }

        if (_size != other._size) {
            return false;
        }

        for (size_type i = 0; i < _size; ++i) {
            if (_dataAddress[i] != other._dataAddress[i])
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
        return (_size == 0);
    }

    /**
     * @return The size of this finite collection
     */
    size_type size() const noexcept { return _size; }

    /**
     * Return iterator to beginning of the collection
     * @return iterator to beginning of the collection
     */
    const_iterator begin() const noexcept {
        return _dataAddress;
    }

    iterator begin() noexcept {
        return _dataAddress;
    }

    /**
     * Return iterator to end of the collection
     * @return iterator to end of the collection
     */
    const_iterator end() const noexcept {
        return _dataAddress + _size;
    }

    iterator end() noexcept {
        return _dataAddress + _size;
    }

    value_type first() const noexcept { return _dataAddress[0]; }
    value_type last()  const noexcept { return _dataAddress[size() - 1]; }

    reference  operator[] (size_type index);
    value_type operator[] (size_type index) const;

    byte* dataAddress() const noexcept { return _dataAddress; }
    byte* dataAddress(size_type offset) const;

    template <typename T>
    T* dataAs(size_type offset = 0) const {
        assertIndexInRange(offset, 0, this->size());
        assertIndexInRange(offset + sizeof(T), offset, this->size() + 1);

        return reinterpret_cast<T*>(_dataAddress + offset);
    }


    /**
     * Copy data from the given memory view into this one
     * @param source Source of data to be written into this location.
     * @param offset Offset location into this buffer to copy data to.
     */
    void write(const MemoryView& source, size_type offset = 0);

    /**
     * Copy data from this buffer into the given one.
     * @param data Data destinatio to transer data into.
     */
    void read(MemoryView& dest);

    /**
     * Copy data from this buffer into the given one.
     * @param data Data destinatio to transer data into.
     * @param bytesToRead Number of bytes to copy from this bufer into the destanation.
     * @param offset Offset location into this buffer to start reading from.
     */
    void read(MemoryView& dest, size_type bytesToRead, size_type offset = 0);

    /** Fill memory block with the given value.
     *
     * @param value Value to fill memory block with.
     *
     * @return A reference to this for fluent interface
     */
    MemoryView& fill(byte value);

    /** Fill memory block with the given value.
     *
     * @param value Value to fill memory block with.
     * @param from Offset to start the fill from.
     * @param to Index to fill the view upto.
     *
     * @return A reference to this for fluent interface
     */
    MemoryView& fill(byte value, size_type from, size_type to);


    /**  Create a slice/window view of this memory segment.
     *
     * @param from [in] Offset to begin the slice from.
     * @param to [in] The last element to slice to.
     *
     * @return The slice of the memory segment.
     */
    MemoryView slice(size_type from, size_type to) const;

    /**
     * Lock this virtual address space memory into RAM, preventing that memory from being paged to the swap area.
     * @note: Memory locking and unlocking are performed in units of whole pages.
     * That is if when this memory view if locked - it will lock all memory that falls onto the same pages as this.
     */
    MemoryView& lock();

    /**
     * Unlock virtual address space, so that pages in the specified virtual address range may once more to be
     * swapped out if required by the kernel memory manager.
     * @note: Memory locking and unlocking are performed in units of whole pages.
     * That is if when this memory view is unlocked - it will also unlock all memory that falls onto the same pages.
     */
    MemoryView& unlock();

    friend MemoryView wrapMemory(byte*, MemoryView::size_type, const std::function<void(MemoryView*)>&);

    MemoryView viewShallow() const;

protected:

    MemoryView(size_type size, void* data, const std::function<void(MemoryView*)>& freeFunc);


private:

    size_type   _size;
    byte*       _dataAddress;

    std::function<void(MemoryView*)> _free;
};


/**
 * Wrap an memory pointer into a memory view object.
 *
 * @param data Pointer the memory to wrap
 * @param size Size in bytes of the allocated memory segment
 * @param freeFunc A funcion to call when the wrapper object is destroyed.
 * This can be used to free the memory pointed by the data pointed as the MemoryView does not assume the ownership.
 *
 * @return MemoryView object wrapping the memory address given
 */
inline MemoryView wrapMemory(byte* data, MemoryView::size_type size,
                             const std::function<void(MemoryView*)>& freeFunc = 0) {
    return MemoryView{size, data, freeFunc};
}

inline MemoryView wrapMemory(void* data, MemoryView::size_type size,
                             const std::function<void(MemoryView*)>& freeFunc = 0) {
    return wrapMemory(reinterpret_cast<byte*>(data), size, freeFunc);
}

inline MemoryView wrapMemory(char* data, MemoryView::size_type size,
                             const std::function<void(MemoryView*)>& freeFunc = 0) {
    return wrapMemory(reinterpret_cast<byte*>(data), size, freeFunc);
}

template<typename PodType, size_t N>
inline MemoryView wrapMemory(PodType (&data)[N],
                             const std::function<void(MemoryView*)>& freeFunc = 0)
{
    return wrapMemory(static_cast<void*>(data), N * sizeof(PodType), freeFunc);
}


inline void swap(MemoryView& a, MemoryView& b) {
    a.swap(b);
}

}  // End of namespace Solace
#endif  // SOLACE_MEMORYVIEW_HPP
