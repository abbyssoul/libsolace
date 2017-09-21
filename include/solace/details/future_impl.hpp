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
 * libSolace: Implementation details of Future callback types
 *	@file		solace/details/future_impl.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_DEATILS_FUTURE_IMPL_HPP
#define SOLACE_DEATILS_FUTURE_IMPL_HPP

#include "solace/promise.hpp"


namespace Solace { namespace details  {


/**
 * Then implementation helper.
 * Generic case
 */
template<typename FutureValueType,
         typename ContinuationResult,
         typename UnpuckedResultType,
         typename F>
struct CB : public CallbackBase<FutureValueType> {
    Promise<UnpuckedResultType> pm;
    F cont;

    CB(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<FutureValueType, Error>&& result) override {
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
struct CB<void, UnpuckedResultType, UnpuckedResultType, F> : public CallbackBase<void> {
    Promise<UnpuckedResultType> pm;
    F cont;

    CB(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) override {
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
struct CB<FutureValueType, void, void, F> : public CallbackBase<FutureValueType>{
    Promise<void> pm;
    F cont;

    CB(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<FutureValueType, Error>&& result) override {
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
struct CB<void, void, void, F> : public CallbackBase<void>{
    Promise<void> pm;
    F cont;

    CB(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) override {
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
struct CB<T, Result<UnpuckedResultType, Error>, UnpuckedResultType, F> : public CallbackBase<T> {
    typedef CB<T, Result<UnpuckedResultType, Error>, UnpuckedResultType, F> CallbackType;

    Promise<UnpuckedResultType> pm;
    F cont;

    CB(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<T, Error>&& result) override {
        if (result.isError()) {
            pm.setError(result.moveError());
        } else {
            cont(result.moveResult())
                .then([self = this->shared_from_this()] (UnpuckedResultType&& rv){
                    std::static_pointer_cast<CallbackType>(self)->pm.setValue(std::move(rv));
                })
                .orElse([self = this->shared_from_this()] (Error&& er) {
                    std::static_pointer_cast<CallbackType>(self)->pm.setError(std::move(er));
                });
        }
    }
};


/**
 * Spacialization for Future<T> and continuation F returning Result<void>
 */
template<typename T,
         typename F>
struct CB<T, Result<void, Error>, void, F> : public CallbackBase<T> {
    typedef CB<T, Result<void, Error>, void, F> CallbackType;

    Promise<void> pm;
    F cont;

    CB(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<T, Error>&& result) override {
        if (result.isError()) {
            pm.setError(result.moveError());
        } else {
            cont(result.moveResult())
                .then([self = this->shared_from_this()] () {
                    std::static_pointer_cast<CallbackType>(self)->pm.setValue();
                })
                .orElse([self = this->shared_from_this()] (Error&& er) {
                    std::static_pointer_cast<CallbackType>(self)->pm.setError(std::move(er));
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
struct CB<void, Result<UnpuckedResultType, Error>, UnpuckedResultType, F> : public CallbackBase<void> {
    typedef CB<void, Result<UnpuckedResultType, Error>, UnpuckedResultType, F> CallbackType;

    Promise<UnpuckedResultType> pm;
    F cont;

    CB(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) override {
        if (result.isError()) {
            pm.setError(result.moveError());
        } else {
            cont()
                .then([self = shared_from_this()] (UnpuckedResultType&& rv){
                    std::static_pointer_cast<CallbackType>(self)->pm.setValue(std::move(rv));
                })
                .orElse([self = shared_from_this()] (Error&& er) {
                    std::static_pointer_cast<CallbackType>(self)->pm.setError(std::move(er));
                });
        }
    }
};

/**
 * Spacialization for Future<void> and continuation F returning Result<void>
 */
template<typename F>
struct CB<void, Result<void, Error>, void, F> : public CallbackBase<void> {
    typedef CB<void, Result<void, Error>, void, F> CallbackType;

    Promise<void> pm;
    F cont;

    CB(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) override {
        if (result.isError()) {
            pm.setError(result.moveError());
        } else {
            cont()
                .then([self = shared_from_this()] () {
                    std::static_pointer_cast<CallbackType>(self)->pm.setValue();
                })
                .orElse([self = shared_from_this()] (Error&& er) {
                    std::static_pointer_cast<CallbackType>(self)->pm.setError(std::move(er));
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
struct CB<T, Future<UnpuckedResultType>, UnpuckedResultType, F> : public CallbackBase<T> {
    typedef CB<T, Future<UnpuckedResultType>, UnpuckedResultType, F> CallbackType;

    Promise<UnpuckedResultType> pm;
    F cont;

    CB(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<T, Error>&& result) override {
        if (result.isError()) {
            pm.setError(result.moveError());
        } else {
            cont(result.moveResult())
                .then([self = this->shared_from_this()] (UnpuckedResultType&& rv) {
                    std::static_pointer_cast<CallbackType>(self)->pm.setValue(std::move(rv));
                })
                .onError([self = this->shared_from_this()] (Error&& er) {
                    std::static_pointer_cast<CallbackType>(self)->pm.setError(std::move(er));
                });
        }
    }
};

/**
 * Spacialization for Future<T> and continuation F returning Future<void>
 */
template<typename T,
         typename F>
struct CB<T, Future<void>, void, F> : public CallbackBase<T> {
    typedef CB<T, Future<void>, void, F> CallbackType;

    Promise<void> pm;
    F cont;

    CB(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<T, Error>&& result) override {
        if (result.isError()) {
            pm.setError(result.moveError());
        } else {
            cont(result.moveResult())
                .then([self = this->shared_from_this()] () {
                    std::static_pointer_cast<CallbackType>(self)->pm.setValue();
                })
                .onError([self = this->shared_from_this()] (Error&& er) {
                    std::static_pointer_cast<CallbackType>(self)->pm.setError(std::move(er));
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
struct CB<void, Future<UnpuckedResultType>, UnpuckedResultType, F> : public CallbackBase<void> {
    typedef CB<void, Future<UnpuckedResultType>, UnpuckedResultType, F> CallbackType;

    Promise<UnpuckedResultType> pm;
    F cont;

    CB(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) override {
        if (result.isError()) {
            pm.setError(result.moveError());
        } else {
            cont()
                .then([self = this->shared_from_this()] (UnpuckedResultType&& rv){
                    std::static_pointer_cast<CallbackType>(self)->pm.setValue(std::move(rv));
                })
                .onError([self = this->shared_from_this()] (Error&& er) {
                    std::static_pointer_cast<CallbackType>(self)->pm.setError(std::move(er));
                });
        }
    }
};


/**
 * Spacialization for Future<void> and continuation F returning Future<void>
 */
template<typename F>
struct CB<void, Future<void>, void, F> : public CallbackBase<void> {
    typedef CB<void, Future<void>, void, F> CallbackType;

    Promise<void> pm;
    F cont;

    CB(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) override {
        if (result.isError()) {
            pm.setError(result.moveError());
        } else {
            cont()
                .then([self = this->shared_from_this()] () {
                    std::static_pointer_cast<CallbackType>(self)->pm.setValue();
                })
                .onError([self = this->shared_from_this()] (Error&& er) {
                    std::static_pointer_cast<CallbackType>(self)->pm.setError(std::move(er));
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
struct ErrBack : public CallbackBase<FutureValueType>{
    Promise<UnpuckedResultType> pm;
    F cont;

    ErrBack(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<FutureValueType, Error>&& result) override {
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
struct ErrBack<void, ContinuationResult, UnpuckedResultType, F> : public CallbackBase<void> {
    Promise<UnpuckedResultType> pm;
    F cont;

    ErrBack(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) override {
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
struct ErrBack<void, void, void, F> : public CallbackBase<void> {
    Promise<void> pm;
    F cont;

    ErrBack(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) override {
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
struct ErrBack<FutureValueType, void, void, F> : public CallbackBase<FutureValueType> {
    Promise<void> pm;
    F cont;

    ErrBack(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<FutureValueType, Error>&& result) override {
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
struct ErrBack<FutureValueType, Result<UnpuckedResultType, Error>, UnpuckedResultType, F> :
        public CallbackBase<FutureValueType> {
    typedef ErrBack<FutureValueType, Result<UnpuckedResultType, Error>, UnpuckedResultType, F> CallbackType;

    Promise<UnpuckedResultType> pm;
    F cont;

    ErrBack(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<FutureValueType, Error>&& result) override {
        if (result.isError()) {
            cont(result.moveError())
                .then([self = this->shared_from_this()] (UnpuckedResultType&& rv){
                    std::static_pointer_cast<CallbackType>(self)->pm.setValue(std::move(rv));
                })
                .orElse([self = this->shared_from_this()] (Error&& er) {
                    std::static_pointer_cast<CallbackType>(self)->pm.setError(std::move(er));
                });
        } else {
            pm.setValue(result.moveResult());
        }
    }
};

template<
         typename UnpuckedResultType,
         typename F>
struct ErrBack<void, Result<UnpuckedResultType, Error>, UnpuckedResultType, F> : public CallbackBase<void> {
    Promise<UnpuckedResultType> pm;
    F cont;

    ErrBack(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) override {
        if (result.isError()) {
            cont(result.moveError())
                .then([self = shared_from_this()] (UnpuckedResultType&& rv){
                    self->pm.setValue(std::move(rv));
                })
                .orElse([self = shared_from_this()] (Error&& er) {
                    self->pm.setError(std::move(er));
                });
        } else {
            pm.setValue();
        }
    }
};


template<typename FutureValueType,
         typename F>
struct ErrBack<FutureValueType, Result<void, Error>, void, F> : public CallbackBase<FutureValueType> {
    Promise<void> pm;
    F cont;

    ErrBack(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<FutureValueType, Error>&& result) override {
        if (result.isError()) {
            cont(result.moveError())
                .then([self = this->shared_from_this()] () {
                    self->pm.setValue();
                })
                .orElse([self = this->shared_from_this()] (Error&& er) {
                    self->pm.setError(std::move(er));
                });
        } else {
            pm.setValue();
        }
    }
};


template<
         typename F>
struct ErrBack<void, Result<void, Error>, void, F> : public CallbackBase<void> {
    typedef ErrBack<void, Result<void, Error>, void, F> CallbackType;

    Promise<void> pm;
    F cont;

    ErrBack(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) override {
        if (result.isError()) {
            cont(result.moveError())
                .then([self = this->shared_from_this()] () {
                    std::static_pointer_cast<CallbackType>(self)->pm.setValue();
                })
                .orElse([self = this->shared_from_this()] (Error&& er) {
                    std::static_pointer_cast<CallbackType>(self)->pm.setError(std::move(er));
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
struct ErrBack<FutureValueType, Future<UnpuckedResultType>, UnpuckedResultType, F> :
        public CallbackBase<FutureValueType> {
    Promise<UnpuckedResultType> pm;
    F cont;

    ErrBack(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<FutureValueType, Error>&& result) override {
        if (result.isError()) {
            cont(result.moveError())
                .then([self = this->shared_from_this()] (UnpuckedResultType&& rv) {
                    self->pm.setValue(std::move(rv));
                })
                .onError([self = this->shared_from_this()] (Error&& er) {
                    self->pm.setError(std::move(er));
                });
        } else {
            pm.setValue(result.moveResult());
        }
    }
};

template<
         typename UnpuckedResultType,
         typename F>
struct ErrBack<void, Future<UnpuckedResultType>, UnpuckedResultType, F> : public CallbackBase<void> {
    Promise<UnpuckedResultType> pm;
    F cont;

    ErrBack(F&& f, Promise<UnpuckedResultType>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) override {
        if (result.isError()) {
            cont(result.moveError())
                .then([self = this->shared_from_this()] (UnpuckedResultType&& rv) {
                    self->pm.setValue(std::move(rv));
                })
                .onError([self = this->shared_from_this()] (Error&& er) {
                    self->pm.setError(std::move(er));
                });
        } else {
            pm.setValue();
        }
    }
};

template<typename FutureValueType,
         typename F>
struct ErrBack<FutureValueType, Future<void>, void, F> : public CallbackBase<FutureValueType> {
    Promise<void> pm;
    F cont;

    ErrBack(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<FutureValueType, Error>&& result) override {
        if (result.isError()) {
            cont(result.moveError())
                .then([self = this->shared_from_this()] () {
                    self->pm.setValue();
                })
                .onError([self = this->shared_from_this()] (Error&& er) {
                    self->pm.setError(std::move(er));
                });
        } else {
            pm.setValue(result.moveResult());
        }
    }
};

template<
         typename F>
struct ErrBack<void, Future<void>, void, F> : public CallbackBase<void> {
    Promise<void> pm;
    F cont;

    ErrBack(F&& f, Promise<void>&& p) : pm(std::move(p)), cont(std::forward<F>(f))
    {}

    void operator() (Result<void, Error>&& result) override {
        if (result.isError()) {
            cont(result.moveError())
                .then([self = this->shared_from_this()] () {
                    self->pm.setValue();
                })
                .onError([self = this->shared_from_this()] (Error&& er) {
                    self->pm.setError(std::move(er));
                });
        } else {
            pm.setValue();
        }
    }
};


}  // End of namespace details
}  // End of namespace Solace
#endif  // SOLACE_DEATILS_FUTURE_IMPL_HPP
