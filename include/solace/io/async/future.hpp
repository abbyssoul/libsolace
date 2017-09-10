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

template<typename result_type,
         typename error_type>
struct ErrorHelper {

    template<typename F>
    void propageteError(F&& f, Error&& error, Promise<result_type>& promise) {
        f(std::move(error))
            .then([&promise] (const result_type& rv){
                promise.setValue(rv);
            })
            .orElse([&promise] (const error_type& er) {
                promise.setError(er);
            });
    }


    void propageteResult(Promise<result_type>& promise, result_type&& result) {
        promise.setValue(result);
    }
};

template<typename error_type>
struct ErrorHelper<void, error_type> {

    template<typename F>
    void propageteError(F&& f, Error&& error, Promise<void>& promise) {
        f(std::move(error))
            .then([&promise] () {
                promise.setValue();
            })
            .orElse([&promise] (error_type&& er) {
                promise.setError(std::move(er));
            });
    }

    template<typename T>
    void propageteResult(Promise<void>& promise, T&&) {
        promise.setValue();
    }
};



template<typename FutureValueType,
         typename ContinuationResult,
         typename UnpuckedResultType, typename UnpackedErrorResultType,
         typename F>
Future<UnpuckedResultType> thenImplementation(std::shared_ptr<Core<FutureValueType>>&& core, F&& f);

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

        auto core = _core.lock();
        if (!core) {
            raiseInvalidStateError("Invalid Future without a Promise");
        }

        return details::thenImplementation<T, R, UnpackedRT, UnpackedET>(std::move(core), std::forward<F>(f));
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

        auto core = _core.lock();
        if (!core) {
            raiseInvalidStateError("Invalid Future without a Promise");
        }

        return details::thenImplementation<T, R, R, error_type>(std::move(core), std::forward<F>(f));
    }


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
        auto core = _core.lock();
        if (!core) {
            raiseInvalidStateError("Invalid Future without a Promise");
        }

        Promise<R> promise;
        auto chainedFuture = promise.getFuture();

        core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<T, Error>&& result) mutable {
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

        auto core = _core.lock();
        if (!core) {
            raiseInvalidStateError("Invalid Future without a Promise");
        }

        Promise<result_value_type> promise;
        auto chainedFuture = promise.getFuture();

        core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<T, Error>&& result) mutable {
            if (result.isError()) {
                details::ErrorHelper<result_value_type, result_error_type>().propageteError(
                            std::forward<F>(cont),
                            result.moveError(),
                            pm);
            } else {
                details::ErrorHelper<result_value_type, result_error_type>().propageteResult(pm, result.unwrap());
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

        auto core = _core.lock();
        if (!core) {
            raiseInvalidStateError("Invalid Future without a Promise");
        }

        Promise<result_value_type> promise;
        auto chainedFuture = promise.getFuture();

        core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<T, Error>&& result) mutable {
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

    Future(const std::shared_ptr<Core<T>>& core): _core{core}
    { }

private:

    std::weak_ptr<Core<T>> _core;

};


/** Specialization of AsyncResult for void type
 *
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

        auto core = _core.lock();
        if (!core) {
            raiseInvalidStateError("Invalid Future without a Promise");
        }

        return details::thenImplementation<void, R, UnpackedRT, UnpackedET>(std::move(core), std::forward<F>(f));
    }


    template<typename F,
             typename R = typename std::result_of<F(void)>::type
             >
    std::enable_if_t<!isSomeResult<R>::value && !isFuture<R>::value, Future<R>>
    then(F&& f) {

        auto core = _core.lock();
        if (!core) {
            raiseInvalidStateError("Invalid Future without a Promise");
        }

        return details::thenImplementation<void, R, R, Error>(std::move(core), std::forward<F>(f));
    }


    /**
     * Attach completion handler to this future to be called when the future is resolved.
     * This is a special case of completion handlers return void this result in Future<void>
     * @param completionHandler A completion handler to attach to this future.
     */
    /*
    template<typename F,
             typename R = typename std::result_of<F(void)>::type
             >
    std::enable_if_t<std::is_void<R>::value, Future<void>>
    then(F&& f) {
        auto core = _core.lock();
        if (!core) {
            raiseInvalidStateError("Invalid Future without a Promise");
        }

        Promise<void> promise;
        auto chainedFuture = promise.getFuture();

        core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<void, Error>&& result) mutable {
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
*/

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

        auto core = _core.lock();
        if (!core) {
            raiseInvalidStateError("Invalid Future without a Promise");
        }

        Promise<result_value_type> promise;
        auto chainedFuture = promise.getFuture();

        core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<void, Error>&& result) mutable {
            if (result.isError()) {
                details::ErrorHelper<result_value_type, result_error_type>().propageteError(
                            std::forward<F>(cont),
                            result.moveError(),
                            pm);
            } else {
                details::ErrorHelper<result_value_type, result_error_type>().propageteResult(pm, 0);
            }
        });

        return chainedFuture;
    }


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
        auto core = _core.lock();
        if (!core) {
            raiseInvalidStateError("Invalid Future without a Promise");
        }

        Promise<void> promise;
        auto chainedFuture = promise.getFuture();

        core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<void, Error>&& result) mutable {
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

/*
template<typename T>
template<typename F,
         typename R
         >
std::enable_if_t<std::is_void<R>::value, Future<void>>
Future<T>::then(F&& f) {

    auto core = _core.lock();
    if (!core) {
        raiseInvalidStateError("Invalid Future without a Promise");
    }

    Promise<void> promise;
    auto chainedFuture = promise.getFuture();

    core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<T, Error>&& result) mutable {
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
*/

template<typename T>
template<typename F,
         typename R
         >
std::enable_if_t<std::is_void<R>::value, Future<void>>
Future<T>::onError(F&& f) {
    auto core = _core.lock();
    if (!core) {
        raiseInvalidStateError("Invalid Future without a Promise");
    }

    Promise<void> promise;
    auto chainedFuture = promise.getFuture();

    core->setCallback([cont = std::forward<F>(f), pm = std::move(promise)] (Result<T, Error>&& result) mutable {
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


template<typename FutureValueType,
         typename ContinuationResult,
         typename UnpuckedResultType,
         typename F>
struct CB {
    Promise<UnpuckedResultType> pm;
    F cont;

    CB(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<FutureValueType, Error>&& result) {
        if (result.isError()) {
            pm.setError(result.moveError());
        } else {
            // FIXME: Handle exceptions in completion handler
            pm.setValue(cont(result.unwrap()));
        }
    }
};


/**
 * Spacialization for Future<void> and continuation F returning R
 */
template<typename UnpuckedResultType,
         typename F>
struct CB<void, UnpuckedResultType, UnpuckedResultType, F> {
    Promise<UnpuckedResultType> pm;
    F cont;

    CB(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) {
        if (result.isError()) {
            pm.setError(result.moveError());
        } else {
            // FIXME: Handle exceptions in completion handler
            pm.setValue(cont());
        }
    }
};


/**
 * Spacialization for Future<T> and continuation F returning Void
 */
template<typename FutureValueType,
         typename F>
struct CB<FutureValueType, void, void, F> {
    Promise<void> pm;
    F cont;

    CB(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<FutureValueType, Error>&& result) {
        if (result.isError()) {
            pm.setError(result.moveError());
        } else {
            // FIXME: Handle exceptions in completion handler
            cont(result.unwrap());
            pm.setValue();
        }
    }
};


/**
 * Spacialization for Future<void> and continuation F returning Void
 */
template<typename F>
struct CB<void, void, void, F> {
    Promise<void> pm;
    F cont;

    CB(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) {
        if (result.isError()) {
            pm.setError(result.moveError());
        } else {
            // FIXME: Handle exceptions in completion handler
            cont();
            pm.setValue();
        }
    }
};

// ---------------------------------------------------------------------------------------------------------------------
// Speciazition of callback wrapper for Result type
// ---------------------------------------------------------------------------------------------------------------------

/**
 * Spacialization for Future<T> and continuation F returning Result<R>
 */
template<typename T,
         typename UnpuckedResultType,
         typename F>
struct CB<T, Result<UnpuckedResultType, Error>, UnpuckedResultType, F> {
    Promise<UnpuckedResultType> pm;
    F cont;

    CB(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<T, Error>&& result) {
        if (result.isError()) {
            pm.setError(result.moveError());
        } else {
            cont(result.unwrap())
                .then([this] (const UnpuckedResultType& rv){
                    pm.setValue(rv);
                })
                .orElse([this] (const Error& er) {
                    pm.setError(er);
                });
        }
    }
};


/**
 * Spacialization for Future<T> and continuation F returning Result<void>
 */
template<typename T,
         typename F>
struct CB<T, Result<void, Error>, void, F> {
    Promise<void> pm;
    F cont;

    CB(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<T, Error>&& result) {
        if (result.isError()) {
            pm.setError(result.moveError());
        } else {
            cont(result.unwrap())
                .then([this] () {
                    pm.setValue();
                })
                .orElse([this] (const Error& er) {
                    pm.setError(er);
                });
        }
    }
};


/**
 * Spacialization for Future<void> and continuation F returning Result<R>
 */
template<
         typename UnpuckedResultType,
         typename F>
struct CB<void, Result<UnpuckedResultType, Error>, UnpuckedResultType, F> {
    Promise<UnpuckedResultType> pm;
    F cont;

    CB(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) {
        if (result.isError()) {
            pm.setError(result.moveError());
        } else {
            cont()
                .then([this] (const UnpuckedResultType& rv){
                    pm.setValue(rv);
                })
                .orElse([this] (const Error& er) {
                    pm.setError(er);
                });
        }
    }
};

/**
 * Spacialization for Future<void> and continuation F returning Result<void>
 */
template<typename F>
struct CB<void, Result<void, Error>, void, F> {
    Promise<void> pm;
    F cont;

    CB(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) {
        if (result.isError()) {
            pm.setError(result.moveError());
        } else {
            cont()
                .then([this] () {
                    pm.setValue();
                })
                .orElse([this] (const Error& er) {
                    pm.setError(er);
                });
        }
    }
};


// ---------------------------------------------------------------------------------------------------------------------
// Speciazition of callback wrapper for Future type
// ---------------------------------------------------------------------------------------------------------------------

/**
 * Spacialization for Future<T> and continuation F returning Future<R>
 */
template<typename T,
         typename UnpuckedResultType,
         typename F>
struct CB<T, Future<UnpuckedResultType>, UnpuckedResultType, F> {
    Promise<UnpuckedResultType> pm;
    F cont;

    CB(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<T, Error>&& result) {
        if (result.isError()) {
            pm.setError(result.moveError());
        } else {
            cont(result.unwrap())
                .then([this] (const UnpuckedResultType& rv) {
                    pm.setValue(rv);
//                    pm.setValue(std::move(rv));
                })
                .onError([this] (Error&& er) {
                    pm.setError(std::move(er));
                });
        }
    }
};

/**
 * Spacialization for Future<T> and continuation F returning Future<void>
 */
template<typename T,
         typename F>
struct CB<T, Future<void>, void, F> {
    Promise<void> pm;
    F cont;

    CB(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<T, Error>&& result) {
        if (result.isError()) {
            pm.setError(result.moveError());
        } else {
            cont(result.unwrap())
                .then([this] () {
                    pm.setValue();
                })
                .onError([this] (Error&& er) {
                    pm.setError(std::move(er));
                });
        }
    }
};


/**
 * Spacialization for Future<void> and continuation F returning Future<R>
 */
template<
         typename UnpuckedResultType,
         typename F>
struct CB<void, Future<UnpuckedResultType>, UnpuckedResultType, F> {
    Promise<UnpuckedResultType> pm;
    F cont;

    CB(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) {
        if (result.isError()) {
            pm.setError(result.moveError());
        } else {
            cont()
                .then([this] (UnpuckedResultType&& rv){
                    pm.setValue(std::move(rv));
                })
                .onError([this] (Error&& er) {
                    pm.setError(std::move(er));
                });
        }
    }
};


/**
 * Spacialization for Future<void> and continuation F returning Future<void>
 */
template<typename F>
struct CB<void, Future<void>, void, F> {
    Promise<void> pm;
    F cont;

    CB(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) {
        if (result.isError()) {
            pm.setError(result.moveError());
        } else {
            cont()
                .then([this] () {
                    pm.setValue();
                })
                .onError([this] (Error&& er) {
                    pm.setError(std::move(er));
                });
        }
    }
};



namespace details {
template<typename T,
         typename ContinuationResult,
         typename UnpuckedResult, typename UnpackedErrorResultType,
         typename F>
Future<UnpuckedResult> thenImplementation(std::shared_ptr<Core<T>>&& core, F&& f) {
    Promise<UnpuckedResult> promise;
    auto chainedFuture = promise.getFuture();

    core->setCallback(CB<T, ContinuationResult, UnpuckedResult, F>(std::forward<F>(f), std::move(promise)));

    return chainedFuture;
}
}  // namespace details


}  // End of namespace async
}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_ASYNC_FUTUE_HPP
