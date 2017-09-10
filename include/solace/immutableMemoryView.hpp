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
 * libSolace: ImmutableMemoryView
 *	@file		solace/immutableMemoryView.hpp
 *	@author		$LastChangedBy: $
 *	@date		$LastChangedDate: $
 *	@brief		MemoryView object
 *	ID:			$Id: $
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IMMUTABLEMEMORYVIEW_HPP
#define SOLACE_IMMUTABLEMEMORYVIEW_HPP

#include "solace/types.hpp"
#include "solace/assert.hpp"

#include <utility>   // std::swap

namespace Solace {

// FWD declaration for disposer below.
class ImmutableMemoryView;

/**
 * Memory view disposer
 */
class MemoryViewDisposer {
public:
    virtual ~MemoryViewDisposer();

    virtual void dispose(ImmutableMemoryView* view) const = 0;
};


/* Fixed-length raw memory buffer/memory view.
 * This is a very thin abstruction on top of raw memory address -
 * it remembers memory block size and associated deallocator to free the memory if there is any.
 *
 * View has a value semantic and gives user random access to the undelying memory.
 * Howeevr ImmutableMemoryView does not allow for modification of underlaying values - it is read-only.
 * For a mutable access please use @see MemoryView
 * For the stream semantic please @see ByteBuffer
 */
class ImmutableMemoryView {
public:
    typedef uint64              size_type;
    typedef byte                value_type;

    typedef const value_type&   const_reference;
    typedef const value_type*   const_iterator;

public:

    /** Deallocate memory... maybe */
    ~ImmutableMemoryView();

    /** Construct an empty memory view */
    ImmutableMemoryView() noexcept :
        _disposer{nullptr},
        _size(0),
        _dataAddress{nullptr}
    {
    }

    ImmutableMemoryView(const ImmutableMemoryView&) = delete;
    ImmutableMemoryView& operator= (const ImmutableMemoryView&) = delete;

    ImmutableMemoryView(ImmutableMemoryView&& rhs) noexcept;

    ImmutableMemoryView& swap(ImmutableMemoryView& rhs) noexcept {
        using std::swap;

        swap(_disposer, rhs._disposer);
        swap(_size, rhs._size);
        swap(_dataAddress, rhs._dataAddress);

        return (*this);
    }


    /** Move assignment **/
    ImmutableMemoryView& operator= (ImmutableMemoryView&& rhs) noexcept {
        return swap(rhs);
    }

    bool equals(const ImmutableMemoryView& other) const noexcept {
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

    bool operator== (const ImmutableMemoryView& rhv) const noexcept {
        return equals(rhv);
    }

    bool operator!= (const ImmutableMemoryView& rhv) const noexcept {
        return !equals(rhv);
    }

    bool empty() const noexcept {
        return (_size == 0);
    }

    explicit operator bool() const noexcept {
        return (_dataAddress != nullptr);
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
    const T* dataAs(size_type offset = 0) const {
        assertIndexInRange(offset, 0, this->size());
        assertIndexInRange(offset + sizeof(T), offset, this->size() + 1);

        return reinterpret_cast<const T*>(_dataAddress + offset);
    }



    /**  Create a slice/window view of this memory segment.
     *
     * @param from [in] Offset to begin the slice from.
     * @param to [in] The last element to slice to.
     *
     * @return The slice of the memory segment.
     */
    ImmutableMemoryView slice(size_type from, size_type to) const;

    /**
     * Get a shallow view of this memory buffer.
     * Shallow view does not get any ownership of the memory.
     * When owner of the memory goes out of scope the view will become invalid.
     *
     * @return A memory view without ownership of the memory.
     */
    ImmutableMemoryView viewImmutableShallow() const;


    friend ImmutableMemoryView wrapMemory(const byte*,
                                          size_type,
                                          const MemoryViewDisposer*);

protected:

    ImmutableMemoryView(size_type size, const void* data, const MemoryViewDisposer* disposer);

private:

    const MemoryViewDisposer*   _disposer;
    size_type                   _size;
    const byte*                 _dataAddress;

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
inline ImmutableMemoryView wrapMemory(const byte* data, ImmutableMemoryView::size_type size,
                             const MemoryViewDisposer* freeFunc = 0) {
    return ImmutableMemoryView{size, data, freeFunc};
}

inline ImmutableMemoryView wrapMemory(const void* data, ImmutableMemoryView::size_type size,
                             const MemoryViewDisposer* freeFunc = 0) {
    return wrapMemory(reinterpret_cast<const byte*>(data), size, freeFunc);
}

inline ImmutableMemoryView wrapMemory(const char* data, ImmutableMemoryView::size_type size,
                             const MemoryViewDisposer* freeFunc = 0) {
    return wrapMemory(reinterpret_cast<const byte*>(data), size, freeFunc);
}

template<typename PodType, size_t N>
inline ImmutableMemoryView wrapMemory(const PodType (&data)[N],
                             const MemoryViewDisposer* freeFunc = 0)
{
    return wrapMemory(static_cast<const void*>(data), N * sizeof(PodType), freeFunc);
}


inline void swap(ImmutableMemoryView& a, ImmutableMemoryView& b) {
    a.swap(b);
}


}  // End of namespace Solace
#endif  // SOLACE_IMMUTABLEMEMORYVIEW_HPP
