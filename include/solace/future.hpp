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
 *	@file		solace/future.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_FUTUE_HPP
#define SOLACE_FUTUE_HPP

#include "solace/promise.hpp"


namespace Solace {


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
Future<UnpuckedResult> thenImplementation(std::shared_ptr<Core<T>>& core, F&& f) {
    if (!core || core->isDetached()) {
        raiseInvalidStateError("Invalid Future without a Promise");
    }

    Promise<UnpuckedResult> promise;
    auto chainedFuture = promise.getFuture();

    core->setCallback(std::make_shared<CB<T, ContinuationResult, UnpuckedResult, F>>(std::forward<F>(f),
                                                                                     std::move(promise)));

    return chainedFuture;
}


template<typename T,
         typename ContinuationResult,
         typename UnpuckedResult, typename UnpackedErrorResultType,
         typename F>
Future<UnpuckedResult> onErrorImplementation(std::shared_ptr<Core<T>>& core, F&& f) {
    if (!core || core->isDetached()) {
        raiseInvalidStateError("Invalid Future without a Promise");
    }

    Promise<UnpuckedResult> promise;
    auto chainedFuture = promise.getFuture();

    core->setCallback(std::make_shared<ErrBack<T, ContinuationResult, UnpuckedResult, F>>(std::forward<F>(f),
                                                                                          std::move(promise)));

    return chainedFuture;
}


}  // namespace details


/**
 * An future.
 * This is an extention of a Result (@see Result) class to represent asynchronous computation.
 * Note that creating future does not spawn a computation. It is a promise that the computation will be
 * carried out some time in the future.
 * Future class is the 'client' side of the computation. Owner responsible for computing the value holds a Promise
 * object and use it to notify interested clients about value being availiable.
 *
 * This particular implementation of the Future paradigm does not store computed value it self. Rather it provides an
 * interface for the consumer to register a funtion that will be colled with the value when it become availiable.
 * Clients also can chose to register another callback to be notified if there was an error in the computation and the
 * value will not be availialbe.
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

    /**
     * Default constructable Future.
     * Not very useful by itself but can accept values via move-assignement.
     */
    Future() :
        _core(std::make_shared<Core<T>>())
    {}


    Future(Future&& rhs) noexcept :
        _core(std::move(rhs._core))
    {}

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

        return details::thenImplementation<T, R, UnpackedRT, UnpackedET>(_core, std::forward<F>(f));
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

        return details::thenImplementation<T, R, R, error_type>(_core, std::forward<F>(f));
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
        return details::onErrorImplementation<T, R, R, error_type>(_core, std::forward<F>(f));
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

        return details::onErrorImplementation<T, R, UnpackedRT, UnpackedET>(_core, std::forward<F>(f));
    }

protected:
    template <typename> friend class Promise;
    template <typename FT> friend Future<FT> makeFuture(FT&&);

    Future(const std::shared_ptr<Core<T>>& core): _core{core}
    {}

private:

    std::shared_ptr<Core<T>> _core;

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

    /**
     * Default constructable Future.
     * Not very useful by itself but can accept values via move-assignement.
     */
    Future() :
        _core(std::make_shared<Core<void>>())
    {}

    Future(Future&& rhs) noexcept :
        _core(std::move(rhs._core))
    {
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

        return details::thenImplementation<void, R, UnpackedRT, UnpackedET>(_core, std::forward<F>(f));
    }


    template<typename F,
             typename R = typename std::result_of<F(void)>::type
             >
    std::enable_if_t<!isSomeResult<R>::value && !isFuture<R>::value, Future<R>>
    then(F&& f) {

        return details::thenImplementation<void, R, R, Error>(_core, std::forward<F>(f));
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
        return details::onErrorImplementation<void, R, R, error_type>(_core, std::forward<F>(f));
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

        return details::onErrorImplementation<void, R, UnpackedRT, UnpackedET>(_core, std::forward<F>(f));
    }


protected:
    template <typename> friend class Promise;

    Future(const std::shared_ptr<Core<void>>& core): _core{core}
    { }

private:

    std::shared_ptr<Core<void>> _core;

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


template<typename T>
Future<T> makeFuture(T&& value) {
    Promise<T> p;
    auto future = p.getFuture();
    p.setValue(std::move(value));

    return future;
}


}  // End of namespace Solace

#include "details/future_impl.hpp"
#include "details/futureComposition.hpp"

#endif  // SOLACE_FUTUE_HPP
