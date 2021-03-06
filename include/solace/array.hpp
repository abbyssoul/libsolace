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
 *  @file		solace/array.hpp
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
		: _buffer{mv(rhs._buffer)}
		, _size{exchange(rhs._size, 0)}
    {
    }

    /** Contruct an array of a given size from a memory resource */
    constexpr Array(MemoryResource&& buffer, size_type len) noexcept
		: _buffer{mv(buffer)}
		, _size{len}
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
		view().set(index, fwd<F>(f));

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

    /**
     * @brief Get view of this array.
     * @return View of this array.
     *
     * @note The view does not take ownership of the storage. It is user's responsibility
     * to make sure that the view return is not used after the array life-time.
     */
    ArrayView<T const> view() const noexcept {
		return arrayView<T const>(_buffer.view().slice(0, _size*sizeof(T)));
    }

    /**
     * @brief Get mutalbe view of this array.
     * @return Mutalbe view of this array.
     *
     * @note The view does not take ownership of the storage. It is user's responsibility
     * to make sure that the view return is not used after the array life-time.
     */
    ArrayView<T> view() noexcept {
		return arrayView<T>(_buffer.view().slice(0, _size*sizeof(T)));
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
				isCallable<F, T const&>::value ||
				isCallable<F, T&>::value ||
				isCallable<F, T&&>::value,
    Array<T>& >
    forEach(F&& f) {
		view().forEach(fwd<F>(f));

        return *this;
    }

    template<typename F>
	std::enable_if_t<
		isCallable<F, T>::value ||
		isCallable<F, T const&>::value,
	Array<T> const& >
    forEach(F&& f) const {
		view().forEach(fwd<F>(f));

        return *this;
    }

    template<typename F>
    std::enable_if_t<
            isCallable<F, size_type, const T&>::value,
    const Array<T>& >
    forEach(F&& f) const {
		view().forEach(fwd<F>(f));

        return *this;
    }


    template<typename F>
    std::enable_if_t<
            isCallable<F, size_type, T>::value ||
            isCallable<F, size_type, T&>::value,
    Array<T>& >
    forEach(F&& f) {
		view().forEach(fwd<F>(f));

        return *this;
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

private:
    MemoryResource      _buffer;
    size_type           _size{0};
};


// Static code check
static_assert(!canMemcpy<Array<char>>(), "canMemcpy<>() is broken");


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

template<typename T>
void swap(Array<T>& lhs, Array<T>& rhs) noexcept {
    lhs.swap(rhs);
}

/** Construct an empty array of T of a size 0 */
template <typename T>
[[nodiscard]]
constexpr Array<T> makeArray() noexcept {
    return {};
}

template <typename T>
[[nodiscard]] constexpr
Result<Array<T>, Error>
makeArray(MemoryResource&& mem, typename Array<T>::size_type initialSize) noexcept {
	return {types::okTag, in_place, mv(mem), initialSize};
}

/**
 * Construct a default-initialized array of T of a given fixed size using custom memory manager.
 * @param memManager Memory manager to use to allocate space for the array.
 * @param initialSize Size of the array to construct.
 * @return Resulting array or an  error.
*/
template <typename T>
[[nodiscard]]
Result<Array<T>, Error>
makeArray(MemoryManager& memManager, typename Array<T>::size_type initialSize) {
	auto maybeBuffer = memManager.allocate(initialSize*sizeof(T));
	if (!maybeBuffer) {
		return maybeBuffer.moveError();
	}

	// Emplce new default constructed values T into a new memory space.
	// This may throw if construction of T{} throws
	initArray(arrayView<T>(maybeBuffer.unwrap().view()));

	return makeArray<T>(maybeBuffer.moveResult(), initialSize);  // No except c-tor
}

/**
 * Construct a default-initialized array of T of a given fixed size using system memory manager.
 * @param initialSize Size of the array to construct.
 * @return Resulting array or an  error.
*/
template <typename T>
[[nodiscard]]
auto makeArray(typename Array<T>::size_type initialSize) {
	return makeArray<T>(getSystemHeapMemoryManager(), initialSize);
}


/**
 * Construct a new array from a C-style array using custom memory manager.
 * @param memManager Memory manager to use to allocate space for the array.
 * @param initialSize Size of the array to construct.
 * @param carray C-style array to _copy_ values from. Should be no less then initialSize elements.
 * @return Resulting array or an  error.
*/
template <typename T>
[[nodiscard]]
Result<Array<T>, Error>
makeArray(MemoryManager& memManager, typename Array<T>::size_type initialSize, T const* carray) {
    auto const arraySize = initialSize;
	auto maybeBuffer = memManager.allocate(arraySize * sizeof(T));
	if (!maybeBuffer) {
		return maybeBuffer.moveError();
	}

	auto src = arrayView(carray, arraySize);					// No except
	auto dest = arrayView<T>(maybeBuffer.unwrap().view());		// No except

    CopyConstructArray_<RemoveConst<T>, Decay<T*>, false>::apply(dest, src);    // May throw if copy-ctor throws

	return makeArray<T>(maybeBuffer.moveResult(), arraySize);  // No except
}

/**
 * Construct a new array from a C-style array using system memory manager.
 * @param initialSize Size of the array to construct.
 * @param carray C-style array to _copy_ values from. Should be no less then initialSize elements.
 * @return Resulting array or an  error.
*/
template <typename T>
[[nodiscard]]
auto makeArray(typename Array<T>::size_type initialSize, T const* carray) {
	return makeArray<T>(getSystemHeapMemoryManager(), initialSize, carray);  // No except
}



/** Create a copy of the given array
 * @param memManager Memory manager to use to allocate space for the array.
 * @param other ArrayView to _copy_ values from.
 * @return Resulting array or an  error.
*/
template <typename T>
[[nodiscard]]
auto makeArray(MemoryManager& memManager, ArrayView<T> other) {
	return makeArray(memManager, other.size(), other.data());
}


/** Create a copy of the given array
 * @param other ArrayView to _copy_ values from.
 * @return Resulting array or an  error.
*/
template <typename T>
[[nodiscard]]
auto makeArray(ArrayView<T> other) {
	return makeArray(getSystemHeapMemoryManager(), other);
}


/** Create a copy of the given array using custom memory manager.
 * @param memManager Memory manager to use to allocate space for the array.
 * @param other ArrayView to _copy_ values from.
 * @return Resulting array or an  error.
*/
template <typename T>
[[nodiscard]]
auto makeArray(MemoryManager& memManager, Array<T> const& other) {
	return makeArray(memManager, other.size(), other.data());
}


/** Create a copy of the given array
 * @param other ArrayView to _copy_ values from.
 * @return Resulting array or an  error.
*/
template <typename T>
[[nodiscard]]
auto makeArray(Array<T> const& other) {
	return makeArray(getSystemHeapMemoryManager(), other);
}


template <typename T,
          typename...Args>
[[nodiscard]]
Result<Array<T>, Error>
makeArrayOf(Args&&...args) {
     // Should be relativily safe to cast: we don't expect > 65k arguments
    using size_type = typename Array<T>::size_type;
    auto const arraySize = narrow_cast<size_type>(sizeof...(args));
	auto maybeBuffer = getSystemHeapMemoryManager().allocate(arraySize * sizeof(T));
	if (!maybeBuffer) {
		return maybeBuffer.moveError();
	}

	auto values = arrayView<T>(maybeBuffer.unwrap().view());
	values.emplaceAll(fwd<Args>(args)...);

	return makeArray<T>(maybeBuffer.moveResult(), arraySize);  // No except
}


}  // End of namespace Solace
#endif  // SOLACE_ARRAY_HPP
