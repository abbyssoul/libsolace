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
 * libSolace: Channel for event loops
 *	@file		solace/io/async/future.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IO_ASYNC_FUTUE_HPP
#define SOLACE_IO_ASYNC_FUTUE_HPP

#include <solace/delegate.hpp>

#include <solace/result.hpp>
#include <solace/error.hpp>


namespace Solace { namespace IO { namespace async {

template<typename T>
class Core {
public:
    ~Core() = default;

    Core() :
        _completionHandler()
    {}

    Core(const Core&) = delete;
    Core& operator= (const Core&) = delete;

    Core(Core&&) = delete;
    Core& operator= (Core&&) = delete;

    template <class F>
    void setCallback(F&& func) {
        _completionHandler = std::forward<F>(func);
    }

    template <typename V>
    void setResult(V&& value) {
        if (_completionHandler) {
            // TODO(abbyssoul): Handle exceptions
            _completionHandler(value);
        }
    }

private:

    delegate<void(const T&)> _completionHandler;
};


template<>
class Core<void> {
public:
    ~Core() = default;

    Core() :
        _completionHandler()
    {}

    Core(const Core&) = delete;
    Core& operator= (const Core&) = delete;

    Core(Core&&) = delete;
    Core& operator= (Core&&) = delete;

    template <class F>
    void setCallback(F&& func) {
        _completionHandler = std::forward<F>(func);
    }

    void setResult() {
        if (_completionHandler) {
            // TODO(abbyssoul): Handle exceptions
            _completionHandler();
        }
    }

private:

    delegate<void()> _completionHandler;
};



template <typename T>
using CorePtr = std::shared_ptr<Core<T>>;


// Forward declation so Promise could reference it
template<typename T>
class Future;

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


/**
 * Promise is the 'push' side of a future
 */
template<typename T>
class Promise {
public:
    typedef T value_type;

public:
    ~Promise() = default;

    Promise(const Promise& ) = delete;
    Promise& operator= (const Promise& rhs) = delete;

    /**
     * Construct an empty promise
     */
    Promise() noexcept :
        _core(std::make_shared<Core<T>>())
    {}

    Promise(Promise<T>&& rhs) noexcept : Promise() {
        swap(rhs);
    }

    Promise& operator= (Promise<T>&& rhs) noexcept {
        return swap(rhs);
    }

    Promise& swap(Promise& rhs) noexcept {
        using std::swap;
        swap(_core, rhs._core);

        return *this;
    }

    /**
     * Get future associated with this Promise.
     * Note: this is mean to be called only once, thereafter
     * an exception will be raised.
     *
     * @return A Future assocciated with this promise
     */
    Future<T> getFuture();

    /**
     * Resolve this promise with a value.
     * (use perfect forwarding for both move and copy)
    */
    template <typename V>
    void setValue(V&& value) {
        if (_core) {
            _core->setResult(std::forward<V>(value));
        }
    }


    /**
     * Fulfill this Promise with the result of a function that takes no
     * arguments and returns something implicitly convertible to T.
     * Captures exceptions. e.g.
     * p.setWith([] { do something that may throw; return a T; });
    */
    template <typename F>
    void setWith(F&& func) {
        // FIXME: Handle excetions!
        setValue(func());
    }

    void setError(const Error& SOLACE_UNUSED(e)) {
        // FIXME: Implement
    }

protected:

    template <class> friend class Future;

private:

    CorePtr<T> _core;

};



template<>
class Promise<void> {
public:
    typedef void value_type;

public:
    ~Promise() = default;

    Promise(const Promise& ) = delete;
    Promise& operator= (const Promise& rhs) = delete;

    /**
     * Construct an empty promise
     */
    Promise() noexcept :
        _core(std::make_shared<Core<void>>())
    {}

    Promise(Promise&& rhs) noexcept : Promise() {
        swap(rhs);
    }

    Promise& operator= (Promise&& rhs) noexcept {
        return swap(rhs);
    }

    Promise& swap(Promise& rhs) noexcept {
        using std::swap;
        swap(_core, rhs._core);

        return *this;
    }

    /**
     * Get future associated with this Promise.
     * Note: this is mean to be called only once, thereafter
     * an exception will be raised.
     *
     * @return A Future assocciated with this promise
     */
    Future<value_type> getFuture();

    /**
     * Resolve this promise with a value.
     * (use perfect forwarding for both move and copy)
    */
    void setValue() {
        _core->setResult();
    }


    /**
     * Fulfill this Promise with the result of a function that takes no
     * arguments and returns something implicitly convertible to T.
     * Captures exceptions. e.g.
     * p.setWith([] { do something that may throw; return a T; });
    */
    template <typename F>
    void setWith(F&& func) {
        // FIXME: Handle excetions!
        func();
        setValue();
    }

    void setError(const Error& SOLACE_UNUSED(e)) {
        // FIXME: Implement
    }

protected:
    template <class> friend class Future;

private:

    CorePtr<void> _core;

};


/**
 * An async future.
 * This is an extention of Result class idea to represent asynchronous computation.
 *
 * @example
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

    typedef T value_type;

public:

    ~Future() = default;

    Future(const Future& ) = delete;
    Future& operator= (const Future& rhs) = delete;


    Future(Future&& rhs) noexcept : Future(CorePtr<T>()) {
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
     * @param completionHandler A completion handler to attach to this futuure.
     */
    template<typename F,
             typename R = typename std::result_of<F(value_type)>::type
             >
    std::enable_if_t<!std::is_void<R>::value && !isSomeResult<R>::value && !isFuture<R>::value, Future<R>>
    then(F&& f) {

        Promise<R> promise;
        auto chainedFuture = promise.getFuture();

        _core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (const T& v) mutable {
                pm.setValue(cont(v));
            });

        return chainedFuture;
    }


    /**
     * Attach completion handler/callback to this future to be called when the future is resolved.
     *
     * @param completionHandler A completion handler to attach to this futuure.
     */
    template<typename F,
             typename R = typename std::result_of<F(value_type)>::type
             >
    std::enable_if_t<isSomeResult<R>::value, Future<typename R::value_type> >
    then(F&& f) {
        using result_value_type = typename R::value_type;
        using result_error_type = typename R::error_type;

        Promise<result_value_type> promise;
        auto chainedFuture = promise.getFuture();

        _core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (const T& v) mutable {
                cont(v)
                    .then([&pm] (const result_value_type& rv){
                        pm.setValue(rv);
                    })
                    .orElse([&pm] (const result_error_type& er) {
                        pm.setError(er);
                    });
            });

        return chainedFuture;
    }


    /**
     * Attach completion handler/callback to this future to be called when the future is resolved.
     *
     * @param completionHandler A completion handler to attach to this futuure.
     */
    template<typename F,
             typename R = typename std::result_of<F(value_type)>::type
             >
    std::enable_if_t<isFuture<R>::value, Future<typename R::value_type> >
    then(F&& f) {
        using result_value_type = typename R::value_type;
//        using result_error_type = typename R::error_type;

        Promise<result_value_type> promise;
        auto chainedFuture = promise.getFuture();

        _core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (const T& v) mutable {
                cont(v)
                    .then([&pm] (const result_value_type& rv){
                        pm.setValue(rv);
                    });
//                    .orElse([&pm] (const result_error_type& er) {
//                        pm.setError(er);
//                    });
            });

        return chainedFuture;
    }

    /**
     * Specialization of continuation assigment method for functions returning void
     */
    template<typename F,
             typename R = typename std::result_of<F(value_type)>::type
             >
    std::enable_if_t<std::is_void<R>::value, Future<void> >
    then(F&& f) {
        Promise<void> promise;
        auto chainedFuture = promise.getFuture();

        _core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (const T& v) mutable {
            // TODO(abbyssoul): Handle exceptions!
            try {
                cont(v);
                pm.setValue();
            } catch (...) {
//                pm.setError(wrapExceptionIntoError);
            }
        });

        return chainedFuture;
    }

protected:
    template <class> friend class Promise;

    Future(const CorePtr<T>& core): _core{core}
    { }

private:

    CorePtr<T> _core;

};


/** Specialization of AsyncResult for void type
 *
 */
template <>
class Future<void> {
public:

    typedef void value_type;

public:

    ~Future() = default;

    Future(const Future& ) = delete;
    Future& operator= (const Future& rhs) = delete;

    Future(Future&& rhs) noexcept : Future(CorePtr<void>()) {
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
     * Attach completion handler to this future to be called when the future is resolved.
     * This is a special case of completion handlers return void this result in Future<void>
     * @param completionHandler A completion handler to attach to this futuure.
     */
    template<typename F,
             typename R = typename std::result_of<F(value_type)>::type
             >
    std::enable_if_t<std::is_void<R>::value, Future<void>>
    then(F&& f) {
        Promise<void> promise;
        auto chainedFuture = promise.getFuture();

        _core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] () mutable {
            // TODO(abbyssoul): Handle exceptions!
            try {
                cont();
                pm.setValue();
            } catch (...) {
//                pm.setError(wrapExceptionIntoError);
            }
        });

        return chainedFuture;
    }

    template<typename F,
             typename R = typename std::result_of<F(value_type)>::type
             >
    std::enable_if_t<!std::is_void<R>::value && !isSomeResult<R>::value, Future<R>>
    then(F&& f) {
        Promise<R> promise;
        auto chainedFuture = promise.getFuture();

        _core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] () mutable {
            // TODO(abbyssoul): Handle exceptions!
            try {
                pm.setValue(cont());
            } catch (...) {
//                pm.setError(wrapExceptionIntoError);
            }
        });

        return chainedFuture;
    }

    /**
     * Attach completion handler/callback to this future to be called when the future is resolved.
     *
     * @param completionHandler A completion handler to attach to this futuure.
     */
    template<typename F,
             typename R = typename std::result_of<F(value_type)>::type
             >
    std::enable_if_t<isSomeResult<R>::value, Future<typename R::value_type> >
    then(F&& f) {
        using result_value_type = typename R::value_type;
        using result_error_type = typename R::error_type;

        Promise<result_value_type> promise;
        auto chainedFuture = promise.getFuture();

        _core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] () mutable {
                cont()
                    .then([&pm] (const result_value_type& rv){
                        pm.setValue(rv);
                    })
                    .orElse([&pm] (const result_error_type& er) {
                        pm.setError(er);
                    });
            });

        return chainedFuture;
    }


protected:
    template <class> friend class Promise;

    Future(const CorePtr<void>& core): _core{core}
    { }

private:

    CorePtr<void> _core;
};


template <typename T>
Future<T> Promise<T>::getFuture() {
    return Future<T>(_core);
}


inline Future<void> Promise<void>::getFuture() {
    return Future<void>(_core);
}

}  // End of namespace async
}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_ASYNC_FUTUE_HPP
