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
#ifndef SOLACE_IO_ASYNC_FUTURE_IMPL_HPP
#define SOLACE_IO_ASYNC_FUTURE_IMPL_HPP

namespace Solace { namespace IO { namespace async { namespace details  {


/**
 * Then implementation helper.
 * Generic case
 */
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
            pm.setValue(cont(result.moveResult()));
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
            cont(result.moveResult());
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
            cont(result.moveResult())
                .then([this] (UnpuckedResultType&& rv){
                    pm.setValue(std::move(rv));
                })
                .orElse([this] (Error&& er) {
                    pm.setError(std::move(er));
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
            cont(result.moveResult())
                .then([this] () {
                    pm.setValue();
                })
                .orElse([this] (Error&& er) {
                    pm.setError(std::move(er));
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
                .then([this] (UnpuckedResultType&& rv){
                    pm.setValue(std::move(rv));
                })
                .orElse([this] (Error&& er) {
                    pm.setError(std::move(er));
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
                .orElse([this] (Error&& er) {
                    pm.setError(std::move(er));
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
            cont(result.moveResult())
                .then([this] (UnpuckedResultType&& rv) {
                    pm.setValue(std::move(rv));
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
            cont(result.moveResult())
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

// ---------------------------------------------------------------------------------------------------------------------
// onError implementation helper
// ---------------------------------------------------------------------------------------------------------------------

/**
 * Generic case
 */
template<typename FutureValueType,
         typename ContinuationResult,
         typename UnpuckedResultType,
         typename F>
struct ErrBack {
    Promise<UnpuckedResultType> pm;
    F cont;

    ErrBack(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<FutureValueType, Error>&& result) {
        if (result.isError()) {
            // FIXME: Handle exceptions in completion handler
            pm.setValue(cont(result.moveError()));
        } else {
            pm.setValue(result.moveResult());
        }
    }
};

template<
         typename ContinuationResult,
         typename UnpuckedResultType,
         typename F>
struct ErrBack<void, ContinuationResult, UnpuckedResultType, F> {
    Promise<UnpuckedResultType> pm;
    F cont;

    ErrBack(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) {
        if (result.isError()) {
            // FIXME: Handle exceptions in completion handler
            pm.setValue(cont(result.moveError()));
        } else {
            pm.setValue();
        }
    }
};

template<
         typename F>
struct ErrBack<void, void, void, F> {
    Promise<void> pm;
    F cont;

    ErrBack(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) {
        if (result.isError()) {
            // FIXME: Handle exceptions in completion handler
            cont(result.moveError());
            pm.setValue();
        } else {
            pm.setValue();
        }
    }
};


template<typename FutureValueType,
         typename F>
struct ErrBack<FutureValueType, void, void, F> {
    Promise<void> pm;
    F cont;

    ErrBack(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<FutureValueType, Error>&& result) {
        if (result.isError()) {
            // FIXME: Handle exceptions in completion handler
//            try {
            cont(result.moveError());
            pm.setValue();
//            } catch (...) {
//                pm.setError(wrapExceptionIntoError);
//            }
        } else {
            pm.setValue();
        }
    }
};



/**
 * Generic case of error handler returning Result<>
 */
template<typename FutureValueType,
         typename UnpuckedResultType,
         typename F>
struct ErrBack<FutureValueType, Result<UnpuckedResultType, Error>, UnpuckedResultType, F> {
    Promise<UnpuckedResultType> pm;
    F cont;

    ErrBack(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<FutureValueType, Error>&& result) {
        if (result.isError()) {
            cont(result.moveError())
                .then([this] (UnpuckedResultType&& rv){
                    pm.setValue(std::move(rv));
                })
                .orElse([this] (Error&& er) {
                    pm.setError(std::move(er));
                });
        } else {
            pm.setValue(result.moveResult());
        }
    }
};

template<
         typename UnpuckedResultType,
         typename F>
struct ErrBack<void, Result<UnpuckedResultType, Error>, UnpuckedResultType, F> {
    Promise<UnpuckedResultType> pm;
    F cont;

    ErrBack(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) {
        if (result.isError()) {
            cont(result.moveError())
                .then([this] (UnpuckedResultType&& rv){
                    pm.setValue(std::move(rv));
                })
                .orElse([this] (Error&& er) {
                    pm.setError(std::move(er));
                });
        } else {
            pm.setValue();
        }
    }
};


template<typename FutureValueType,
         typename F>
struct ErrBack<FutureValueType, Result<void, Error>, void, F> {
    Promise<void> pm;
    F cont;

    ErrBack(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<FutureValueType, Error>&& result) {
        if (result.isError()) {
            cont(result.moveError())
                .then([this] () {
                    pm.setValue();
                })
                .orElse([this] (Error&& er) {
                    pm.setError(std::move(er));
                });
        } else {
            pm.setValue();
        }
    }
};


template<
         typename F>
struct ErrBack<void, Result<void, Error>, void, F> {
    Promise<void> pm;
    F cont;

    ErrBack(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) {
        if (result.isError()) {
            cont(result.moveError())
                .then([this] () {
                    pm.setValue();
                })
                .orElse([this] (Error&& er) {
                    pm.setError(std::move(er));
                });
        } else {
            pm.setValue();
        }
    }
};




/**
 * Generic case of error handler returning Future<>
 */
template<typename FutureValueType,
         typename UnpuckedResultType,
         typename F>
struct ErrBack<FutureValueType, Future<UnpuckedResultType>, UnpuckedResultType, F> {
    Promise<UnpuckedResultType> pm;
    F cont;

    ErrBack(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<FutureValueType, Error>&& result) {
        if (result.isError()) {
            cont(result.moveError())
                .then([this] (UnpuckedResultType&& rv) {
                    pm.setValue(std::move(rv));
                })
                .onError([this] (Error&& er) {
                    pm.setError(std::move(er));
                });
        } else {
            pm.setValue(result.moveResult());
        }
    }
};

template<
         typename UnpuckedResultType,
         typename F>
struct ErrBack<void, Future<UnpuckedResultType>, UnpuckedResultType, F> {
    Promise<UnpuckedResultType> pm;
    F cont;

    ErrBack(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) {
        if (result.isError()) {
            cont(result.moveError())
                .then([this] (UnpuckedResultType&& rv) {
                    pm.setValue(std::move(rv));
                })
                .onError([this] (Error&& er) {
                    pm.setError(std::move(er));
                });
        } else {
            pm.setValue();
        }
    }
};

template<typename FutureValueType,
         typename F>
struct ErrBack<FutureValueType, Future<void>, void, F> {
    Promise<void> pm;
    F cont;

    ErrBack(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<FutureValueType, Error>&& result) {
        if (result.isError()) {
            cont(result.moveError())
                .then([this] () {
                    pm.setValue();
                })
                .onError([this] (Error&& er) {
                    pm.setError(std::move(er));
                });
        } else {
            pm.setValue(result.moveResult());
        }
    }
};

template<
         typename F>
struct ErrBack<void, Future<void>, void, F> {
    Promise<void> pm;
    F cont;

    ErrBack(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) {
        if (result.isError()) {
            cont(result.moveError())
                .then([this] () {
                    pm.setValue();
                })
                .onError([this] (Error&& er) {
                    pm.setError(std::move(er));
                });
        } else {
            pm.setValue();
        }
    }
};


}  // End of namespace details
}  // End of namespace async
}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_ASYNC_FUTURE_IMPL_HPP
