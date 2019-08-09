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
 * libSolace: A trait to test if a type is callable
 *	@file		solace/traits/callable.hpp
 ******************************************************************************/
#pragma once
#ifndef SOLACE_TRAITS_CALLABLE_HPP
#define SOLACE_TRAITS_CALLABLE_HPP

#include "solace/utils.hpp"


namespace Solace {

/**
 * Template meta-programming trait to test if an objec is callable.
 */
template<typename F, typename...Args>
struct isCallable {

    template<typename F2, typename...Args2>
    static constexpr std::true_type
    test(decltype(instance<F2>()(instance<Args2>()...)) *) { return {}; }

    template<typename F2, typename...Args2>
    static constexpr std::false_type
    test(...) { return {}; }

    static constexpr bool value = decltype(test<F, Args...>(nullptr))::value;
};


template<typename F, typename...Args>
using isCallable_v = typename isCallable<F, Args...>::value;

}  // End of namespace Solace
#endif  // SOLACE_TRAITS_CALLABLE_HPP
