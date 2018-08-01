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


namespace Solace {

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
 * Class invariant:
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
    MemoryView(void const* data, size_type size);

    MemoryView(MemoryView const&) noexcept = default;
    MemoryView& operator= (MemoryView const&) noexcept = default;

    /** Move construct an instance of the memory view **/
    MemoryView(MemoryView&& rhs) noexcept :
        _size(std::exchange(rhs._size, 0)),
        _dataAddress(std::exchange(rhs._dataAddress, nullptr))
    {
    }

    /** Move assignment **/
    MemoryView& operator= (MemoryView&& rhs) noexcept {
        return swap(rhs);
    }

    MemoryView& swap(MemoryView& rhs) noexcept {
        using std::swap;

        swap(_size, rhs._size);
        swap(_dataAddress, rhs._dataAddress);

        return (*this);
    }


    bool equals(MemoryView const& other) const noexcept {
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

    explicit operator bool() const noexcept {
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
    const_iterator begin() const noexcept {
        return _dataAddress;
    }

    /**
     * Return iterator to end of the collection
     * @return iterator to end of the collection
     */
    const_iterator end() const noexcept {
        return _dataAddress + _size;
    }


    value_type first() const noexcept { return _dataAddress[0]; }
    value_type last()  const noexcept { return _dataAddress[size() - 1]; }

    value_type operator[] (size_type index) const;

    const value_type* dataAddress() const noexcept { return _dataAddress; }
    const value_type* dataAddress(size_type offset) const;

    template <typename T>
    const T* dataAs() const noexcept { return reinterpret_cast<const T*>(_dataAddress); }

    template <typename T>
    const T* dataAs(size_type offset) const {
        assertIndexInRange(offset, 0, this->size());
        assertIndexInRange(offset + sizeof(T), offset, this->size());

        return reinterpret_cast<const T*>(_dataAddress + offset);
    }



    /**  Create a slice/window view of this memory segment.
     *
     * @param from [in] Offset to begin the slice from.
     * @param to [in] The last element to slice to.
     *
     * @return The slice of the memory segment.
     */
    MemoryView slice(size_type from, size_type to) const;

    /**
     * Get a shallow view of this memory buffer.
     * Shallow view does not get any ownership of the memory.
     * When owner of the memory goes out of scope the view will become invalid.
     *
     * @return A memory view without ownership of the memory.
     */
    MemoryView viewImmutableShallow() const;

private:

    size_type                   _size{};
    byte const*                 _dataAddress{nullptr};
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
inline MemoryView wrapMemory(void const* data, MemoryView::size_type size) { return {data, size}; }

inline MemoryView wrapMemory(byte const* data, MemoryView::size_type size) { return {data, size}; }

inline MemoryView wrapMemory(char const* data, MemoryView::size_type size) { return {data, size}; }

template<typename PodType, size_t N>
inline MemoryView wrapMemory(PodType const (&data)[N]) {
    return wrapMemory(static_cast<void const*>(data), N * sizeof(PodType));
}


inline void swap(MemoryView& a, MemoryView& b) {
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



std::ostream& operator<< (std::ostream& ostr, MemoryView const& str);


}  // End of namespace Solace
#endif  // SOLACE_MEMORYVIEW_HPP
