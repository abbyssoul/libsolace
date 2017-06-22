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

    void setResult(Result<T, Error>&& result) {
        if (_completionHandler) {
            // TODO(abbyssoul): Handle exceptions. What happenes when completion handler throws?
            _completionHandler(std::move(result));
        }
    }

private:

    // FIXME: Use Try<> instead of result to capture exceptions
    delegate<void(Result<T, Error>&&)> _completionHandler;
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
//            _core->setResult(Ok<V>(std::move(value)));
            _core->setResult(Ok<V>(std::forward<V>(value)));
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

    void setError(Error&& e) {
        if (_core) {
            _core->setResult(Err(std::move(e)));
        }
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
        _core->setResult(Ok());
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

    void setError(Error&& e) {
        if (_core) {
            _core->setResult(Err(std::move(e)));
        }
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
     * @param completionHandler A completion handler to attach to this future.
     */
    template<typename F,
             typename R = typename std::result_of<F(T)>::type
             >
    std::enable_if_t<!std::is_void<R>::value && !isSomeResult<R>::value && !isFuture<R>::value, Future<R>>
    then(F&& f) {

        Promise<R> promise;
        auto chainedFuture = promise.getFuture();

        _core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<T, Error>&& result) mutable {
            if (result.isError()) {
                pm.setError(result.moveError());
            } else {
                // FIXME: Handle exceptions in completion handler
                pm.setValue(cont(result.unwrap()));
            }
        });

        return chainedFuture;
    }


    /**
     * Attach completion handler/callback to this future to be called when the future is resolved.
     *
     * @param completionHandler A completion handler to attach to this future.
     */
    template<typename F,
             typename R = typename std::result_of<F(T)>::type
             >
    std::enable_if_t<isSomeResult<R>::value, Future<typename R::value_type> >
    then(F&& f) {
        using result_value_type = typename R::value_type;
        using result_error_type = typename R::error_type;

        Promise<result_value_type> promise;
        auto chainedFuture = promise.getFuture();

        _core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<T, Error>&& result) mutable {
            if (result.isError()) {
                pm.setError(result.moveError());
            } else {
                cont(result.unwrap())
                    .then([&pm] (const result_value_type& rv){
                        pm.setValue(rv);
                    })
                    .orElse([&pm] (result_error_type&& er) {
                        pm.setError(std::move(er));
                    });
            }
        });

        return chainedFuture;
    }


    /**
     * Attach completion handler/callback to this future to be called when the future is resolved.
     *
     * @param completionHandler A completion handler to attach to this future.
     */
    template<typename F,
             typename R = typename std::result_of<F(T)>::type
             >
    std::enable_if_t<isFuture<R>::value, Future<typename R::value_type> >
    then(F&& f) {
        using result_value_type = typename R::value_type;

        Promise<result_value_type> promise;
        auto chainedFuture = promise.getFuture();

        _core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<T, Error>&& result) mutable {
            if (result.isError()) {
                pm.setError(result.moveError());
            } else {
                cont(result.unwrap())
                    .then([&pm] (const result_value_type& rv){
                        pm.setValue(rv);
                    })
                    .onError([&pm] (Error&& er) {
                        pm.setError(std::move(er));
                    });
            }
        });

        return chainedFuture;
    }

    /**
     * Specialization of continuation assigment method for functions returning void
     */
    template<typename F,
             typename R = typename std::result_of<F(T)>::type
             >
    std::enable_if_t<std::is_void<R>::value, Future<void> >
    then(F&& f);


    //------------------------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------

    /**
     * Attach error handler/callback to this future to be called when the future has failed.
     * @param f A completion handler to attach to this future.
     */
    template<typename F,
             typename R = typename std::result_of<F(Error)>::type
             >
    std::enable_if_t<!std::is_void<R>::value && !isSomeResult<R>::value && !isFuture<R>::value, Future<R>>
    onError(F&& f) {

        Promise<R> promise;
        auto chainedFuture = promise.getFuture();

        _core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<T, Error>&& result) mutable {
            if (result.isError()) {
                // FIXME: Handle exceptions in completion handler
                pm.setValue(cont(result.getError()));
            } else {
                pm.setValue(result.unwrap());
            }
        });

        return chainedFuture;
    }


    /**
     * Attach error handler/callback to this future to be called when the future has failed.
     * @param f A completion handler to attach to this future.
     */
    template<typename F,
             typename R = typename std::result_of<F(Error)>::type
             >
    std::enable_if_t<isSomeResult<R>::value, Future<typename R::value_type> >
    onError(F&& f) {
        using result_value_type = typename R::value_type;
        using result_error_type = typename R::error_type;

        Promise<result_value_type> promise;
        auto chainedFuture = promise.getFuture();

        _core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<T, Error>&& result) mutable {
            if (result.isError()) {
                cont(result.getError())
                    .then([&pm] (const result_value_type& rv){
                        pm.setValue(rv);
                    })
                    .orElse([&pm] (const result_error_type& er) {
                        pm.setError(er);
                    });
            } else {
                pm.setResult(result.unwrap());
            }
        });

        return chainedFuture;
    }


    /**
     * Attach error handler/callback to this future to be called when the future has failed.
     * @param f A completion handler to attach to this future.
     */
    template<typename F,
             typename R = typename std::result_of<F(Error)>::type
             >
    std::enable_if_t<isFuture<R>::value, Future<typename R::value_type> >
    onError(F&& f) {
        using result_value_type = typename R::value_type;

        Promise<result_value_type> promise;
        auto chainedFuture = promise.getFuture();

        _core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<T, Error>&& result) mutable {
            if (result.isError()) {
                cont(result.getError())
                    .then([&pm] (const result_value_type& rv){
                        pm.setValue(rv);
                    })
                    .onError([&pm] (Error&& er) {
                        pm.setError(std::move(er));
                    });
            } else {
                pm.setValue(result.unwrap());
            }

        });

        return chainedFuture;
    }

    /**
     * Attach error handler/callback to this future to be called when the future has failed.
     * Specialization for methods returning void.
     * @param f A completion handler to attach to this future.
     */
    template<typename F,
             typename R = typename std::result_of<F(Error)>::type
             >
    std::enable_if_t<std::is_void<R>::value, Future<void> >
    onError(F&& f);


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
     * @param completionHandler A completion handler to attach to this future.
     */
    template<typename F,
             typename R = typename std::result_of<F(void)>::type
             >
    std::enable_if_t<std::is_void<R>::value, Future<void>>
    then(F&& f) {
        Promise<void> promise;
        auto chainedFuture = promise.getFuture();

        _core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<void, Error>&& result) mutable {
            // TODO(abbyssoul): Handle exceptions!
            try {
                if (result.isError()) {
                    pm.setError(result.moveError());
                } else {
                    cont();
                    pm.setValue();
                }
            } catch (...) {
//                pm.setError(wrapExceptionIntoError);
            }
        });

        return chainedFuture;
    }

    template<typename F,
             typename R = typename std::result_of<F(void)>::type
             >
    std::enable_if_t<!std::is_void<R>::value && !isSomeResult<R>::value, Future<R>>
    then(F&& f) {
        Promise<R> promise;
        auto chainedFuture = promise.getFuture();

        _core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<void, Error>&& result) mutable {
            // TODO(abbyssoul): Handle exceptions!
            try {
                if (result.isError()) {
                    pm.setError(result.moveError());
                } else {
                    pm.setValue(cont());
                }
            } catch (...) {
//                pm.setError(wrapExceptionIntoError);
            }
        });

        return chainedFuture;
    }

    /**
     * Attach completion handler/callback to this future to be called when the future is resolved.
     *
     * @param completionHandler A completion handler to attach to this future.
     */
    template<typename F,
             typename R = typename std::result_of<F(void)>::type
             >
    std::enable_if_t<isSomeResult<R>::value, Future<typename R::value_type> >
    then(F&& f) {
        using result_value_type = typename R::value_type;
        using result_error_type = typename R::error_type;

        Promise<result_value_type> promise;
        auto chainedFuture = promise.getFuture();

        _core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<void, Error>&& result) mutable {
            if (result.isError()) {
                pm.setError(result.moveError());
            } else {
                cont()
                    .then([&pm] (const result_value_type& rv){
                        pm.setValue(rv);
                    })
                    .orElse([&pm] (const result_error_type& er) {
                        pm.setError(er);
                    });
            }
        });

        return chainedFuture;
    }

    //------------------------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------

//    /**
//     * Attach error handler/callback to this future to be called when the future has failed.
//     * @param f A completion handler to attach to this future.
//     */
//    template<typename F,
//             typename R = typename std::result_of<F(Error)>::type
//             >
//    std::enable_if_t<!std::is_void<R>::value && !isSomeResult<R>::value && !isFuture<R>::value, Future<R>>
//    onError(F&& f) {

//        Promise<R> promise;
//        auto chainedFuture = promise.getFuture();

//     _core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<void, Error>&& result) mutable {
//            if (result.isError()) {
//                // FIXME: Handle exceptions in completion handler
//                pm.setValue(cont(result.getError()));
//            } else {
//                pm.setValue(result.unwrap());
//            }
//        });

//        return chainedFuture;
//    }


//    /**
//     * Attach error handler/callback to this future to be called when the future has failed.
//     * @param f A completion handler to attach to this future.
//     */
//    template<typename F,
//             typename R = typename std::result_of<F(Error)>::type
//             >
//    std::enable_if_t<isSomeResult<R>::value, Future<typename R::value_type> >
//    onError(F&& f) {
//        using result_value_type = typename R::value_type;
//        using result_error_type = typename R::error_type;

//        Promise<result_value_type> promise;
//        auto chainedFuture = promise.getFuture();

//        _core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<T, Error>&& result) mutable {
//            if (result.isError()) {
//                cont(result.getError())
//                    .then([&pm] (const result_value_type& rv){
//                        pm.setValue(rv);
//                    })
//                    .orElse([&pm] (const result_error_type& er) {
//                        pm.setError(er);
//                    });
//            } else {
//                pm.setResult(result.unwrap());
//            }
//        });

//        return chainedFuture;
//    }


//    /**
//     * Attach error handler/callback to this future to be called when the future has failed.
//     * @param f A completion handler to attach to this future.
//     */
//    template<typename F,
//             typename R = typename std::result_of<F(Error)>::type
//             >
//    std::enable_if_t<isFuture<R>::value, Future<typename R::value_type> >
//    onError(F&& f) {
//        using result_value_type = typename R::value_type;

//        Promise<result_value_type> promise;
//        auto chainedFuture = promise.getFuture();

//        _core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<T, Error>&& result) mutable {
//            if (result.isError()) {
//                cont(result.getError())
//                    .then([&pm] (const result_value_type& rv){
//                        pm.setValue(rv);
//                    })
//                    .onError([&pm] (Error&& er) {
//                        pm.setError(std::move(er));
//                    });
//            } else {
//                pm.setValue(result.unwrap());
//            }

//        });

//        return chainedFuture;
//    }

    /**
     * Attach error handler/callback to this future to be called when the future has failed.
     * Specialization for methods returning void.
     * @param f A completion handler to attach to this future.
     */
    template<typename F,
             typename R = typename std::result_of<F(Error)>::type
             >
    std::enable_if_t<std::is_void<R>::value, Future<void> >
    onError(F&& f) {
        Promise<void> promise;
        auto chainedFuture = promise.getFuture();

        _core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<void, Error>&& result) mutable {
            // TODO(abbyssoul): Handle exceptions!
            try {
                if (result.isError()) {
                    cont(std::move(result.getError()));
                    pm.setValue();
                } else {
                    pm.setValue();
                }
            } catch (...) {
    //                pm.setError(wrapExceptionIntoError);
            }
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
Future<T>
Promise<T>::getFuture() {
    return Future<T>(_core);
}


inline Future<void>
Promise<void>::getFuture() {
    return Future<void>(_core);
}


template<typename T>
template<typename F,
         typename R
         >
std::enable_if_t<std::is_void<R>::value, Future<void>>
Future<T>::then(F&& f) {
    Promise<void> promise;
    auto chainedFuture = promise.getFuture();

    _core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<T, Error>&& result) mutable {
        // TODO(abbyssoul): Handle exceptions!
        try {
            if (result.isError()) {
                pm.setError(std::move(result).getError());
            } else {
                cont(result.unwrap());
                pm.setValue();
            }
        } catch (...) {
//                pm.setError(wrapExceptionIntoError);
        }
    });

    return chainedFuture;
}


template<typename T>
template<typename F,
         typename R
         >
std::enable_if_t<std::is_void<R>::value, Future<void>>
Future<T>::onError(F&& f) {
    Promise<void> promise;
    auto chainedFuture = promise.getFuture();

    _core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<T, Error>&& result) mutable {
        // TODO(abbyssoul): Handle exceptions!
        try {
            if (result.isError()) {
                cont(std::move(result.getError()));
                pm.setValue();
            } else {
                pm.setValue();
            }
        } catch (...) {
//                pm.setError(wrapExceptionIntoError);
        }
    });

    return chainedFuture;
}

}  // End of namespace async
}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_ASYNC_FUTUE_HPP
