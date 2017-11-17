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

#include "solace/immutableMemoryView.hpp"


namespace Solace {

/* Fixed-length raw data buffer/memory view.
 * This is a very thin abstruction on top of raw memory address -
 * it remembers memory block size and deallocates memory only if it ownes it.
 *
 * Buffer has value semantic and gives users random access to the undelying memory.
 * For the stream semantic please @see ByteBuffer
 */
class MemoryView : public ImmutableMemoryView {
public:
    using ImmutableMemoryView::size_type;
    using ImmutableMemoryView::value_type;

    using ImmutableMemoryView::const_iterator;
    using ImmutableMemoryView::const_reference;

    typedef value_type&         reference;
    typedef value_type*         iterator;

public:

    /** Deallocate memory.. maybe */
    ~MemoryView() = default;

    /** Construct an empty memory view */
    MemoryView() noexcept = default;

    MemoryView(const MemoryView&) = delete;
    MemoryView& operator= (const MemoryView&) = delete;

    MemoryView(MemoryView&& rhs) noexcept:
        ImmutableMemoryView(std::move(rhs))
    { }

    MemoryView& swap(MemoryView& rhs) noexcept {
        ImmutableMemoryView::swap(rhs);

        return (*this);
    }


    MemoryView& operator= (MemoryView&& rhs) noexcept {
        return swap(rhs);
    }

    bool equals(const MemoryView& other) const noexcept {
        return ImmutableMemoryView::equals(other);
    }

    bool operator== (const MemoryView& rhv) const noexcept {
        return equals(rhv);
    }

    bool operator!= (const MemoryView& rhv) const noexcept {
        return !equals(rhv);
    }

    /**
     * Return iterator to beginning of the collection
     * @return iterator to beginning of the collection
     */
    iterator begin() noexcept {
        return const_cast<value_type*>(dataAddress());
    }
    using ImmutableMemoryView::begin;

    /**
     * Return iterator to end of the collection
     * @return iterator to end of the collection
     */
    iterator end() noexcept {
        return const_cast<value_type*>(dataAddress() + size());
    }
    using ImmutableMemoryView::end;

    reference  operator[] (size_type index);
    using ImmutableMemoryView::operator[];

    using ImmutableMemoryView::dataAddress;

    value_type* dataAddress();
    value_type* dataAddress(size_type offset);


    using ImmutableMemoryView::dataAs;
    template <typename T>
    T* dataAs(size_type offset = 0) {
        assertIndexInRange(offset, 0, this->size());
        assertIndexInRange(offset + sizeof(T), offset, this->size() + 1);

        return reinterpret_cast<T*>(dataAddress() + offset);
    }


    /**
     * Copy data from the given memory view into this one
     * @param source Source of data to be written into this location.
     * @param offset Offset location into this buffer to copy data to.
     */
    void write(const ImmutableMemoryView& source, size_type offset = 0);

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

    using ImmutableMemoryView::slice;
    MemoryView slice(size_type from, size_type to);

    MemoryView viewShallow() const;

    friend MemoryView wrapMemory(byte*, MemoryView::size_type, const MemoryViewDisposer*);

protected:

    MemoryView(size_type size, void* data, const MemoryViewDisposer* disposer);

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
                             const MemoryViewDisposer* freeFunc = 0) {
    return MemoryView{size, data, freeFunc};
}

inline MemoryView wrapMemory(void* data, MemoryView::size_type size,
                             const MemoryViewDisposer* freeFunc = 0) {
    return wrapMemory(reinterpret_cast<byte*>(data), size, freeFunc);
}

inline MemoryView wrapMemory(char* data, MemoryView::size_type size,
                             const MemoryViewDisposer* freeFunc = 0) {
    return wrapMemory(reinterpret_cast<byte*>(data), size, freeFunc);
}

template<typename PodType, size_t N>
inline MemoryView wrapMemory(PodType (&data)[N],
                             const MemoryViewDisposer* freeFunc = 0)
{
    return wrapMemory(static_cast<void*>(data), N * sizeof(PodType), freeFunc);
}


inline void swap(MemoryView& a, MemoryView& b) {
    a.swap(b);
}


/// Some data manipulication utilities:

/*
 * 32-bit integer manipulation (little endian)
 */
inline
void getUint32_BE(uint32& n, const byte* b, size_t i) {
    n =   static_cast<uint32>(b[i    ]) << 24
        | static_cast<uint32>(b[i + 1]) << 16
        | static_cast<uint32>(b[i + 2]) <<  8
        | static_cast<uint32>(b[i + 3]);
}

inline
void putUint32_BE(uint32& n, byte* b, size_t i) {
    b[i    ] = static_cast<byte>(n >> 24);
    b[i + 1] = static_cast<byte>(n >> 16);
    b[i + 2] = static_cast<byte>(n >>  8);
    b[i + 3] = static_cast<byte>(n);
}


/*
 * 32-bit integer manipulation macros (little endian)
 */
inline
void getUint32_LE(uint32& n, const byte* b, size_t i) {
    n =   static_cast<uint32>(b[i   ])
        | static_cast<uint32>(b[i + 1]) <<  8
        | static_cast<uint32>(b[i + 2]) << 16
        | static_cast<uint32>(b[i + 3]) << 24;
}

inline
void putUint32_LE(uint32& n, byte* b, size_t i) {
    b[i    ] = static_cast<byte>((n)       & 0xFF);
    b[i + 1] = static_cast<byte>((n >>  8) & 0xFF);
    b[i + 2] = static_cast<byte>((n >> 16) & 0xFF);
    b[i + 3] = static_cast<byte>((n >> 24) & 0xFF);
}


/*
 * 64-bit integer manipulation macros (little endian)
 */
inline
void getUint64_LE(uint64& n, const byte* b, size_t i) {
    n =   static_cast<uint64>(b[i   ])
        | static_cast<uint64>(b[i + 1]) <<  8
        | static_cast<uint64>(b[i + 2]) << 16
        | static_cast<uint64>(b[i + 3]) << 24
        | static_cast<uint64>(b[i + 4]) << 32
        | static_cast<uint64>(b[i + 5]) << 40
        | static_cast<uint64>(b[i + 6]) << 48
        | static_cast<uint64>(b[i + 7]) << 56;
}

inline
void putUint64_LE(uint64& n, byte* b, size_t i) {
    b[i    ] = static_cast<byte>((n)       & 0xFF);
    b[i + 1] = static_cast<byte>((n >>  8) & 0xFF);
    b[i + 2] = static_cast<byte>((n >> 16) & 0xFF);
    b[i + 3] = static_cast<byte>((n >> 24) & 0xFF);
    b[i + 4] = static_cast<byte>((n >> 32) & 0xFF);
    b[i + 5] = static_cast<byte>((n >> 40) & 0xFF);
    b[i + 6] = static_cast<byte>((n >> 48) & 0xFF);
    b[i + 7] = static_cast<byte>((n >> 56) & 0xFF);
}

}  // End of namespace Solace
#endif  // SOLACE_MEMORYVIEW_HPP
