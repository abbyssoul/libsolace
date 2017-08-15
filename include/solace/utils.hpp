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
 * libSolace: Various utilities for meta progamming.
 *	@file		solace/utils.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_UTILS_HPP
#define SOLACE_UTILS_HPP

#include <utility>  // FIXME(later): remove after removal of std::true_type/false_type


/**
 * Use DontInfer<T>::value in place of T for a template function parameter to prevent inference of
 * the type based on the parameter value.
 */
template <typename T> struct DontInfer { typedef T value; };
template <typename T> using DontInfer_t = typename DontInfer<T>::value;

/**
 * A version of std::declval<> that doesn't transform T into an rvalue reference.
 * If you need that, specify instance<T&&>()
 */
template <typename T>
T instance() noexcept;


template<typename F, typename...Args> struct isCallable {
  template<typename F2, typename...Args2> static constexpr std::true_type
  test(decltype(instance<F2>()(instance<Args2>()...)) *) { return {}; }

  template<typename F2, typename...Args2> static constexpr std::false_type
  test(...) { return {}; }

  static constexpr bool value = decltype(test<F, Args...>(nullptr))::value;
};


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


#endif  // SOLACE_UTILS_HPP
