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
 *	@file		solace/details/array_utils.hpp
 *  @brief		Implemenetation details for collections.
 * Note: Not to be included directly.
 ******************************************************************************/
#pragma once
#ifndef SOLACE_DETAILS_ARRAY_UTILS_HPP
#define SOLACE_DETAILS_ARRAY_UTILS_HPP

#include "solace/utils.hpp"
#include "solace/arrayView.hpp"


namespace Solace {

template<typename T>
struct ExceptionGuard {
    T const* start;
    T* pos;

    inline ~ExceptionGuard() noexcept(false) {
        while (pos > start) {
            dtor(*--pos);
        }
    }

    constexpr explicit ExceptionGuard(T* p) noexcept : start{p}, pos{p} {}
    constexpr void release() noexcept { start = pos; }
};


template <typename T>
void initArray(MutableMemoryView bufferView, typename ArrayView<T>::size_type arraySize) {
    if (std::is_nothrow_default_constructible<T>::value) {
        auto pos = bufferView.template dataAs<T>();
        for (size_t i = 0; i < arraySize; ++i) {
            ctor(*pos++);
        }
    } else {
        ExceptionGuard<T> guard(bufferView.template dataAs<T>());
        for (size_t i = 0; i < arraySize; ++i) {
            ctor(*guard.pos);
            ++guard.pos;  // Important to be on a different line, in case of exception.
        }
        guard.release();
    }
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
    static void apply(ArrayView<T> dest, ArrayView<const T> src) {
        auto start = src.begin();
        auto const end = src.end();

        if constexpr (std::is_nothrow_copy_constructible<T>::value) {
            auto pos = dest.begin();
            while (start != end) {
                ctor(*pos++, *start++);
            }
        } else {
            ExceptionGuard<T> guard(dest.begin());
            while (start != end) {
                ctor(*guard.pos, *start++);
                ++guard.pos;
            }
            guard.release();
        }
    }
};

template <typename T, typename Iterator>
struct CopyConstructArray_<T, Iterator, true, false> {
    static void apply(ArrayView<T> dest, ArrayView<T> src) {
        auto start = src.begin();
        auto const end = src.end();

        if constexpr(std::is_nothrow_move_constructible<T>::value) {
            auto pos = dest.begin();
            while (start != end) {
                ctor(*pos++, std::move(*start++));
            }
        } else {
            ExceptionGuard<T> guard(dest.begin());
            while (start != end) {
                ctor(*guard.pos, std::move(*start++));
                ++guard.pos;
            }
            guard.release();
        }
    }
};


static_assert(canMemcpy<char>(), "canMemcpy<>() is broken");


}  // End of namespace Solace
#endif  // SOLACE_DETAILS_ARRAY_UTILS_HPP
