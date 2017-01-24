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
 * libSolace: Functional alternative to exeption throwing
 *	@file		solace/result.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_RESULT_HPP
#define SOLACE_RESULT_HPP

#include "solace/types.hpp"
#include "solace/assert.hpp"


#include <functional>


namespace Solace {

// Implementation types
namespace types {


template<typename T>
struct Ok {
    Ok(const T& val) : val_(val) { }
    Ok(T&& val) : val_(std::move(val)) { }

    T val_;
};

template<>
struct Ok<void> { };

template<typename E>
struct Err {
    Err(const E& val) : val_(val) { }
    Err(E&& val) : val_(std::move(val)) { }

    E val_;
};

}  // End of namespace types


/**
 * Syntactic sugar to return successful Result
 * Note: this function is a producer / constructor of type T
 *
 * @return type::Ok<T> that can be converted into a successful Result<T, E>
 */
template<typename T,
         typename CleanT = typename std::decay<T>::type>
inline types::Ok<CleanT> Ok(T&& val) {
    return types::Ok<CleanT>(std::forward<T>(val));
}

/**
 * A specialisation of Ok helper function to construct successful Result<Void, E>
 * @return type::Ok<T> that can be converted into a successful Result<T, E>
 */
inline types::Ok<void> Ok() {
    return types::Ok<void>();
}


/**
 * Syntactic sugar to produce error Result<T, E>
 * Note: this function is a producer / constructor of type E
 *
 * @return type::Err<T> that can be converted into a failed Result<T, E>
 */
template<typename E, typename CleanE = typename std::decay<E>::type>
inline types::Err<CleanE> Err(E&& val) {
    return types::Err<CleanE>(std::forward<E>(val));
}


/**
 * Result is function wrapping for function execution result and an alternative to exception throwing.
 *
 * Note: Result<void, Error> is technically equivalent of Optional<Error>
 */
template <typename V, typename E>
// TODO(c++17): [[nodiscard]]
class Result {
public:
    typedef V value_type;
    typedef E error_type;

//    static_assert(!std::is_same<V, E>::value,
//            "Result must have distinct types for value and error");

    static_assert(!std::is_same<E, void>::value,
            "Error type must be non-void");

public:

//    Result(const V& value):
//        _state( ::new (_stateBuffer.okSpace) OkState(value) )
//    {}

//    Result(V&& value):
//        _state( ::new (_stateBuffer.okSpace) OkState(std::move(value)) )
//    {}

//    Result(const V*, const E& err):
//        _state( ::new (_stateBuffer.errSpace) ErrorState(err) )
//    {}

//    Result(const V*, E && err):
//        _state( ::new (_stateBuffer.errSpace) ErrorState(std::move(err)) )
//    {}

    Result(const types::Ok<V>& value):
        _state( ::new (_stateBuffer.okSpace) OkState(value) )
    {}

    Result(types::Ok<V>&& value):
        _state( ::new (_stateBuffer.okSpace) OkState(std::move(value)) )
    {}

    template<typename DV>
    Result(types::Ok<DV>&& value):
        _state( ::new (_stateBuffer.okSpace) OkState(std::move(value.val_)) )
    {}

    Result(const types::Err<E>& err):
        _state( ::new (_stateBuffer.errSpace) ErrorState(err) )
    {}

    Result(types::Err<E>&& err):
        _state( ::new (_stateBuffer.errSpace) ErrorState(std::move(err)) )
    {}


    Result(const Result& rhs) noexcept :
        _state(rhs.isOk()
               ? static_cast<IState*>(::new (_stateBuffer.okSpace) OkState(rhs.getResult()))
               : static_cast<IState*>(::new (_stateBuffer.errSpace) ErrorState(rhs.getError())))
    {
    }

    Result(Result&& rhs) noexcept :
        _state(rhs.isOk()
               ? static_cast<IState*>(::new (_stateBuffer.okSpace) OkState(rhs.getResult()))
               : static_cast<IState*>(::new (_stateBuffer.errSpace) ErrorState(rhs.getError())))
    {
    }

    template<typename DV>
    Result(const Result<DV, E>& rhs) noexcept :
        _state(rhs.isOk()
               ? static_cast<IState*>(::new (_stateBuffer.okSpace) OkState(rhs.getResult()))
               : static_cast<IState*>(::new (_stateBuffer.errSpace) ErrorState(rhs.getError())))
    {
    }


    ~Result() {
        clear();
    }


    template<typename D>
    auto then(const std::function<D(V)>& success,
              const std::function<D(E)>& failure) -> D {

        return isOk()
                ? success(getResult())
                : failure(getError());
    }

    template<typename F,
             typename U = typename std::result_of<F(V)>::type>
    Result<U, E> then(F f) {
        if (isOk())
            return Ok<U>(f(getResult()));

        return Err(getError());
    }

    template<typename F,
             typename U = typename std::result_of<F(E)>::type>
    Result<U, E> orElse(F f) {
        return (isOk())
                ? *this
                : Ok<U>(f(getError()));
    }

    Result& swap(Result& rhs) noexcept {

        if (isOk()) {
            V v{ std::move(this->getResult()) };

            if (rhs.isOk())
                _state = ::new (_stateBuffer.okSpace) OkState(std::move(rhs.getResult()));
            else
                _state = ::new (_stateBuffer.errSpace) ErrorState(std::move(rhs.getError()));

            rhs.assign(v);
        } else {
            E e{ std::move(getError()) };

            if (rhs.isOk())
                _state = ::new (_stateBuffer.okSpace) OkState(std::move(rhs.getResult()));
            else
                _state = ::new (_stateBuffer.errSpace) ErrorState(std::move(rhs.getError()));

            rhs.assign(nullptr, e);
        }

        return (*this);
    }


    void assign(const V& v) {
        clear();

        _state = ::new (_stateBuffer.okSpace) OkState(v);
    }

    void assign(V&& v) {
        clear();

        _state = ::new (_stateBuffer.okSpace) OkState(std::move(v));
    }

    void assign(const V*, const E& v) {
        clear();

        _state = ::new (_stateBuffer.errSpace) ErrorState(v);
    }

    void assign(const V*, E && v) {
        clear();

        _state = ::new (_stateBuffer.errSpace) ErrorState(std::move(v));
    }

    void clear() {
        if (_state) {
            _state->~IState();
            _state = nullptr;
        }
    }

    Result& operator= (Result&& rhs) noexcept {
        return swap(rhs);
    }

    Result& operator= (const Result& rhs) noexcept = delete;

    explicit operator bool () const noexcept {
        return isOk();
    }

    bool isOk() const noexcept {
        return _state->isOk();
    }

    bool isError() const noexcept {
        return _state->isError();
    }

    V& getResult() {
        return _state->getResult();
    }

    const V& getResult() const {
        return _state->getResult();
    }

    const E& getError() const {
        return _state->getError();
    }

    E& getError() {
        return _state->getError();
    }

private:

    template<typename T>
    class AlignedStorage {
    private:
        union dummy_u {
            char data[ sizeof(T) ];
        } _dummy;

      public:
        void const* address() const { return _dummy.data; }
        void      * address()       { return _dummy.data; }

        T const* ptr_ref() const { return static_cast<T const*>(address()); }
        T *      ptr_ref()       { return static_cast<T *>     (address()); }
        T const& ref() const { return *ptr_ref(); }
        T &      ref()       { return *ptr_ref(); }
    };

private:

    class IState {
    public:
        virtual ~IState() = default;

        virtual bool isOk() const = 0;
        virtual bool isError() const = 0;

        virtual const V& getResult() const = 0;
        virtual V&       getResult()       = 0;

        virtual const E& getError() const = 0;
        virtual E&       getError()       = 0;
    };


    class OkState: public IState {
    public:
        OkState(const types::Ok<V>& val) {
            ::new (_storage.address()) V(val.val_);
        }

        OkState(types::Ok<V>&& val) {
            ::new (_storage.address()) V(std::move(val.val_));
        }

        OkState(V&& val) {
            ::new (_storage.address()) V(std::move(val));
        }

        OkState(const V& val) {
            ::new (_storage.address()) V(val);
        }

        ~OkState() {
            _storage.ref().V::~V();
        }

        bool isOk() const override { return true; }
        bool isError() const override { return false; }

        const V& getResult() const override   { return _storage.ref(); }
        V& getResult() override               { return _storage.ref(); }

        const E& getError() const override   { raiseInvalidStateError(); return *reinterpret_cast<E*>(NULL); }
        E& getError() override               { raiseInvalidStateError(); return *reinterpret_cast<E*>(NULL); }

    private:

        AlignedStorage<V> _storage;
    };


    class ErrorState: public IState {
    public:

        ErrorState(const types::Err<E>& val) {
            ::new (_storage.address()) E(val.val_);
        }

        ErrorState(types::Err<E>&& val) {
            ::new (_storage.address()) E(std::move(val.val_));
        }

        ~ErrorState() {
            _storage.ref().E::~E();
        }

        bool isOk() const override { return false; }
        bool isError() const override { return true; }

        const V& getResult() const override   { raiseInvalidStateError(); return *reinterpret_cast<V*>(NULL); }
        V& getResult() override               { raiseInvalidStateError(); return *reinterpret_cast<V*>(NULL); }

        const E& getError() const override   { return _storage.ref(); }
        E& getError() override               { return _storage.ref(); }

    private:

        AlignedStorage<E> _storage;
    };

    /**
     * Well, honestly it should have been called Schrodinger's Cat State 0_0
     */
    union SchrodingerState {
        byte okSpace[ sizeof(OkState) ];
        byte errSpace[ sizeof(ErrorState) ];
    } _stateBuffer;

    IState* _state;
};

}  // End of namespace Solace
#endif  // SOLACE_RESULT_HPP
