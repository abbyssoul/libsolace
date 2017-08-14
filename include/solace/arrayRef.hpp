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
 * libSolace: A OOP wrapper for C-style arrays
 *	@file		solace/arrayRef.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_ARRAYREF_HPP
#define SOLACE_ARRAYREF_HPP

#include "solace/assert.hpp"
#include "solace/memoryView.hpp"
#include "solace/optional.hpp"

#include <initializer_list>
#include <algorithm>    // std::swap, std::find


namespace Solace {

/**
 * A wrapper for c-style arrays. It includes array length and does not own the data.
 * Can be passed by value by copies the pointer, not the target.
 */
template <typename T>
class ArrayPtr {
public:
    typedef T           value_type;
    typedef uint32      size_type;

    typedef T*          Iterator;
    typedef const T*    const_iterator;

    typedef T&          reference;
    typedef const T&    const_reference;

    typedef T*          pointer_type;
    typedef const T*    const_pointer;

public:

    /** Construct an empty array */
    inline constexpr ArrayPtr() noexcept :
        _ptr(nullptr), _size(0)
    {}

    inline constexpr ArrayPtr(decltype(nullptr)) noexcept :
        _ptr(nullptr), _size(0)
    {}

    inline constexpr ArrayPtr(T* ptr, size_type size) noexcept :
        _ptr(ptr), _size(size)
    {}

    inline constexpr ArrayPtr(T* begin, T* end) noexcept :
        _ptr(begin), _size(end - begin)
    {}


    inline ArrayPtr(const std::initializer_list<T> initList) noexcept :
        _ptr(initList.begin()), _size(initList.size())
    {}

    template <size_t size>
    inline constexpr ArrayPtr(T (&carray)[size]) noexcept :
        _ptr(carray), _size(size)
    {}

public:

    ArrayPtr<T>& swap(ArrayPtr<T>& rhs) noexcept {
        using std::swap;
        swap(_ptr, rhs._ptr);
        swap(_size, rhs._size);

        return (*this);
    }

    ArrayPtr<T>& operator= (const ArrayPtr<T>& rhs) noexcept {
        ArrayPtr<T>(rhs).swap(*this);

        return *this;
    }

    ArrayPtr<T>& operator= (ArrayPtr&& rhs) noexcept {
        return swap(rhs);
    }

    inline bool equals(const ArrayPtr& other) const noexcept {
        if (_size != other._size) {
            return false;
        }

        if ((&other == this) || (_ptr == other._ptr)) {
            return true;
        }

        for (size_type i = 0; i < _size; ++i) {
          if (_ptr[i] != other._ptr[i]) {
              return false;
          }
        }

        return true;
    }

    inline bool operator== (decltype(nullptr)) const noexcept { return _size == 0; }
    inline bool operator!= (decltype(nullptr)) const noexcept { return _size != 0; }

    inline bool operator== (const ArrayPtr& other) const noexcept {
        return equals(other);
    }

    inline bool operator!= (const ArrayPtr& other) const noexcept {
        return !equals(other);
    }

    template <typename U>
    inline bool operator== (const ArrayPtr<U>& other) const {
        return equals(other);
    }

    template <typename U>
    inline bool operator!= (const ArrayPtr<U>& other) const noexcept {
        return !equals(other);
    }

    /**
     * Check if this collection is empty.
     * @return True is this is an empty collection.
     */
    bool empty() const noexcept {
        return (_size == 0);
    }

    /**
     * Get the number of elements in this array
     * @return number of elements in this collection.
     */
    inline size_type size() const noexcept {
        return _size;
    }


    inline const_reference operator[] (size_type index) const {
        index = assertIndexInRange(index, 0, _size, "ArrayPtr[] const");

        return _ptr[index];
    }


    inline reference operator[] (size_type index) {
        index = assertIndexInRange(index, 0, _size, "ArrayPtr[]");

        return _ptr[index];
    }


    inline Iterator begin()     { return _ptr; }
    inline Iterator end()       { return _ptr + _size; }
    inline reference front()    { return *_ptr; }
    inline reference back()     { return *(_ptr + _size - 1); }

    inline const_iterator begin()   const { return _ptr; }
    inline const_iterator end()     const { return (_ptr + _size); }
    inline const_reference front()  const { return *_ptr; }
    inline const_reference back()   const { return *(_ptr + _size - 1); }


    inline ArrayPtr<const T> slice(size_type start, size_type end) const {
        start   = assertIndexInRange(start, 0,      _size, "ArrayPtr::slice() const");
        end     = assertIndexInRange(end,   start,  _size, "ArrayPtr::slice() const");

        return ArrayPtr<const T>(_ptr + start, end - start);
    }

    inline ArrayPtr slice(size_type start, size_type end) {
        start   = assertIndexInRange(start, 0,      _size, "ArrayPtr::slice()");
        end     = assertIndexInRange(end,   start,  _size, "ArrayPtr::slice()");

        return ArrayPtr<const T>(_ptr + start, end - start);
    }

    inline ImmutableMemoryView view() const noexcept {
        return wrapMemory(_ptr, _size * sizeof(T));
    }

    inline MemoryView view() noexcept {
        return wrapMemory(_ptr, _size * sizeof(T));
    }

    inline bool contains(const_reference value) const noexcept {
        return (std::find(begin(), end(), value) != end());
    }

    inline Optional<size_type> indexOf(const_reference value) const noexcept {
        const auto it = std::find(begin(), end(), value);
        return (it == end())
                ? Optional<size_type>::of(std::distance(begin(), it))
                : Optional<size_type>::none();
    }

    inline ArrayPtr<const T> asConst() const noexcept {
      return ArrayPtr<const T>(_ptr, _size);
    }

    inline operator ArrayPtr<const T>() const noexcept {
      return asConst();
    }


private:
    const_pointer   _ptr;
    size_type       _size;
};




template<typename T>
void swap(ArrayPtr<T>& lhs, ArrayPtr<T>& rhs) noexcept {
    lhs.swap(rhs);
}

/** Syntactic sugar to create ArrayPtr without spelling out the type name. */
template <typename T>
inline constexpr ArrayPtr<T> arrayPtr(T* ptr, size_t size) {
  return ArrayPtr<T>(ptr, size);
}


/** Syntactic sugar to create ArrayPtr without spelling out the type name. */
template <typename T>
inline constexpr ArrayPtr<T> arrayPtr(T* begin, T* end) {
  return ArrayPtr<T>(begin, end);
}

}  // End of namespace Solace
#endif  // SOLACE_ARRAYREF_HPP
