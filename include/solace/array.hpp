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
 * libSolace
 *  @brief		Fixed size array container
 *	@file		solace/array.hpp
 ******************************************************************************/
#pragma once
#ifndef SOLACE_ARRAY_HPP
#define SOLACE_ARRAY_HPP

#include "solace/types.hpp"

#include "solace/assert.hpp"
#include "solace/arrayView.hpp"
#include "solace/mutableMemoryView.hpp"
#include "solace/memoryManager.hpp"

#include "solace/traits/callable.hpp"
#include "solace/details/array_utils.hpp"


namespace Solace {

/** Fixed-size indexed collection of elements aka Array.
 * Array is a collection that has a fixed number of elements specified at the time of its creation.
 * Each element can be accessed via its index. As such array is an associative container that maps index -> element.
 * Note: unlike vector, all elents of the array are constructed in the moment of array creation.
 *
 * Array destroys elements when collection is desctoryed.
 */
template<typename T>
class Array {
public:
    using ViewType = ArrayView<T>;

    using value_type = T;
    using size_type = typename ViewType::size_type;

    using Iterator = typename ViewType::Iterator;
    using const_iterator = typename ViewType::const_iterator;

    using reference = typename ViewType::reference;
    using const_reference = typename ViewType::const_reference;

    using pointer = typename ViewType::pointer_type;
    using const_pointer = typename ViewType::const_pointer;

public:

    inline ~Array() { dispose(); }

    constexpr explicit Array(decltype(nullptr)) noexcept
    {}

    /** Construct an empty array */
    constexpr Array() noexcept = default;

    /** Construct a new array by moving content of a given array */
    constexpr Array(Array<T>&& rhs) noexcept
        : _buffer(std::move(rhs._buffer))
        , _size(std::exchange(rhs._size, 0))
    {
    }

    /** */
    /*constexpr */Array(MemoryResource buffer, size_type len) noexcept
        : _buffer(std::move(buffer))
        , _size(len)
    {
    }

public:

    Array<T>& swap(Array<T>& rhs) noexcept {
        using std::swap;
        swap(_buffer, rhs._buffer);
        swap(_size, rhs._size);

        return (*this);
    }

    inline Array& operator= (decltype(nullptr)) {
      dispose();

      return *this;
    }

    Array<T>& operator= (Array<T>&& rhs) noexcept {
        return swap(rhs);
    }


    bool equals(Array<T> const& other) const noexcept {
        return ((&other == this) ||
                (view() == other.view()));
    }

    bool equals(std::initializer_list<T> other) const noexcept {
        if (size() != other.size()) {
            return false;
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

    /**
     * Check if this collection is empty.
     * @return True is this is an empty collection.
     */
    constexpr bool empty() const noexcept {
        return _buffer.empty();
    }

    /**
     * Get the number of elements in this array
     * @return The size of this finite collection
     */
    constexpr size_type size() const noexcept {
        return _size;
    }

    const_reference operator[] (size_type index) const {
        return view()[index];
    }

    reference operator[] (size_type index) {
        return view()[index];
    }

    template<typename F>
    Array& set(size_type index, F&& f) {
        view().set(index, std::forward<F>(f));

        return *this;
    }


    /**
     * Return iterator to beginning of the collection
     * @return iterator to beginning of the collection
     */
    const_iterator begin() const noexcept {
        return view().begin();
    }

    Iterator begin() noexcept {
        return view().begin();
    }

    /**
     * Return iterator to end of the collection
     * @return iterator to end of the collection
     */
    const_iterator end() const noexcept {
        return view().end();
    }

    Iterator end() noexcept {
        return view().end();
    }

    Array<const T> slice(size_type from, size_type to) const {
        return view().slice(from, to);
    }

    Array<T> slice(size_type from, size_type to) {
        return view().slice(from, to);
    }

    pointer data() noexcept {
        return view().data();
    }

    const_pointer data() const noexcept {
        return view().data();
    }

    ArrayView<const T> view() const noexcept {
        return arrayView<T const>(_buffer.view(), _size);
    }

    ArrayView<T> view() noexcept {
        return arrayView<T>(_buffer.view(), _size);
    }

    bool contains(const_reference value) const noexcept {
        return view().contains(value);
    }

    Optional<size_type> indexOf(const_reference value) const noexcept {
        return view().indexOf(value);
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
    Array<T>& >
    forEach(F&& f) {
        view().forEach(std::forward<F>(f));

        return *this;
    }

    template<typename F>
    std::enable_if_t<isCallable<F, const T&>::value, const Array<T>& >
    forEach(F&& f) const {
        view().forEach(std::forward<F>(f));

        return *this;
    }

    template<typename F>
    std::enable_if_t<
            isCallable<F, size_type, const T&>::value,
    const Array<T>& >
    forEach(F&& f) const {
        view().forEach(std::forward<F>(f));

        return *this;
    }


    template<typename F>
    std::enable_if_t<
            isCallable<F, size_type, T>::value ||
            isCallable<F, size_type, T&>::value,
    Array<T>& >
    forEach(F&& f) {
        view().forEach(std::forward<F>(f));

        return *this;
    }

/*
    template<typename F,
             typename R = typename std::result_of<F(T)>::type>
    Array<R> map(F&& f) const {
        auto const thisSize = size();
        typename Array<R>::Storage mappedStorage;  // NOTE: No default size here as it will insert that many elements.
        mappedStorage.reserve(thisSize);

        for (auto const& x : _storage) {
            mappedStorage.emplace_back(f(x));
        }

        return mappedStorage;
    }

    template <typename F,
              typename R = typename std::result_of<F(size_type, T)>::type>
    Array<R> mapIndexed(F&& f) const {
        const size_type thisSize = size();
        typename Array<R>::Storage mappedStorage;  // NOTE: No default size here as it will insert that many elements.
        mappedStorage.reserve(thisSize);

        for (size_type i = 0; i < thisSize; ++i) {
            mappedStorage.emplace_back(f(i, _storage[i]));
        }

        return mappedStorage;
    }
*/

protected:

    inline void dispose() {
        auto v = view();
        for (auto& i : v) {
            dtor(i);
        }
    }

    template <typename U>
    friend class Array;

private:
    MemoryResource      _buffer;
    size_type           _size{0};
};


template<typename T>
void swap(Array<T>& lhs, Array<T>& rhs) noexcept {
    lhs.swap(rhs);
}

/** Construct an default-initialized array of T of a given fixed size */
template <typename T>
Array<T> makeArray(typename Array<T>::size_type initialSize) {
    auto buffer = getSystemHeapMemoryManager().allocate(initialSize*sizeof(T));           // May throw

    initArray<T>(buffer.view(), initialSize);

    return {std::move(buffer), initialSize};  // No except c-tor
}


/** Construct a new array from a C-style array */
template <typename T>
Array<T> makeArray(typename Array<T>::size_type initialSize, T const* carray) {
    auto const arraySize = initialSize;
    auto buffer = getSystemHeapMemoryManager().allocate(arraySize*sizeof(T));           // May throw

    ArrayView<const T> src = arrayView(carray, arraySize);                                  // No except
    ArrayView<T> dest = arrayView<T>(buffer.view());       // May throw

    CopyConstructArray_<RemoveConst<T>, Decay<T*>, false>::apply(dest, src);    // May throw if copy-ctor throws

    return {std::move(buffer), arraySize};                                 // No except
}


/** Create a copy of the given array */
template <typename T>
Array<T> makeArray(ArrayView<T> other) {
    return makeArray(other.size(), other.data());
}

/** Create a copy of the given array */
template <typename T>
Array<T> makeArray(Array<T> const& other) {
    return makeArray(other.size(), other.data());
}

/**
 * Create a single element array.
 * (A degenerate case of variable argument list)
 */
template <typename T>
Array<T> makeArray(T&& arg) {
    const typename Array<T>::size_type arraySize = 1;
    auto buffer = getSystemHeapMemoryManager().allocate(arraySize*sizeof(T));           // May throw

    if (std::is_nothrow_default_constructible<T>::value) {
        auto pos = buffer.view().template dataAs<T>();
        for (size_t i = 0; i < arraySize; ++i) {
            ctor(*pos++, std::forward<T>(arg));
        }
    } else {
        ExceptionGuard<T> guard(buffer.view().template dataAs<T>());
        for (size_t i = 0; i < arraySize; ++i) {
            ctor(*guard.pos, std::forward<T>(arg));
            ++guard.pos;  // Important to be on a different line, in case of exception.
        }
        guard.release();
    }

    return {std::move(buffer), arraySize};  // No except c-tor
}

template<typename T, typename A>
void arrayConstructNoThrow(T* &pos, A&& a) noexcept {
    ctor(*pos++, std::forward<A>(a));
}

template<typename T, typename A, typename...Args>
void arrayConstructNoThrow(T* &pos, A&& a, Args&&...args) noexcept {
    arrayConstructNoThrow(pos, std::forward<A>(a));

    // recurse:
    arrayConstructNoThrow(pos, std::forward<Args>(args)...);
}


template<typename T, typename A>
void arrayConstructRecursive(ExceptionGuard<T>& guard, A&& a) {
    ctor(*guard.pos, std::forward<A>(a));
    ++guard.pos;  // Important to be on a different line, in case of exception.
}

template<typename T, typename A, typename...Args>
void arrayConstructRecursive(ExceptionGuard<T>& guard, A&& a, Args&&...args) {
    arrayConstructRecursive(guard, std::forward<A>(a));

    // recurse:
    arrayConstructRecursive(guard, std::forward<Args>(args)...);
}


template <typename T, typename...Args>
Array<T> makeArray(T&& a0, Args&&...args) {
     // Should be relativily safe to cast: we don't expect > 65k arguments
    auto const arraySize = narrow_cast<typename Array<T>::size_type>(1 + sizeof...(args));
    auto buffer = getSystemHeapMemoryManager().allocate(arraySize*sizeof(T));           // May throw

    if (std::is_nothrow_move_constructible<T>::value) {
        auto pos = buffer.view().template dataAs<T>();
        arrayConstructNoThrow(pos, std::forward<T>(a0), std::forward<Args>(args)...);
    } else {
        ExceptionGuard<T> guard(buffer.view().template dataAs<T>());
        arrayConstructRecursive(guard, std::forward<T>(a0), std::forward<Args>(args)...);
        guard.release();
    }

    return Array<T>{ std::move(buffer), arraySize };  // No except c-tor
}


template <typename T>
bool operator== (Array<T> const& lhv, decltype(nullptr)) noexcept {
    return lhv.empty();
}

template <typename T>
bool operator!= (Array<T> const& lhv, decltype(nullptr)) noexcept {
    return !lhv.empty();
}

template <typename T>
bool operator== (Array<T> const& lhv, Array<T> const& rhv) noexcept {
    return lhv.equals(rhv);
}

template <typename T>
bool operator!= (Array<T> const& lhv, Array<T> const& rhv) noexcept {
    return !lhv.equals(rhv);
}


// Static code check
static_assert(!canMemcpy<Array<char>>(), "canMemcpy<>() is broken");

}  // End of namespace Solace
#endif  // SOLACE_ARRAY_HPP
