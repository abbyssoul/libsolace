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
 * libSolace:
 *  @brief		Fixed size vector container
 *	@file		solace/vector.hpp
 ******************************************************************************/
#pragma once
#ifndef SOLACE_VECTOR_HPP
#define SOLACE_VECTOR_HPP

#include "solace/types.hpp"

#include "solace/assert.hpp"
#include "solace/arrayView.hpp"
#include "solace/array.hpp"
#include "solace/mutableMemoryView.hpp"
#include "solace/memoryManager.hpp"  // TODO(abbyssoul): Allocate memory via memory manager

#include "solace/traits/callable.hpp"
#include "solace/details/array_utils.hpp"


namespace Solace {

/** Fixed size vector.
 * A collection of up-to N elements. Very similar to std::vector with the
 * key difference that all the memory is allocated upfront and never re-allocated.
 */
template<typename T>
class Vector {
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

    inline ~Vector() { dispose(); }

    /** Construct an empty array */
    constexpr Vector() noexcept = default;

    Vector(Vector const& ) = delete;
    Vector& operator= (Vector const& ) = delete;

    /** Construct a new array by moving content of a given array */
    constexpr Vector(Vector<T>&& rhs) noexcept
        : _buffer(std::move(rhs._buffer))
        , _position(std::exchange(rhs._position, 0))
    {
    }

    Vector<T>& operator= (Vector<T>&& rhs) noexcept {
        return swap(rhs);
    }

    /**
     * Construct empty vector with zero capacity.
     */
    constexpr explicit Vector(decltype(nullptr))
        : _buffer{}
        , _position{0}
    {}


    /** */
    constexpr Vector(MemoryBuffer&& buffer, size_type count) noexcept
        : _buffer(std::move(buffer))
        , _position(count)
    {
    }

public:

    Vector<T>& swap(Vector<T>& rhs) noexcept {
        using std::swap;
        swap(_buffer, rhs._buffer);
        swap(_position, rhs._position);

        return (*this);
    }

    inline Vector& operator= (decltype(nullptr)) {
      dispose();

      return *this;
    }


    bool equals(Vector<T> const& other) const noexcept {
        return ((&other == this) ||
                (view() == other.view()));
    }


    /**
     * Check if this collection is empty.
     * @return True is this is an empty collection.
     */
    constexpr bool empty() const noexcept {
        return (_position == 0);
    }

    /**
     * Get the number of elements in this array
     * @return The size of this finite collection
     */
    constexpr size_type size() const noexcept {
        return _position;
    }

    constexpr size_type capacity() const noexcept {
        return _buffer.size() / sizeof(T);
    }

    /**
     * Return iterator to beginning of the collection
     * @return iterator to beginning of the collection
     */
    const_iterator begin() const noexcept {
        return view()
                .begin();
    }

    Iterator begin() noexcept {
        return view()
                .begin();
    }

    /**
     * Return iterator to end of the collection
     * @return iterator to end of the collection
     */
    const_iterator end() const noexcept {
        return view()
                .end();
    }

    Iterator end() noexcept {
        return view()
                .end();
    }

    ArrayView<const T> slice(size_type from, size_type to) const {
        return view()
                .slice(from, to);
    }

    ArrayView<T> slice(size_type from, size_type to) {
        return view()
                .slice(from, to);
    }

    pointer data() noexcept {
        return _buffer.view().template dataAs<T>();
    }

    const_pointer data() const noexcept {
        return _buffer.view().template dataAs<T>();
    }

    ArrayView<T const> view() const noexcept {
        return arrayView<T const>(_buffer.view(), _position);
    }

    ArrayView<T> view() noexcept {
        return arrayView<T>(_buffer.view(), _position);
    }

    bool contains(const_reference value) const noexcept {
        return view().contains(value);
    }

    Optional<size_type> indexOf(const_reference value) const noexcept {
        return view().indexOf(value);
    }

    template<typename... Args>
    void emplace_back(Args... args) {
        assertIndexInRange(_position, 0, capacity());

        _buffer.view()
                .slice(_position * sizeof(value_type), (_position + 1) * sizeof(value_type))
                . template construct<value_type>(std::forward<Args>(args)...);

        _position += 1;
    }

    void push_back(T const& value)  {
        assertIndexInRange(_position, 0, capacity());

        _buffer.view()
                .slice(_position * sizeof(value_type), (_position + 1) * sizeof(value_type))
                . template construct<value_type>(value);

        _position += 1;
    }

    const_reference operator[] (size_type index) const {
        return view()[index];
    }


    /** Removes the last element of the container.
     * Last element is destroyed and containter size decremented by one.
     * @note This method modifies container size thus invalidating iterators.
     * @note No exception is thrown if element doesn't throw on destruction.
     */
    void pop_back() noexcept(std::is_nothrow_destructible<T>::value) {
        if (_position < 1) {
            return;
        }

        _position -= 1;
        _buffer.view()
                .slice(_position * sizeof(value_type), (_position+1) * sizeof(value_type))
                . template destruct<value_type>();
    }


    /**
     * Transfer content of this vector into an array
     * @return An array owning the content.
     */
    Array<T> toArray() {
        return {std::move(_buffer), std::exchange(_position, 0)};
    }

protected:

    inline void dispose() {
        auto v = view();
        for (auto& i : v) {
            dtor(i);
        }
    }

    template <typename U>
    friend class Array;
    template <typename U>
    friend class ArrayBuilder;

private:
    MemoryBuffer                _buffer;
    size_type                   _position{0};
};


template<typename T>
bool operator== (Vector<T> const& v, decltype(nullptr)) noexcept { return v.empty(); }
template<typename T>
bool operator!= (Vector<T> const& v, decltype(nullptr)) noexcept { return !v.empty(); }
template<typename T>
bool operator== (decltype(nullptr), Vector<T> const& v) noexcept { return v.empty(); }
template<typename T>
bool operator!= (decltype(nullptr), Vector<T> const& v) noexcept { return !v.empty(); }

template<typename T>
bool operator== (Vector<T> const& v, Vector<T> const& other) noexcept { return v.equals(other); }
template<typename T, typename V>
bool operator== (Vector<T> const& v, Vector<V> const& other) noexcept { return v.equals(other); }
template<typename T>
bool operator!= (Vector<T> const& v, Vector<T> const& other) noexcept { return !v.equals(other); }
template<typename T, typename V>
bool operator!= (Vector<T> const& v, Vector<V> const& other) noexcept { return !v.equals(other); }


template<typename T>
bool operator== (Vector<T> const& v, ArrayView<T> const& other) noexcept { return v.view().equals(other); }
template<typename T>
bool operator== (ArrayView<T> const& other, Vector<T> const& v) noexcept { return v.view().equals(other); }

template<typename T>
bool operator!= (Vector<T> const& v, ArrayView<T> const& other) noexcept { return !v.view().equals(other); }
template<typename T>
bool operator!= (ArrayView<T> const& other, Vector<T> const& v) noexcept { return !v.view().equals(other); }

template <typename T, typename U>
bool operator== (Vector<T> const& v, const ArrayView<U>& other) noexcept {
    return v.view().equals(other);
}

template <typename T, typename U>
bool operator!= (Vector<T> const& v, const ArrayView<U>& other) noexcept {
    return !v.view().equals(other);
}



/**
 * Vector factory function.
 * @return A newly constructed empty vector of the required capacity.
 */
template<typename T>
Vector<T> makeVector(typename Vector<T>::size_type size) {
    return { getSystemHeapMemoryManager().allocate(size*sizeof(T)), 0 };
}

/** Construct a new vector from a C-style array */
template <typename T>
Vector<T> makeVector(T const* carray, typename Vector<T>::size_type len) {
    auto buffer = getSystemHeapMemoryManager().allocate(len*sizeof(T));           // May throw

    ArrayView<const T> src = arrayView(carray, len);                            // No except
    ArrayView<T> dest = arrayView<T>(buffer.view());                            // No except

    CopyConstructArray_<RemoveConst<T>, Decay<T*>, false>::apply(dest, src);    // May throw if copy-ctor throws

    return { std::move(buffer), len };                                 // No except
}


/** Create a copy of the given vector */
template <typename T>
Vector<T> makeVector(Vector<T> const& other) {
    return makeVector(other.data(), other.size());
}

/**
 * Vector factory function.
 * @return A newly constructed vector with given items.
 */
template <typename T>
Vector<T> makeVector(std::initializer_list<T> list) {
    // FIXME: Should be checked cast
    auto const vectorSize = narrow_cast<typename Vector<T>::size_type>(list.size());

    auto buffer = getSystemHeapMemoryManager().allocate(vectorSize * sizeof(T));  // May throw
    if (std::is_nothrow_copy_constructible<T>::value) {
        auto pos = buffer.view().template dataAs<T>();
        for (T const& i : list) {
            ctor(*pos++, i);
        }
    } else {
        ExceptionGuard<T> guard(buffer.view().template dataAs<T>());
        for (T const& i : list) {
            ctor(*guard.pos, i);
            ++guard.pos;
        }
        guard.release();
    }

    return {std::move(buffer), vectorSize};                                 // No except
}


}  // End of namespace Solace
#endif  // SOLACE_VECTOR_HPP
