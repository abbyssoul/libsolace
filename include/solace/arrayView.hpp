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
 *	@file		solace/arrayView.hpp
 ******************************************************************************/
#pragma once
#ifndef SOLACE_ARRAYVIEW_HPP
#define SOLACE_ARRAYVIEW_HPP

#include "solace/assert.hpp"
#include "solace/mutableMemoryView.hpp"
#include "solace/optional.hpp"

#include "solace/traits/callable.hpp"  // isCallable


namespace Solace {

/**
 * A wrapper for c-style arrays.
 * This array does not own the underlaying memory.
 * Passed by value copies the pointer, not the data.
 */
template <typename T>
class ArrayView {
public:
    using ViewType = typename std::conditional<std::is_const<T>::value, MemoryView, MutableMemoryView>::type;

    using value_type = T;
    using size_type = uint32;

    using Iterator = T *;
    using const_iterator = T const *;

    using reference = T &;
    using const_reference = T const&;

    using pointer_type = T *;
    using const_pointer = T const *;

public:

    ~ArrayView() noexcept = default;

    /** Construct an empty array */
    constexpr ArrayView() noexcept = default;

    /** Construct an empty array */
    constexpr ArrayView(decltype(nullptr)) noexcept
    {}

    constexpr ArrayView(ArrayView const& other) noexcept
        : _memory(other._memory)
    {}

    constexpr ArrayView(ArrayView&& other) noexcept
        : _memory(std::move(other._memory))
    {}

    /** Construct an array from C-style array with the given size */
    constexpr ArrayView(T* ptr, size_type arraySize)
        : _memory{wrapMemory(ptr, sizeof(T) * arraySize)}
    {}

    constexpr ArrayView(T* beginSeq, T* endSeq) noexcept
        : _memory{wrapMemory(beginSeq, sizeof(T) * (endSeq - beginSeq))}
    {}

    template <size_t size>
    constexpr ArrayView(T (&carray)[size]) noexcept
        : _memory{wrapMemory(carray)}
    {}

    constexpr ArrayView(ViewType memview) noexcept :
        _memory(std::move(memview))
    {}

public:

    ArrayView<T>& swap(ArrayView<T>& rhs) noexcept {
        _memory.swap(rhs._memory);

        return (*this);
    }

    ArrayView& operator= (ArrayView const& rhs) noexcept {
        ArrayView(rhs).swap(*this);

        return *this;
    }

    ArrayView<T>& operator= (ArrayView&& rhs) noexcept {
        return swap(rhs);
    }

    bool equals(ArrayView const& other) const noexcept {
        if (size() != other.size()) {
            return false;
        }

        if ((&other == this) || (_memory.dataAddress() == other._memory.dataAddress())) {
            return true;
        }

        auto selfIt = this->begin();
        auto otherIt = other.begin();
        auto const selfEnd = this->end();
        auto const otherEnd = other.end();

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
    constexpr bool empty() const noexcept {
        return _memory.empty();
    }

    /**
     * Get the number of elements in this array
     * @return number of elements in this collection.
     */
    constexpr size_type size() const noexcept {
        return _memory.size() / sizeof(T);
    }


    const_reference operator[] (size_type index) const {
        index = assertIndexInRange(index, 0, size(), "ArrayView[] const");

        return _memory.template dataAs<T>()[index];
    }


    reference operator[] (size_type index) {
        index = assertIndexInRange(index, 0, size(), "ArrayView[]");

        return _memory.template dataAs<T>()[index];
    }


    Iterator begin() noexcept {
        return _memory.empty()
                ? nullptr
                : _memory.template dataAs<T>();
    }

    Iterator end() noexcept { return begin() + size(); }

    const_iterator begin() const noexcept {
        return _memory.empty()
                ? nullptr
                : _memory.template dataAs<T>();
    }

    const_iterator end()     const noexcept { return (begin() + size()); }

    const_pointer data() const noexcept { return begin(); }

    ArrayView<const T>
    slice(size_type from, size_type to) const noexcept {
        return {_memory.slice(from*sizeof(T), to*sizeof(T))};
    }

    ArrayView slice(size_type from, size_type to) noexcept {
        return {_memory.slice(from*sizeof(T), to*sizeof(T))};
    }

    constexpr MemoryView view() const & noexcept {
        return _memory;
    }

    template<class Q = T>
    std::enable_if_t<!std::is_const<Q>::value, MutableMemoryView>
    view() & noexcept {
        return _memory;
    }

    template<typename F, class Q = T>
    std::enable_if_t<!std::is_const<Q>::value, void>
    set(size_type index, F&& f) {
        index = assertIndexInRange(index, 0, size(), "ArrayView.set()");

        _memory.template dataAs<T>()[index] = f();
    }


    /**
     * Construct a new object in-place. Old value gets destroyed first
     */
    template<class Q = T, typename...Args>
    std::enable_if_t<!std::is_const<Q>::value, void>
    emplace(size_type index, Args&&...args) {
        index = assertIndexInRange(index, 0, size(), "ArrayView.emplace()");

        auto memBlock = _memory.template sliceFor<T>(index);
        memBlock.template destruct<T>();
        memBlock.template construct<T>(std::forward<Args>(args)...);
    }

    /**
     * Construct a new object in-place without destroying old value.
     * @note This is a dangereous opeartion tha can lead to memory leak if
     * used on already initialised storage.
     */
    template<class Q = T, typename...Args>
    std::enable_if_t<!std::is_const<Q>::value, void>
    _emplace_unintialized(size_type index, Args&&...args) {
        index = assertIndexInRange(index, 0, size(), "ArrayView.emplace()");

        _memory .template sliceFor<T>(index)
                .template construct<T>(std::forward<Args>(args)...);
    }


    template<typename...Args>
    void emplaceAll(Args&&... args);

//    template<typename F, typename...Args>
//    void emplaceAllWith(F&& f, Args&&... args);

    bool contains(const_reference value) const noexcept {
        return indexOf(value).isSome();
    }

    Optional<size_type> indexOf(const_reference value) const noexcept {
        auto const len = size();
        auto it = begin();
        for (size_type i = 0; i < len; ++i, ++it) {
            if (value == *it) {
                return Optional<size_type>(i);
            }
        }

        return none;
    }

    constexpr ArrayView<T const> asConst() const noexcept {
      return ArrayView<T const>(_memory);
    }

    constexpr operator ArrayView<T const>() const noexcept {
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
        auto const itEnd = end();

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
    std::enable_if_t<
                isCallable<F, T>::value ||
                isCallable<F, T&>::value,
    ArrayView<T>& >
    forEach(F&& f) {
        for (auto& x : *this) {
            f(x);
        }

        return *this;
    }

    template<typename F>
    std::enable_if_t<isCallable<F, const T&>::value,
    const ArrayView<T>& >
    forEach(F&& f) const {
        for (const auto& x : *this) {
            f(x);
        }

        return *this;
    }

    template<typename F>
    std::enable_if_t<
            isCallable<F, size_type, const T&>::value,
    const ArrayView<T>& >
    forEach(F&& f) const {
        auto const thisSize = size();
        auto const pData = _memory.template dataAs<T>();

        for (size_type i = 0; i < thisSize; ++i) {
            f(i, pData[i]);
        }

        return *this;
    }


    template<typename F>
    std::enable_if_t<
            isCallable<F, size_type, T>::value ||
            isCallable<F, size_type, T&>::value,
    ArrayView<T>& >
    forEach(F&& f) {
        auto const thisSize = size();
        auto const pData = _memory.template dataAs<T>();

        for (size_type i = 0; i < thisSize; ++i) {
            f(i, pData[i]);
        }

        return *this;
    }

private:

    /// Memory where the array data is stored.
    ViewType _memory;
};


template<typename T>
struct ArrayExceptionGuard {
    using value_type = T;
    using ArrayType = ArrayView<T>;
    using Iterator = typename ArrayType::Iterator;
    using const_iterator = typename ArrayType::const_iterator;

    const_iterator start;
    Iterator pos;

    inline ~ArrayExceptionGuard() noexcept(false) {
        while (pos > start) {
            dtor(*--pos);
        }
    }

    constexpr explicit ArrayExceptionGuard(ArrayView<T>& a) noexcept : start{a.begin()}, pos{a.begin()} {}
    constexpr void release() noexcept { start = pos; }


    template <typename V>
    void emplace(V&& v) {
        ctor(*pos, std::forward<V>(v));
        ++pos;
    }
};

template <typename T>
template<typename...Args>
void ArrayView<T>::emplaceAll(Args&&... args) {
    ArrayExceptionGuard<T> valuesGuard(*this);

    (valuesGuard.emplace(std::forward<Args>(args)), ...);

    valuesGuard.release();
}


template<typename T>
void swap(ArrayView<T>& lhs, ArrayView<T>& rhs) noexcept {
    lhs.swap(rhs);
}


/** ArrayView factory */
template <typename T>
[[nodiscard]] constexpr ArrayView<T const> arrayView(MemoryView memView) noexcept {
  return ArrayView<T const>(memView);
}

template <typename T>
[[nodiscard]]
constexpr ArrayView<T const> arrayView(MemoryView memView, typename ArrayView<T const>::size_type len) noexcept {
  return ArrayView<T const>(memView.slice(0, len * sizeof(T)));
}

template <typename T>
[[nodiscard]] constexpr ArrayView<T> arrayView(MutableMemoryView memView) noexcept {
  return ArrayView<T>(memView);
}

template <typename T>
[[nodiscard]]
constexpr ArrayView<T> arrayView(MutableMemoryView memView, typename ArrayView<T>::size_type len) noexcept {
  return ArrayView<T>(memView.slice(0, len * sizeof(T)));
}


/** Syntactic sugar to create ArrayView without spelling out the type name. */
template <typename T>
[[nodiscard]] constexpr ArrayView<T> arrayView(T* ptr, size_t size) noexcept {
  return ArrayView<T>(ptr, size);
}

/** Syntactic sugar to create ArrayView without spelling out the type name. */
template <typename T>
[[nodiscard]] constexpr ArrayView<const T> arrayView(T const* ptr, size_t size) noexcept {
  return ArrayView<const T>(ptr, size);
}

/** Syntactic sugar to create ArrayView without spelling out the type name. */
template <typename T, size_t N>
[[nodiscard]] constexpr ArrayView<T> arrayView(T (&carray)[N]) noexcept {
  return ArrayView<T>(carray);
}


/** Syntactic sugar to create ArrayView without spelling out the type name. */
template <typename T>
[[nodiscard]] constexpr ArrayView<T> arrayView(T* begin, T* end) noexcept {
  return ArrayView<T>(begin, end);
}


}  // End of namespace Solace
#endif  // SOLACE_ARRAYVIEW_HPP
