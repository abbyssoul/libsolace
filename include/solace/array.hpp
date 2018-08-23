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
#include "solace/memoryManager.hpp"  // TODO(abbyssoul): Allocate memory via memory manager

#include "solace/traits/callable.hpp"


namespace Solace {

/**
 * Strategy to dispose of array content.
 * @see MemoryViewDisposer
 */
class ArrayDisposer {
public:
    using size_type = uint32;

    virtual ~ArrayDisposer() = default;

    template<typename T>
    void dispose(ArrayView<T>* view) const;

protected:

    virtual void disposeImpl(void* firstElement, size_t elementSize, size_type elementCount,
                             void (*destroyElement)(void*)) const = 0;

private:
    template <typename T, bool hasTrivialDestructor = __has_trivial_destructor(T)>
    struct Dispose_;
};



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

    constexpr explicit Array(decltype(nullptr)) : _data(nullptr)
    {}

    /** Construct an empty array */
    constexpr Array() noexcept = default;

    /** Construct a new array by moving content of a given array */
    constexpr Array(Array<T>&& rhs) noexcept
        : _data(std::move(rhs._data))
        , _disposer(std::exchange(rhs._disposer, nullptr))
    {
    }

    /** */
    constexpr Array(ArrayView<T> view, ArrayDisposer const& disposer) noexcept
        : _data(std::move(view))
        , _disposer(&disposer)
    {
    }

public:

    Array<T>& swap(Array<T>& rhs) noexcept {
        using std::swap;
        swap(_data, rhs._data);
        swap(_disposer, rhs._disposer);

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
                (_data == other._data));
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
        return _data.empty();
    }

    /**
     * Get the number of elements in this array
     * @return The size of this finite collection
     */
    constexpr size_type size() const noexcept {
        return _data.size();
    }

    const_reference operator[] (size_type index) const {
        return _data[index];
    }

    reference operator[] (size_type index) {
        return _data[index];
    }

    /**
     * Return iterator to beginning of the collection
     * @return iterator to beginning of the collection
     */
    const_iterator begin() const noexcept {
        return _data.begin();
    }

    Iterator begin() noexcept {
        return _data.begin();
    }

    /**
     * Return iterator to end of the collection
     * @return iterator to end of the collection
     */
    const_iterator end() const noexcept {
        return _data.end();
    }

    Iterator end() noexcept {
        return _data.end();
    }

    Array<const T> slice(size_type from, size_type to) const {
        return _data.slice(from, to);
    }

    Array<T> slice(size_type from, size_type to) {
        return _data.slice(from, to);
    }

    pointer data() noexcept {
        return _data.data();
    }

    const_pointer data() const noexcept {
        return _data.data();
    }

    ArrayView<const T> view() const noexcept {
        return _data;
    }

    ArrayView<T> view() noexcept {
        return _data;
    }

    bool contains(const_reference value) const noexcept {
        return _data.contains(value);
    }

    Optional<size_type> indexOf(const_reference value) const noexcept {
        return _data.indexOf(value);
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
        _data.forEach(std::forward<F>(f));

        return *this;
    }

    template<typename F>
    std::enable_if_t<isCallable<F, const T&>::value, const Array<T>& >
    forEach(F&& f) const {
        _data.forEach(std::forward<F>(f));

        return *this;
    }

    template<typename F>
    std::enable_if_t<
            isCallable<F, size_type, const T&>::value,
    const Array<T>& >
    forEach(F&& f) const {
        _data.forEach(std::forward<F>(f));

        return *this;
    }


    template<typename F>
    std::enable_if_t<
            isCallable<F, size_type, T>::value ||
            isCallable<F, size_type, T&>::value,
    Array<T>& >
    forEach(F&& f) {
        _data.forEach(std::forward<F>(f));

        return *this;
    }

/*
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
*/

protected:

    inline void dispose() {
        // Make sure that if an exception is thrown,
        // we are left with a null ptr, so we won't possibly dispose again.
        auto viewCopy = _data;

        if (_data != nullptr && _disposer != nullptr) {
            _data = nullptr;
            _disposer->dispose(&viewCopy);
        }
    }

    template <typename U>
    friend class Array;
    template <typename U>
    friend class ArrayBuilder;

private:

    ArrayView<T>                _data;
    ArrayDisposer const*        _disposer {nullptr};
};


template<typename T>
void swap(Array<T>& lhs, Array<T>& rhs) noexcept {
    lhs.swap(rhs);
}


class HeapArrayDisposer final: public ArrayDisposer {
public:
    using size_type = uint32;

    static const HeapArrayDisposer instance;

    template<typename T>
    static ArrayView<T> allocate(size_type count);

    template<typename T>
    static ArrayView<T> allocateUninitialized(size_type count);

    // ArrayDisposer interface
protected:

    static void* allocateImpl(size_t elementSize, size_type elementCount, size_type capacity,
                            void (*constructElement)(void*), void (*destroyElement)(void*));

    // Allocates and constructs the array.  Both function pointers are null if the constructor is
    // trivial, otherwise destroyElement is null if the constructor doesn't throw.
    void disposeImpl(void* firstElement, size_t elementSize, size_type elementCount,
                           void (*destroyElement)(void*)) const override;

    template <typename T,   bool hasTrivialConstructor = __has_trivial_constructor(T),
                            bool hasNothrowConstructor = __has_nothrow_constructor(T)>
    struct Allocate_;
};


template <typename T>
struct HeapArrayDisposer::Allocate_<T, true, true> {
    static void construct(void* ptr) {
        ctor(*reinterpret_cast<T*>(ptr));
    }

    static ArrayView<T> allocate(size_type elementCount, size_type capacity) {
        return {reinterpret_cast<T*>(allocateImpl(sizeof(T), elementCount, capacity, &construct, nullptr)), capacity};
    }
};

template <typename T>
struct HeapArrayDisposer::Allocate_<T, false, true> {
    static void construct(void* ptr) {
        ctor(*reinterpret_cast<T*>(ptr));
    }

    static ArrayView<T> allocate(size_type elementCount, size_type capacity) {
        return {reinterpret_cast<T*>(allocateImpl(sizeof(T), elementCount, capacity, &construct, nullptr)), capacity};
    }
};

template <typename T>
struct HeapArrayDisposer::Allocate_<T, false, false> {
    static void construct(void* ptr) {
        ctor(*reinterpret_cast<T*>(ptr));
    }

    static void destruct(void* ptr) {
        dtor(*reinterpret_cast<T*>(ptr));
    }

    static ArrayView<T> allocate(size_type elementCount, size_type capacity) {
        return {reinterpret_cast<T*>(allocateImpl(sizeof(T), elementCount, capacity, &construct, &destruct)), capacity};
    }
};


template <typename T>
ArrayView<T> HeapArrayDisposer::allocate(size_type count) {
    return Allocate_<T>::allocate(count, count);
}


template <typename T>
ArrayView<T> HeapArrayDisposer::allocateUninitialized(size_type count) {
    return Allocate_<T>::allocate(0, count);
}


/** Construct an array of a given fixed size */
template <typename T>
Array<T> allocArray(size_t initialSize) {
    return Array<T>{                                                // No except c-tor
                    HeapArrayDisposer::allocate<T>(initialSize),    // May throw
                    HeapArrayDisposer::instance                     // No except
                };
}



template <typename Element, typename Iterator, bool move, bool = canMemcpy<Element>()>
struct CopyConstructArray_;

template <typename T, bool move>
struct CopyConstructArray_<T, T*, move, true> {
    static inline void apply(ArrayView<T> dest, ArrayView<const T> src) {
        if (!src.empty()) {
            dest.view().write(src.view());
        }
    }
};

template <typename T>
struct CopyConstructArray_<T, const T*, false, true> {
    static inline void apply(ArrayView<T> dest, ArrayView<const T> src) {
        if (!src.empty()) {
            dest.view().write(src.view());
        }
    }
};

template <typename T, typename Iterator, bool move>
struct CopyConstructArray_<T, Iterator, move, true> {
    // Since both the copy constructor and assignment operator are trivial, we know that assignment
    // is equivalent to copy-constructing. So we can make this case somewhat easier for the
    // compiler to optimize.
    static inline T* apply(T* __restrict__ pos, Iterator start, Iterator end) {
        while (start != end) {
            *pos++ = *start++;
        }

        return pos;
    }
};


template <typename T, typename Iterator>
struct CopyConstructArray_<T, Iterator, false, false> {
    struct ExceptionGuard {
        T const* start;
        T* pos;

        ~ExceptionGuard() noexcept(false) {
            while (pos > start) {
                dtor(*--pos);
            }
        }

        constexpr explicit ExceptionGuard(T* p) noexcept : start{p}, pos{p} {}
        constexpr void release() noexcept { start = pos; }
    };

    static void apply(ArrayView<T> dest, ArrayView<const T> src) {
        auto start = src.begin();
        auto const end = src.end();

        if (noexcept(T(*start))) {
            auto pos = dest.begin();
            while (start != end) {
                ctor(*pos++, *start++);
            }
        } else {
            // Crap.  This is complicated.
            ExceptionGuard guard(dest.begin());
            while (start != end) {
                ctor(*guard.pos, *start++);
                ++guard.pos;
            }
            guard.release();
        }
    }
};

static_assert(canMemcpy<char>(), "canMemcpy<>() is broken");
static_assert(!canMemcpy<Array<char>>(), "canMemcpy<>() is broken");


/** Construct a new array from a C-style array */
template <typename T>
Array<T> allocArray(T const* carray, size_t len) {
    ArrayView<const T> src = arrayView(carray, len);                                  // No except
    ArrayView<T> dest = HeapArrayDisposer::allocateUninitialized<T>(len);       // May throw

    CopyConstructArray_<RemoveConst<T>, Decay<T*>, false>::apply(dest, src);    // May throw if copy-ctor throws

    return {dest, HeapArrayDisposer::instance};                                 // No except
}


/** Create a copy of the given array */
template <typename T>
Array<T> allocArray(Array<T> const& other) {
    return allocArray(other.data(), other.size());
}

/** Construct an array from an initialized list */
template <typename T>
Array<T> allocArray(std::initializer_list<T> list) {
    return allocArray(list.begin(), list.size());
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


template <typename T>
struct ArrayDisposer::Dispose_<T, true> {

    static void dispose(ArrayView<T>* view, ArrayDisposer const& disposer) {
        disposer.disposeImpl(const_cast<RemoveConst<T>*>(view->begin()),
                         sizeof(T), view->size(), nullptr);
    }
};

template <typename T>
struct ArrayDisposer::Dispose_<T, false> {

    static void destruct(void* ptr) {
        dtor(*reinterpret_cast<T*>(ptr));
    }

    static void dispose(ArrayView<T>* view, ArrayDisposer const& disposer) {
        disposer.disposeImpl(view->begin(), sizeof(T), view->size(), &destruct);
    }
};

template <typename T>
void ArrayDisposer::dispose(ArrayView<T>* view) const {
  Dispose_<T>::dispose(view, *this);
}


}  // End of namespace Solace
#endif  // SOLACE_ARRAY_HPP
