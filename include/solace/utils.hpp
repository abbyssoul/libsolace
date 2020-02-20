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
 * libSolace: Meta progamming utilities.
 *	@file		solace/utils.hpp
 ******************************************************************************/
#pragma once
#ifndef SOLACE_UTILS_HPP
#define SOLACE_UTILS_HPP

#include "solace/types.hpp"  // size_t

#include <type_traits>


namespace Solace {

namespace _ {  // private
// Custom placement new tag
struct PlacementNew {};
}  // namespace _


/**
 * Use DontInfer<T>::value in place of T for a template function parameter to prevent inference of
 * the type based on the parameter value.
 */
template <typename T> struct DontInfer_ { using value_type = T; };
template <typename T> using DontInfer_v = typename DontInfer_<T>::value_type;

/**
 * A version of std::declval<> that doesn't transform T into an rvalue reference.
 * If you need that, specify instance<T&&>()
 */
template <typename T>
T instance() noexcept;

template <typename T> struct RemoveConst_ { using Type = T; };
template <typename T> struct RemoveConst_<const T> { using Type = T; };
template <typename T> using RemoveConst = typename RemoveConst_<T>::Type;

template <typename T> struct Decay_ { using Type = T; };
template <typename T> struct Decay_<T&> { typedef typename Decay_<T>::Type Type; };
template <typename T> struct Decay_<T&&> { typedef typename Decay_<T>::Type Type; };
template <typename T> struct Decay_<T[]> { typedef typename Decay_<T*>::Type Type; };
template <typename T> struct Decay_<const T[]> { typedef typename Decay_<const T*>::Type Type; };
template <typename T, size_t s> struct Decay_<T[s]> { typedef typename Decay_<T*>::Type Type; };
template <typename T, size_t s> struct Decay_<const T[s]> { typedef typename Decay_<const T*>::Type Type; };
template <typename T> struct Decay_<const T> { typedef typename Decay_<T>::Type Type; };
template <typename T> struct Decay_<volatile T> { typedef typename Decay_<T>::Type Type; };
template <typename T> using Decay = typename Decay_<T>::Type;

/**
 * Replacement for std::move() and std::forward() in order to not pull
 * std header <utility> that drags a lot of other stuff we don't use.
 */
template<class T>
constexpr typename Decay_<T>::Type&&
mv(T&& t) noexcept {
	return static_cast<typename Decay_<T>::Type&&>(t);
}


template<typename T>
constexpr T&&
fwd(DontInfer_v<T>& t) noexcept {
    return static_cast<T&&>(t);
}


template<class T, class U = T>
constexpr T exchange(T& obj, U&& newValue) {
    T old_value{mv(obj)};
	obj = fwd<U>(newValue);

    return old_value;
}


/// An expressive way to do narrowing casts
template <typename T, typename U>
constexpr T narrow_cast(U&& u) noexcept {
	return static_cast<T>(fwd<U>(u));
}

/**
 * Returns true if T can be (possibly) copied using memcpy instead of using the copy constructor or
 * assignment operator.
 */
template <typename T>
constexpr bool canMemcpy() {
    return std::is_trivially_copy_constructible<T>::value && std::is_trivially_copy_assignable<T>::value;
}

template <typename T> struct RefOrRValue_		{ using Type = T&&; };
template <typename T> struct RefOrRValue_<T&>	{ using Type = T&; };
template <typename T> struct RefOrRValue_<T&&>	{ using Type = T&&; };
template <typename T> using RefOrRValue = typename RefOrRValue_<T>::Type;

}  // End of namespace Solace


inline void* operator new(size_t, Solace::_::PlacementNew, void* __p) noexcept { return __p; }
inline void operator delete(void*, Solace::_::PlacementNew, void*) noexcept {}


namespace Solace {

template <typename T, typename... Params>
inline T* ctor(T& location, Params&&... params) {
	return new (_::PlacementNew(), &location) T{fwd<Params>(params)...};
}

template <typename T>
inline void dtor(T& location) noexcept(std::is_nothrow_destructible_v<T>) {
	location.~T();
}

}  // End of namespace Solace



#endif  // SOLACE_UTILS_HPP
