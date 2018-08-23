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
#include "solace/mutableMemoryView.hpp"
#include "solace/memoryManager.hpp"  // TODO(abbyssoul): Allocate memory via memory manager

#include "solace/traits/callable.hpp"


namespace Solace {

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
    constexpr Vector(MemoryBuffer&& buffer) noexcept
        : _buffer(std::move(buffer))
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
//        auto const start = assertIndexInRange(from, 0, _position);
//        auto const iend = assertIndexInRange(to, from, _position);

        return view()
                .slice(from, to);
    }

    ArrayView<T> slice(size_type from, size_type to) {
        return view()
                .slice(from, to);
    }

    pointer data() noexcept {
        return _buffer.view().dataAddress();
    }

    const_pointer data() const noexcept {
        return _buffer.view().dataAddress();
    }

    ArrayView<const T> view() const noexcept {
        return ArrayView<const T>(_buffer.view(), _position);
    }

    ArrayView<T> view() noexcept {
        return {_buffer.view().slice(0, _position * sizeof(value_type))};
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

protected:

    inline void dispose() {
        auto v = view();
        for (auto i : v) {
            i.~T();
        }

        // Make sure that if an exception is thrown,
        // we are left with a null ptr, so we won't possibly dispose again.
//        auto viewCopy = _data;

//        if (_data != nullptr && _disposer != nullptr) {
//            _data = nullptr;
//            _disposer->dispose(&viewCopy);
//        }
    }

    template <typename U>
    friend class Array;
    template <typename U>
    friend class ArrayBuilder;

private:
    MemoryBuffer                _buffer;
    size_type                   _position{0};
};



/**
 * Factory function
 */
template<typename T>
Vector<T> makeVector(typename Vector<T>::size_type size) {
    return { getSystemHeapMemoryManager().create(size*sizeof(T)) };
}

}  // End of namespace Solace
#endif  // SOLACE_VECTOR_HPP
