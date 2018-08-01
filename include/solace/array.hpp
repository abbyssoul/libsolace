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
 * libSolace: Fixed size array template
 *	@file		solace/array.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	@brief		Fixed size array type
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_ARRAY_HPP
#define SOLACE_ARRAY_HPP

#include "solace/types.hpp"

#include "solace/assert.hpp"
#include "solace/arrayView.hpp"
#include "solace/mutableMemoryView.hpp"
#include "solace/memoryManager.hpp"  // TODO(abbyssoul): Allocate memory via memory manager


// TODO(abbyssoul): Remove std dependencies!
#include <vector>       // TODO(abbyssoul): Remove! No dynamic reallocation is needed!



namespace Solace {

/** Fixed-size indexed collection of elements aka Array.
 * Array is a collection that has a fixed number of elements specified at the time of its creation.
 * Each element can be accessed via its index. As such array is an associative container that maps index -> element.
 * Note - unlike vector, in array all elents are constructed in the moment of array creation.
 *
 * Array destroys elements when collection is desctoryed.
 */
template<typename T>
class Array {
public:
    typedef T                                   value_type;
    typedef std::vector<T> Storage;
    typedef uint32                              size_type;

    typedef typename Storage::iterator          Iterator;
    typedef typename Storage::const_iterator    const_iterator;

    typedef typename Storage::const_reference   const_reference;
    typedef typename Storage::reference         reference;

    typedef typename Storage::pointer           pointer_type;
    typedef typename Storage::const_pointer     const_pointer;

public:

    /** Construct an empty array
     * note: stl vector does not guaranty 'noexcept' even for an empty vector :'(
     * */
    Array(): _storage() {
    }

    /** Construct a new array by moving content of a given array */
    Array(Array<T>&& other) noexcept: _storage(std::move(other._storage)) {
    }

    /** Construct an array of a given fixed size */
    explicit Array(size_type initialSize): _storage(initialSize) {
        _storage.reserve(initialSize);
    }

    /** Construct a new array to be a copy of a given */
    Array(const Array<T>& other): _storage(other._storage) {
    }

    /** Construct a new array from a C-style array */
    Array(size_type len, const T* carray): _storage(carray, carray + len) {
    }

    /** */
    Array(const Storage& list): _storage(list) {
    }

    /** */
    Array(Storage&& list): _storage(std::move(list)) {
    }

    /** Construct an array from an memory buffer */
    Array(MutableMemoryView&& memView) :
        _storage(memView.dataAs<T>(), memView.dataAs<T>() + memView.size() / sizeof(T)) {
        // FIXME(abbyssoul): current implementation copies data but should use mem location.
    }

    //------------------------------------------------------------------------------------------------------------------
    // Next section is STD lib compatibility interface
    //------------------------------------------------------------------------------------------------------------------

    /** Construct an array from an initialized list */
    Array(const std::initializer_list<T>& list): _storage(list) {
    }


public:

    Array<T>& swap(Array<T>& rhs) noexcept {
        using std::swap;
        swap(_storage, rhs._storage);

        return (*this);
    }

    Array<T>& operator= (const Array<T>& rhs) noexcept {
        Array<T>(rhs).swap(*this);

        return *this;
    }

    Array<T>& operator= (Array<T>&& rhs) noexcept {
        return swap(rhs);
    }

    Array<T>& operator= (std::initializer_list<T> rhs) noexcept {
        _storage = rhs;

        return *this;
    }

    bool equals(const Array<T>& other) const noexcept {
        return ((&other == this) ||
                (_storage == other._storage));
    }

    /**
     * Overloaded operator for syntactic sugar. @see equals
     * @param rhv The object to compare this instance to.
     * @return True if this object is equal to the given
     */
    bool operator== (const Array<T>& rhv) const noexcept {
        return equals(rhv);
    }

    /**
     * Overloaded operator for syntactic sugar. @see equals
     * @param rhv The object to compare this instance to.
     * @return True if this object is NOT equal to the given.
     */
    bool operator!= (const Array<T>& rhv) const noexcept {
        return !equals(rhv);
    }

    /**
     * Check if this collection is empty.
     * @return True is this is an empty collection.
     */
    bool empty() const noexcept {
        return _storage.empty();
    }

    /**
     * Get the number of elements in this array
     * @return The size of this finite collection
     */
    size_type size() const noexcept {
        return _storage.size();
    }

    const_reference operator[] (size_type index) const {
        index = assertIndexInRange(index, 0, size());

        return _storage[index];
    }

    reference operator[] (size_type index) {
        index = assertIndexInRange(index, 0, size());

        return _storage[index];
    }

    /**
     * Return iterator to beginning of the collection
     * @return iterator to beginning of the collection
     */
    const_iterator begin() const noexcept {
        return _storage.begin();
    }

    Iterator begin() noexcept {
        return _storage.begin();
    }

    /**
     * Return iterator to end of the collection
     * @return iterator to end of the collection
     */
    const_iterator end() const noexcept {
        return _storage.end();
    }

    Iterator end() noexcept {
        return _storage.end();
    }

    const_reference first() const { return _storage.front(); }

    const_reference last() const { return _storage.back(); }

    // TODO(abbyssoul): Implement once we got read of stl::vector
//    Array<T> slice(size_type from, size_type to) const {
//        return { _storage };
//    }

    pointer_type data() noexcept {
        return _storage.data();
    }

    const_pointer data() const noexcept {
        return _storage.data();
    }

    MemoryView view() const noexcept {
        return wrapMemory(_storage.data(), _storage.size() * sizeof(T));
    }

    MutableMemoryView view() noexcept {
        return wrapMemory(_storage.data(), _storage.size() * sizeof(T));
    }

    bool contains(const_reference value) const {
        return (std::find(_storage.begin(), _storage.end(), value) != _storage.end());
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
        for (auto& x : _storage) {
            f(x);
        }

        return *this;
    }

    template<typename F>
    std::enable_if_t<isCallable<F, const T&>::value, const Array<T>& >
    forEach(F&& f) const {
        for (auto& x : _storage) {
            f(x);
        }

        return *this;
    }

    template<typename F>
    std::enable_if_t<
            isCallable<F, size_type, const T&>::value,
    const Array<T>& >
    forEach(F&& f) const {
        const auto thisSize = size();
        for (size_type i = 0; i < thisSize; ++i) {
            f(i, _storage[i]);
        }

        return *this;
    }


    template<typename F>
    std::enable_if_t<
            isCallable<F, size_type, T>::value ||
            isCallable<F, size_type, T&>::value,
    Array<T>& >
    forEach(F&& f) {
        const auto thisSize = size();
        for (size_type i = 0; i < thisSize; ++i) {
            f(i, _storage[i]);
        }

        return *this;
    }


    template<typename F,
             typename R = typename std::result_of<F(T)>::type>
    Array<R> map(F&& f) const {
        const auto thisSize = size();
        typename Array<R>::Storage mappedStorage;  // NOTE: No default size here as it will insert that many elements.
        mappedStorage.reserve(thisSize);

        for (const auto& x : _storage) {
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

private:

    Storage _storage;
};


template<typename T>
void swap(Array<T>& lhs, Array<T>& rhs) noexcept {
    lhs.swap(rhs);
}

}  // End of namespace Solace
#endif  // SOLACE_ARRAY_HPP
