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
    using value_type = T;
    using size_type = uint32;

    using Iterator = T *;
    using const_iterator = const T *;

    using reference = T &;
    using const_reference = const T &;

    using pointer_type = T *;
    using const_pointer = const T *;

public:

    /** Construct an empty array */
    constexpr ArrayView() noexcept = default;

    /** Construct an empty array */
    constexpr ArrayView(decltype(nullptr)) noexcept
    {}

    /** Construct an array from C-style array with the given size */
    constexpr ArrayView(T* ptr, size_type arraySize) noexcept :
        _memory(wrapMemory(ptr, arraySize))
    {}

    constexpr ArrayView(T* beginSeq, T* endSeq) noexcept :
        _memory(wrapMemory(beginSeq, endSeq - beginSeq))
    {}

    template <size_t size>
    constexpr ArrayView(T (&carray)[size]) noexcept :
        _memory(wrapMemory(carray))
    {}

    constexpr ArrayView(const ArrayView& other) noexcept :
        _memory(other._memory)
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

        auto selfIt = this->begin();
        auto otherIt = other.begin();
        const auto selfEnd = this->end();
        const auto otherEnd = other.end();

        for (; (selfIt != selfEnd) && (otherIt != otherEnd); ++otherIt, ++selfIt) {
            if ( !(*selfIt == *otherIt) ) {
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


    Iterator begin() noexcept {
        return _memory.empty()
                ? nullptr
                : _memory.dataAs<T>();
    }

    Iterator end() noexcept { return begin() + size(); }
    reference front() noexcept { return *begin(); }
    reference back() noexcept { return *(begin() + size() - 1); }

    const_iterator begin() const noexcept {
        return _memory.empty()
                ? nullptr
                : _memory.dataAs<T>();
    }

    const_iterator end()     const noexcept { return (begin() + size()); }
    const_reference front()  const noexcept { return *begin(); }
    const_reference back()   const noexcept { return *(begin() + size() - 1); }


    ArrayView<const T> slice(size_type from, size_type to) const {
        from   = assertIndexInRange(from, 0,     size(), "ArrayView::slice() const");
        to     = assertIndexInRange(to,   from,  size(), "ArrayView::slice() const");

        return ArrayView<const T>(begin() + from, to - from);
    }

    ArrayView slice(size_type from, size_type to) {
        from   = assertIndexInRange(from, 0,     size(), "ArrayView::slice()");
        to     = assertIndexInRange(to,   from,  size(), "ArrayView::slice()");

        return ArrayView<T>(begin() + from, to - from);
    }

    ImmutableMemoryView view() const noexcept {
        return _memory;
    }

    MemoryView view() noexcept {
        return _memory;
    }

    // cppcheck-suppress unusedFunction
    bool contains(const_reference value) const noexcept {
        return indexOf(value).isSome();
    }

    // cppcheck-suppress unusedFunction
    Optional<size_type> indexOf(const_reference value) const noexcept {
        const auto len = size();
        auto it = begin();
        for (size_type i = 0; i < len; ++i, ++it) {
            if (value == *it) {
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

    ArrayView& fill(const_reference value) noexcept {
        for (auto& v : *this) {
            v = value;
        }

        return (*this);
    }

    template<typename F>
    std::enable_if_t<isCallable<F, size_type>::value, ArrayView&>
    fill(F&& f) noexcept {
        auto it = begin();
        const auto itEnd = end();

        for (size_type i = 0; it != itEnd; ++i, ++it) {
            *it = f(i);
        }

        return (*this);
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

    /// Memory where the array data is stored.
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
