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
 * libSolace: Implementation details of Future composition methods
 *	@file		solace/details/futureComposition.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_DEATILS_FUTURE_COMPOSITION_HPP
#define SOLACE_DEATILS_FUTURE_COMPOSITION_HPP

#include "solace/future.hpp"
#include "solace/asyncErrorDomain.hpp"


#include <atomic>
#include <vector>
#include <algorithm>
#include <memory>  // make_shared<>


namespace Solace { namespace details {

template <typename T>
struct CollectContext {

    struct Nothing {
        explicit Nothing(int SOLACE_UNUSED(n))
        {}
    };

    using Result = typename std::conditional<std::is_void<T>::value, void, std::vector<T>>::type;

    using InternalResult = typename std::conditional<std::is_void<T>::value, Nothing, std::vector<Optional<T>>>::type;

    explicit CollectContext(size_t n) : result(n)
    {}

    ~CollectContext() {
        if (!threw.exchange(true)) {
            // map Optional<T> -> T
            std::vector<T> finalResult;
            finalResult.reserve(result.size());

            std::transform(result.begin(), result.end(), std::back_inserter(finalResult),
                           [](Optional<T>& o) { return o.move(); });

            p.setValue(std::move(finalResult));
        } else {
            p.setError(makeError(AsyncError::AsyncError, "CollectContext"));
        }
    }

    inline void setPartialResult(size_t i, T&& t) {
        result[i] = std::move(t);
    }

    void setError(size_t , Error&& ) {
        threw.exchange(true);
    }

    Future<Result> getFuture() {
        return p.getFuture();
    }

private:

    Promise<Result> p;
    InternalResult result;
    std::atomic<bool> threw {false};
};


template <>
struct CollectContext<void> {

    CollectContext() = default;

    ~CollectContext() {
        if (!threw.exchange(true)) {
            p.setValue();
        } else {
            p.setError(makeError(AsyncError::AsyncError, "CollectContext"));
        }
    }

    void setPartialResult(size_t ) {
    }

    void setError(size_t , Error&& ) {
        threw.exchange(true);
    }

    Future<void> getFuture() {
        return p.getFuture();
    }

private:

    Promise<void> p;
    std::atomic<bool> threw {false};
};

}  // namespace details


template <class InputIterator>
Future<typename details::CollectContext<typename std::iterator_traits<InputIterator>::value_type::value_type>::Result>
collect(InputIterator first, InputIterator last) {
  typedef typename std::iterator_traits<InputIterator>::value_type::value_type T;

    auto ctx = std::make_shared<details::CollectContext<T>>(std::distance(first, last));
    for (size_t i = 0; first != last; ++first, ++i) {
        first->then([i, ctx](T&& val) {
            ctx->setPartialResult(i, std::move(val));
        })
        .onError([i, ctx](Error&& e) {
            ctx->setError(i, std::move(e));
        });
    }

    return ctx->getFuture();
}

inline
Future<void> collect(std::vector<Future<void>>& futures) {
    auto ctx = std::make_shared<details::CollectContext<void>>();

    for (typename std::vector<Future<void>>::size_type i = 0; i < futures.size(); ++i) {
        futures[i].then([ctx, i]() {
            ctx->setPartialResult(i);
        })
        .onError([ctx, i](Error&& e) {
            ctx->setError(i, std::move(e));
        });
    }

    return ctx->getFuture();
}


/// Sugar for the most common case
template <class Collection>
auto collect(Collection&& c) -> decltype(collect(c.begin(), c.end())) {
  return collect(c.begin(), c.end());
}

}  // End of namespace Solace
#endif  // SOLACE_DEATILS_FUTUREC_OMPOSITION_HPP
