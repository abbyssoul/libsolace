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

#include "solace/utils.hpp"
#include "solace/assert.hpp"
#include "solace/memoryView.hpp"
#include "solace/optional.hpp"

#include <initializer_list>
#include <algorithm>    // std::swap, std::find


namespace Solace {

/**
 * A wrapper for c-style arrays.
 * This array does not own the underlaying memory.
 * Passed by value copies the pointer, not the data.
 */
template <typename T>
class ArrayView {
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
    constexpr ArrayView() noexcept
    {}

    constexpr ArrayView(decltype(nullptr)) noexcept
    {}

    constexpr ArrayView(T* ptr, size_type arraySize) noexcept :
        _memory(wrapMemory(ptr, arraySize))
    {}

    constexpr ArrayView(T* begin, T* end) noexcept :
        _memory(wrapMemory(begin, end - begin))
    {}

    template <size_t size>
    constexpr ArrayView(T (&carray)[size]) noexcept :
        _memory(wrapMemory(carray))
    {}

    constexpr ArrayView(const ArrayView& other) noexcept :
        _memory(other._memory.viewShallow())
    {}

    constexpr ArrayView(MemoryView&& memview) noexcept :
        _memory(std::move(memview))
    {}

public:

    ArrayView<T>& swap(ArrayView<T>& rhs) noexcept {
        using std::swap;
        swap(_memory, rhs._memory);

        return (*this);
    }

    ArrayView& operator= (const ArrayView& rhs) noexcept {
        ArrayView(rhs).swap(*this);

        return *this;
    }

    ArrayView<T>& operator= (ArrayView&& rhs) noexcept {
        return swap(rhs);
    }

    bool equals(const ArrayView& other) const noexcept {
        if (size() != other.size()) {
            return false;
        }

        if ((&other == this) || (_memory.dataAddress() == other._memory.dataAddress())) {
            return true;
        }

        const T* self = _memory.dataAs<T>();
        const T* that = _memory.dataAs<T>();

        for (size_type i = 0; i < size(); ++i) {
          if (self[i] != that[i]) {
              return false;
          }
        }

        return true;
    }

    bool operator== (decltype(nullptr)) const noexcept { return empty(); }
    bool operator!= (decltype(nullptr)) const noexcept { return !empty(); }

    bool operator== (const ArrayView& other) const noexcept {
        return equals(other);
    }

    bool operator!= (const ArrayView& other) const noexcept {
        return !equals(other);
    }

    template <typename U>
    bool operator== (const ArrayView<U>& other) const {
        return equals(other);
    }

    template <typename U>
    bool operator!= (const ArrayView<U>& other) const noexcept {
        return !equals(other);
    }

    /**
     * Check if this collection is empty.
     * @return True is this is an empty collection.
     */
    bool empty() const noexcept {
        return _memory.empty();
    }

    /**
     * Get the number of elements in this array
     * @return number of elements in this collection.
     */
    size_type size() const noexcept {
        return _memory.size() / sizeof(T);
    }


    const_reference operator[] (size_type index) const {
        index = assertIndexInRange(index, 0, size(), "ArrayView[] const");

        return _memory.dataAs<T>()[index];
    }


    reference operator[] (size_type index) {
        index = assertIndexInRange(index, 0, size(), "ArrayView[]");

        return _memory.dataAs<T>()[index];
    }


    Iterator begin()     {
        return _memory.empty()
                ? nullptr
                : _memory.dataAs<T>();
    }

    Iterator end()       { return begin() + size(); }
    reference front()    { return *begin(); }
    reference back()     { return *(begin() + size() - 1); }

    const_iterator begin()   const {
        return _memory.empty()
                ? nullptr
                : _memory.dataAs<T>();
    }

    const_iterator end()     const { return (begin() + size()); }
    const_reference front()  const { return *begin(); }
    const_reference back()   const { return *(begin() + size() - 1); }


    ArrayView<const T> slice(size_type start, size_type end) const {
        start   = assertIndexInRange(start, 0,      size(), "ArrayView::slice() const");
        end     = assertIndexInRange(end,   start,  size(), "ArrayView::slice() const");

        return ArrayView<const T>(begin() + start, end - start);
    }

    ArrayView slice(size_type start, size_type end) {
        start   = assertIndexInRange(start, 0,      size(), "ArrayView::slice()");
        end     = assertIndexInRange(end,   start,  size(), "ArrayView::slice()");

        return ArrayView<T>(begin() + start, end - start);
    }

    ImmutableMemoryView view() const noexcept {
        return _memory;
    }

    MemoryView view() noexcept {
        return _memory;
    }

    bool contains(const_reference value) const noexcept {
        return indexOf(value).isSome();
    }

    Optional<size_type> indexOf(const_reference value) const noexcept {
        const auto it = begin();
        for (size_type i = 0; i < size(); ++i) {
            if (value == it[i]) {
                return Optional<size_type>::of(i);
            }
        }

        return None();
    }

    ArrayView<const T> asConst() const noexcept {
      return ArrayView<const T>(_memory);
    }

    operator ArrayView<const T>() const noexcept {
      return asConst();
    }

    void fill(const_reference value) noexcept {
        for (auto& v : *this) {
            v = value;
        }
    }

    template<typename F>
    std::enable_if_t<isCallable<F, size_type>::value, void>
    fill(F&& f) noexcept {
        const auto len = size();
        const auto it = begin();

        for (size_type i = 0; i < len; ++i) {
            it[i] = f(i);
        }
    }

    /*
     *--------------------------------------------------------------------------
     * Functional methods that operates on the collection without changing it.
     *--------------------------------------------------------------------------
     */

    template<typename F>
    const ArrayView& forEach(F&& f) const {
        for (const auto& x : *this) {
            f(x);
        }

        return *this;
    }


private:

    MemoryView _memory;
};




template<typename T>
void swap(ArrayView<T>& lhs, ArrayView<T>& rhs) noexcept {
    lhs.swap(rhs);
}

/** Syntactic sugar to create ArrayView without spelling out the type name. */
template <typename T>
constexpr ArrayView<T> arryaView(T* ptr, size_t size) {
  return ArrayView<T>(ptr, size);
}

/** Syntactic sugar to create ArrayView without spelling out the type name. */
template <typename T, size_t N>
constexpr ArrayView<T> arrayView(T (&carray)[N]) {
  return ArrayView<T>(carray);
}


/** Syntactic sugar to create ArrayView without spelling out the type name. */
template <typename T>
constexpr ArrayView<T> arrayView(T* begin, T* end) {
  return ArrayView<T>(begin, end);
}

}  // End of namespace Solace
#endif  // SOLACE_ARRAYREF_HPP
