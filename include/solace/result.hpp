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


/**
 * Result is function wrapping for function execution result and an alternative to exception throwing.
 *
 * Note: Result<void, Error> is technically equivalent of Optional<Error>
 */
template <typename V, typename E>
class Result {
public:
    typedef V value_type;
    typedef E error_type;

public:

    Result(const V& value):
        _state( ::new (_stateBuffer.okSpace) OkState(value) )
    {}

    Result(V&& value):
        _state( ::new (_stateBuffer.okSpace) OkState(std::move(value)) )
    {}

    Result(const V*, const E& err):
        _state( ::new (_stateBuffer.errSpace) ErrorState(err) )
    {}

    Result(const V*, E && err):
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
        OkState(const V& val) {
            ::new (_storage.address()) V(val);
        }

        OkState(V&& val) {
            ::new (_storage.address()) V(std::move(val));
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

        ErrorState(const E& val) {
            ::new (_storage.address()) E(val);
        }

        ErrorState(E&& val) {
            ::new (_storage.address()) E(std::move(val));
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


template <typename V,
          typename E,
//          typename ValueT = std::decay<V>::type,
          typename ValueType = typename std::remove_reference<V>::type,
          typename ErrorType = typename std::remove_reference<E>::type
          >
Result<ValueType, ErrorType> Ok(V&& value) {
    return Result<ValueType, ErrorType>(std::forward<V>(value));
}

template <typename V,
          typename E,
          typename ValueType = typename std::remove_reference<V>::type,
          typename ErrorType = typename std::remove_reference<E>::type
          >
Result<ValueType, ErrorType> Err(E&& value) {
    return Result<ValueType, ErrorType>(nullptr, std::forward<E>(value));
}


}  // End of namespace Solace
#endif  // SOLACE_RESULT_HPP

