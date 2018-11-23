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
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_UTILS_HPP
#define SOLACE_UTILS_HPP

#include "solace/types.hpp"  // size_t

#include <utility>  // FIXME(later): remove after removal of std::true_type/false_type

namespace Solace {

namespace _ {  // private
// Custom placement new tag
struct PlacementNew {};
}  // namespace _

/**
 * Use DontInfer<T>::value in place of T for a template function parameter to prevent inference of
 * the type based on the parameter value.
 */
template <typename T> struct DontInfer { using value = T; };
template <typename T> using DontInfer_t = typename DontInfer<T>::value;

/**
 * A version of std::declval<> that doesn't transform T into an rvalue reference.
 * If you need that, specify instance<T&&>()
 */
template <typename T>
T instance() noexcept;


template<class X, class Y, class Op>
struct op_valid_impl {
    template<class U, class L, class R>
    static auto test(int) -> decltype(std::declval<U>()(std::declval<L>(), std::declval<R>()),
                                      void(), std::true_type());

    template<class U, class L, class R>
    static auto test(...) -> std::false_type;

    using type = decltype(test<Op, X, Y>(0));

};

template <typename T> struct RemoveConst_ { typedef T Type; };
template <typename T> struct RemoveConst_<const T> { typedef T Type; };
template <typename T> using RemoveConst = typename RemoveConst_<T>::Type;

template <typename T> struct Decay_ { typedef T Type; };
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
template<typename T> constexpr
T&& mv(T& t) noexcept {
    return static_cast<T&&>(t);
}

template<typename T> constexpr
T&& fwd(DontInfer<T>& t) noexcept {
    return static_cast<T&&>(t);
}


template<class T, class U = T>
constexpr T exchange(T& obj, U&& newValue) {
    T old_value{mv(obj)};
    obj = std::forward<U>(newValue);

    return old_value;
}


/// An expressive way to do narrowing casts
template <typename T, typename U>
constexpr T narrow_cast(U&& u) noexcept {
    return static_cast<T>(std::forward<U>(u));
}

/**
 * Returns true if T can be (possibly) copied using memcpy instead of using the copy constructor or
 * assignment operator.
 */
template <typename T>
constexpr bool canMemcpy() {
    return std::is_trivially_copy_constructible<T>::value && std::is_trivially_copy_assignable<T>::value;
}


/* Unused meta code to check if class support certain operations
 *
template<class X, class Y, class Op> using op_valid = typename op_valid_impl<X, Y, Op>::type;
template<class X, class Y> using has_equality = op_valid<X, Y, std::equal_to<>>;
template<class X, class Y> using has_inequality = op_valid<X, Y, std::not_equal_to<>>;
template<class X, class Y> using has_less_than = op_valid<X, Y, std::less<>>;
template<class X, class Y> using has_less_equal = op_valid<X, Y, std::less_equal<>>;
template<class X, class Y> using has_greater_than = op_valid<X, Y, std::greater<>>;
template<class X, class Y> using has_greater_equal = op_valid<X, Y, std::greater_equal<>>;
*/

}  // End of namespace Solace


inline void* operator new(size_t, Solace::_::PlacementNew, void* __p) noexcept { return __p; }
inline void operator delete(void*, Solace::_::PlacementNew, void*) noexcept {}


#endif  // SOLACE_UTILS_HPP
