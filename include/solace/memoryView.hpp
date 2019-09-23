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
 *	@brief		MemoryView object
 ******************************************************************************/
#pragma once
#ifndef SOLACE_MEMORYVIEW_HPP
#define SOLACE_MEMORYVIEW_HPP

#include "solace/types.hpp"
#include "solace/assert.hpp"
#include "solace/utils.hpp"
#include "solace/result.hpp"


namespace Solace {

/// FWD declaration as error depends on StringView which in turn depends on this file.
//;

/**
 * Check if Runtime platform is big or little endian.
 * @return True if running on a big endian system.
 */
bool isBigendian() noexcept;


/* Read-only view into a fixed-length raw memory buffer.
 * A very thin abstruction on top of raw memory address -
 * it remembers memory block address and size.
 *
 * View has a value semantic and gives a user random access to the undelying memory.
 * Howeevr ImmutableMemoryView does not allow for modification of underlaying values - it is read-only.
 *
 * For a mutable access please use @see MemoryView
 * For the stream semantic please @see ReadBuffer
 *
 * Class invariants:
 *  * if (a.size() > 0) => (a.dataAddress() != nullptr)
 *  * if (a.dataAddress() == nullptr) => (a.size() == 0)
 *
 */
class MemoryView {
public:

    using MemoryAddress = void *;

    using size_type  = uint64;
    using value_type = byte;

    using const_reference = value_type const&;
    using const_iterator  = value_type const*;


    /**
     * Memory lock RAII wrapper
     */
    class Lock;

public:

    /**
     * Construct an empty memory view with:
     *  size == 0 and dataAddress == nullptr
     */
    constexpr MemoryView() noexcept = default;

    /**
     * Construct an empty memory view with:
     *  size == 0 and dataAddress == nullptr
     */
    constexpr MemoryView(decltype(nullptr)) noexcept
    {}

    /**
     * Construct an memory view from a data ptr and a size
     * @param data Memory address.
     * @param size The size of the memory block.
     * @note: it is illigal to pass null data with a non 0 size.
    */
    MemoryView(void const* data, size_type dataSize)
        : _size{dataSize}
        , _dataAddress{reinterpret_cast<value_type const*>(data)}
    {
        assertTrue(_dataAddress || _size == 0, "data");
    }

    template<typename PodType, size_t N>
    constexpr MemoryView(PodType const (&data)[N]) noexcept
        : _size{N * sizeof(PodType)}
        , _dataAddress{reinterpret_cast<value_type const*>(data)}
    {
    }


    MemoryView(MemoryView const&) noexcept = default;
    MemoryView& operator= (MemoryView const&) noexcept = default;

    /** Move construct an instance of the memory view **/
    constexpr MemoryView(MemoryView&& rhs) noexcept
        : _size{exchange(rhs._size, 0)}
        , _dataAddress{exchange(rhs._dataAddress, nullptr)}
    {
    }

    /** Move assignment **/
    MemoryView& operator= (MemoryView&& rhs) noexcept {
        return swap(rhs);
    }

    /*constexpr*/ MemoryView& swap(MemoryView& rhs) noexcept {
        using std::swap;

        swap(_size, rhs._size);
        swap(_dataAddress, rhs._dataAddress);

        return (*this);
    }


    constexpr bool equals(MemoryView const& other) const noexcept {
        if ((&other == this) ||
            ((_size == other._size) && (_dataAddress == other._dataAddress))) {
            return true;
        }

        if (_size != other._size) {
            return false;
        }

        for (size_type i = 0; i < _size; ++i) {
            if (_dataAddress[i] != other._dataAddress[i]) {
                return false;
            }
        }

        return true;
    }

    constexpr bool empty() const noexcept {
        return (_size == 0);
    }

    constexpr explicit operator bool() const noexcept {
        return (_dataAddress != nullptr);
    }

    /**
     * @return The size of this finite collection
     */
    constexpr size_type size() const noexcept { return _size; }

    /**
     * Return iterator to beginning of the collection
     * @return iterator to beginning of the collection
     */
    constexpr const_iterator begin() const noexcept {
        return _dataAddress;
    }

    /**
     * Return iterator to end of the collection
     * @return iterator to end of the collection
     */
    constexpr const_iterator end() const noexcept {
        return _dataAddress + _size;
    }

    value_type operator[] (size_type index) const;

    constexpr value_type const* dataAddress() const noexcept { return _dataAddress; }
    value_type const* dataAddress(size_type offset) const;

    template <typename T>
    constexpr T const* dataAs() const noexcept { return reinterpret_cast<const T*>(_dataAddress); }

    template <typename T>
    T const* dataAs(size_type offset) const {
        assertIndexInRange(offset, 0, this->size());
        assertIndexInRange(offset + sizeof(T), offset, this->size());

        return reinterpret_cast<const T*>(_dataAddress + offset);
    }


    /**
     * Lock this virtual address space memory into RAM, preventing that memory from being paged to the swap area.
     * @note: Memory locking and unlocking are performed in units of whole pages.
     * That is if when this memory view if locked - it will lock all memory that falls onto the same pages as this.
     */
	Result<Lock, class Error> lock();


    /**  Create a slice/window view of this memory segment.
     *
     * @param from [in] Offset to begin the slice from: [0, size())
     * @param to [in] The index to slise up until: [from, size())
     *
     * @return The slice of the memory segment, which may be empty.
     */
    MemoryView slice(size_type from, size_type to) const noexcept;

    template<typename T>
    MemoryView sliceFor(size_type offset, size_type count = 1) const noexcept {
        return slice(offset * sizeof(T), (offset + count) * sizeof(T));
    }

private:

    size_type                   _size{};
    byte const*                 _dataAddress{nullptr};
};



/**
 * Memory lock RAII wrapper
 */
class MemoryView::Lock {
public:
    ~Lock();

    Lock(MemoryView lockedMem) noexcept
		: _lockedMem(mv(lockedMem))
    {
        // No-op
    }

    Lock(Lock&&) noexcept = default;
    Lock(Lock const&) noexcept = delete;

    Lock& operator= (Lock const& SOLACE_UNUSED(v)) = delete;
    Lock& operator= (Lock&& SOLACE_UNUSED(v)) = default;

private:
    MemoryView  _lockedMem;
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
[[nodiscard]]
inline MemoryView wrapMemory(void const* data, MemoryView::size_type size) { return {data, size}; }
[[nodiscard]]
inline MemoryView wrapMemory(byte const* data, MemoryView::size_type size) { return {data, size}; }
[[nodiscard]]
inline MemoryView wrapMemory(char const* data, MemoryView::size_type size) { return {data, size}; }

template<typename PodType, size_t N>
[[nodiscard]]
constexpr MemoryView wrapMemory(PodType const (&data)[N]) noexcept {
    return {data};
}


inline
void swap(MemoryView& a, MemoryView& b) noexcept {
    a.swap(b);
}


inline
bool operator== (MemoryView const& lhs, MemoryView const& rhs) noexcept {
    return lhs.equals(rhs);
}

inline
bool operator!= (MemoryView const& lhs, MemoryView const& rhs) noexcept {
    return !lhs.equals(rhs);
}


}  // End of namespace Solace
#endif  // SOLACE_MEMORYVIEW_HPP
