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
 * libSolace: Future class to bind completion handlers for async operatoins
 *	@file		solace/io/async/future.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IO_ASYNC_FUTUE_HPP
#define SOLACE_IO_ASYNC_FUTUE_HPP

#include "solace/io/async/promise.hpp"


namespace Solace { namespace IO { namespace async {


template <typename T>
struct isFuture : std::false_type {
    using std::false_type::value;

    typedef T value_type;
};

template <typename T>
struct isFuture<Future<T>> : std::true_type {
    using std::true_type::value;

    typedef T value_type;
};


namespace details  {

template<typename FutureValueType,
         typename ContinuationResult,
         typename UnpuckedResultType,
         typename F>
struct CB;


template<typename FutureValueType,
         typename ContinuationResult,
         typename UnpuckedResultType,
         typename F>
struct ErrBack;


template<typename T,
         typename ContinuationResult,
         typename UnpuckedResult, typename UnpackedErrorResultType,
         typename F>
Future<UnpuckedResult> thenImplementation(std::shared_ptr<Core<T>>&& core, F&& f) {
    Promise<UnpuckedResult> promise;

    if (!core) {
        raiseInvalidStateError("Invalid Future without a Promise");
    }

    auto chainedFuture = promise.getFuture();

    core->setCallback(std::make_shared<CB<T, ContinuationResult, UnpuckedResult, F>>(std::forward<F>(f), std::move(promise)));

    return chainedFuture;
}


template<typename T,
         typename ContinuationResult,
         typename UnpuckedResult, typename UnpackedErrorResultType,
         typename F>
Future<UnpuckedResult> onErrorImplementation(std::shared_ptr<Core<T>>&& core, F&& f) {
    Promise<UnpuckedResult> promise;

    if (!core) {
        raiseInvalidStateError("Invalid Future without a Promise");
    }

    auto chainedFuture = promise.getFuture();

    core->setCallback(std::make_shared<ErrBack<T, ContinuationResult, UnpuckedResult, F>>(std::forward<F>(f), std::move(promise)));

    return chainedFuture;
}


}  // namespace details


/**
 * An async future.
 * This is an extention of Result class idea to represent asynchronous computation.
 *
 * \code{.cpp}
 * io_object.do_something_async().then([](auto value) {
 *      ...
 *      use the value when it become available
 *      ...
 *
 * });
 * \endcode
 */
template <typename T>
class Future {
public:

    typedef T       value_type;
    typedef Error   error_type;

public:

    ~Future() = default;

    Future(const Future& ) = delete;
    Future& operator= (const Future& rhs) = delete;


    Future(Future&& rhs) noexcept : Future(std::shared_ptr<Core<T>>()) {
        swap(rhs);
    }

    Future& operator= (Future&& rhs) noexcept {
        return swap(rhs);
    }

    Future& swap(Future& rhs) noexcept {
        using std::swap;
        swap(_core, rhs._core);

        return *this;
    }


    /**
     * Attach completion handler/callback to this future to be called when the future is resolved.
     *
     * @param completionHandler A completion handler to attach to this future.
     */
    template<typename F,
             typename R = typename std::result_of<F(T)>::type
             >
    std::enable_if_t<isFuture<R>::value || isSomeResult<R>::value, Future<typename R::value_type> >
    then(F&& f) {
        using UnpackedRT = typename R::value_type;
        using UnpackedET = typename R::error_type;

        return details::thenImplementation<T, R, UnpackedRT, UnpackedET>(_core.lock(), std::forward<F>(f));
    }


    /**
     * Attach completion handler/callback to this future to be called when the future is resolved.
     *
     * @param completionHandler A completion handler to attach to this future.
     */
    template<typename F,
             typename R = typename std::result_of<F(T)>::type
             >
    std::enable_if_t<!isSomeResult<R>::value && !isFuture<R>::value, Future<R>>
    then(F&& f) {

        return details::thenImplementation<T, R, R, error_type>(_core.lock(), std::forward<F>(f));
    }


    //------------------------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------

    /**
     * Attach error handler/callback to this future to be called when the future has failed.
     * @param f A completion handler to attach to this future.
     */
    template<typename F,
             typename R = typename std::result_of<F(error_type)>::type
             >
    std::enable_if_t<(!isFuture<R>::value && !isSomeResult<R>::value) && std::is_convertible<R, T>::value,
    Future<R>>
    onError(F&& f) {
        return details::onErrorImplementation<T, R, R, error_type>(_core.lock(), std::forward<F>(f));
    }


    /**
     * Attach error handler/callback to this future to be called when the future has failed.
     * @param f A completion handler to attach to this future.
     */
    template<typename F,
             typename R = typename std::result_of<F(error_type)>::type
             >
    std::enable_if_t<(isFuture<R>::value || isSomeResult<R>::value) &&
            std::is_convertible<typename R::value_type, T>::value,
    Future<typename R::value_type>>
    onError(F&& f) {
        using UnpackedRT = typename R::value_type;
        using UnpackedET = typename R::error_type;

        return details::onErrorImplementation<T, R, UnpackedRT, UnpackedET>(_core.lock(), std::forward<F>(f));
    }

protected:
    template <class> friend class Promise;

    Future(const std::shared_ptr<Core<T>>& core): _core{core}
    { }

private:

    std::weak_ptr<Core<T>> _core;

};


/**
 * Specialization of Future for void type
 * (this exists to prevent formation of references to void.
 */
template <>
class Future<void> {
public:

    typedef void    value_type;
    typedef Error   error_type;

public:

    ~Future() = default;

    Future(const Future& ) = delete;
    Future& operator= (const Future& rhs) = delete;

    Future(Future&& rhs) noexcept : Future(std::shared_ptr<Core<void>>()) {
        swap(rhs);
    }

    Future& operator= (Future&& rhs) noexcept {
        return swap(rhs);
    }

    Future& swap(Future& rhs) noexcept {
        using std::swap;
        swap(_core, rhs._core);

        return *this;
    }


    /**
     * Attach completion handler/callback to this future to be called when the future is resolved.
     *
     * @param completionHandler A completion handler to attach to this future.
     */
    template<typename F,
             typename R = typename std::result_of<F(void)>::type
             >
    std::enable_if_t<isFuture<R>::value || isSomeResult<R>::value, Future<typename R::value_type> >
    then(F&& f) {
        using UnpackedRT = typename R::value_type;
        using UnpackedET = typename R::error_type;

        return details::thenImplementation<void, R, UnpackedRT, UnpackedET>(_core.lock(), std::forward<F>(f));
    }


    template<typename F,
             typename R = typename std::result_of<F(void)>::type
             >
    std::enable_if_t<!isSomeResult<R>::value && !isFuture<R>::value, Future<R>>
    then(F&& f) {

        return details::thenImplementation<void, R, R, Error>(_core.lock(), std::forward<F>(f));
    }

    //------------------------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------

    /**
     * Attach error handler/callback to this future to be called when the future has failed.
     * @param f A completion handler to attach to this future.
     */
    template<typename F,
             typename R = typename std::result_of<F(error_type)>::type
             >
    std::enable_if_t<(!isFuture<R>::value && !isSomeResult<R>::value) && std::is_void<R>::value,
    Future<R>>
    onError(F&& f) {
        return details::onErrorImplementation<void, R, R, error_type>(_core.lock(), std::forward<F>(f));
    }


    /**
     * Attach error handler/callback to this future to be called when the future has failed.
     * @param f A completion handler to attach to this future.
     */
    template<typename F,
             typename R = typename std::result_of<F(error_type)>::type
             >
    std::enable_if_t<(isFuture<R>::value || isSomeResult<R>::value) && std::is_void<typename R::value_type>::value,
    Future<typename R::value_type>>
    onError(F&& f) {
        using UnpackedRT = typename R::value_type;
        using UnpackedET = typename R::error_type;

        return details::onErrorImplementation<void, R, UnpackedRT, UnpackedET>(_core.lock(), std::forward<F>(f));
    }


protected:
    template <class> friend class Promise;

    Future(const std::shared_ptr<Core<void>>& core): _core{core}
    { }

private:

    std::weak_ptr<Core<void>> _core;

};


template <typename T>
Future<T>
Promise<T>::getFuture() {
    return Future<T>(_core);
}


inline Future<void>
Promise<void>::getFuture() {
    return Future<void>(_core);
}



}  // End of namespace async
}  // End of namespace IO
}  // End of namespace Solace

#include "future_impl.hpp"
#include "futureComposition.hpp"

#endif  // SOLACE_IO_ASYNC_FUTUE_HPP
