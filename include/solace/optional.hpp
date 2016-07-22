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
 * libSolace: Primitive type to represent optional value
 *	@file		solace/optional.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_OPTIONAL_HPP
#define SOLACE_OPTIONAL_HPP


#include "solace/icomparable.hpp"
#include "solace/exception.hpp"

#include <functional>   // std::function
#include <ostream>


namespace Solace {

void raiseInvalidStateError();

/** Optional monad
 * Optional is a monad that represent a optionality of value returned by a function.
    Parameterized type: Optional<T>
    unit: Optional.of()
    bind: Optional.flatMap()
 */
template<typename T>
class Optional: public IComparable<Optional<T>> {
public:

    static Optional<T> none() {
        return Optional<T>();
    }

    static Optional<T> of(const T& t) {
        return Optional<T>(t);
    }

    static Optional<T> of(T&& t) {
        return Optional<T>(t);
    }

public:

    Optional() {
        _state = ::new (_stateBuffer.noneSpace) NoneState();
    }

    Optional(T&& t) {
        _state = ::new (_stateBuffer.someSpace) SomeState(std::move(t));
    }

    Optional(Optional<T>&& that) : Optional() {
        swap(that);
    }

    Optional(const Optional<T>& that) {
        if (that.isSome())
            _state = ::new (_stateBuffer.someSpace) SomeState(that.get());
        else
            _state = ::new (_stateBuffer.noneSpace) NoneState();
    }

    virtual ~Optional() noexcept {
        _state->~IState();
    }

    Optional<T>& swap(Optional<T>& rhs) noexcept {

        if (isNone() && rhs.isNone()) {
            return *this;
        } else if (isSome()) {  // This has something inside:
            if (rhs.isNone()) {
                rhs.makeSome(get());
                makeNone();
            } else {
                T c(get());

                makeSome(rhs.get());
                rhs.makeSome(c);
            }
        } else {  // Rhs has something inside:
            if (isNone()) {
                makeSome(rhs.get());
                rhs.makeNone();
            } else {
                T c(get());

                makeSome(rhs.get());
                rhs.makeSome(c);
            }
        }

        return (*this);
    }

    Optional<T>& operator= (const Optional<T>& rhs) noexcept {
        Optional<T>(rhs).swap(*this);

        return *this;
    }

    Optional<T>& operator= (Optional<T>&& rhs) noexcept {
        return swap(rhs);
    }

    bool equals(const Optional<T>& other) const noexcept override {

        if (&other == this) {
            return true;
        }

        if (isNone() && other.isNone())
            return true;

        if (isSome() && other.isSome())
            return get() == other.get();

        return false;
    }

    bool isSome() const noexcept { return _state->isSome(); }

    bool isNone() const noexcept { return _state->isNone(); }

    const T& get() const {
        return _state->ref();
    }

    const T& orElse(const T& t) const noexcept {
        return _state->orElse(t);
    }

    template <typename U>
    Optional<U> map(const std::function<U(const T&)>& f) const {
        return (isSome()) ? Optional<U>::of(f(_state->ref())) : Optional<U>::none();
    }

    template <typename U>
    Optional<U> flatMap(const std::function<Optional<U> (const T&)>& f) const {
        return (isSome()) ? f(_state->ref()) : Optional<U>::none();
    }

protected:

    Optional(const T& t) {
        _state = ::new (_stateBuffer.someSpace) SomeState(t);
    }

private:

    template<typename V>
    class AlignedStorage {
    private:
        union dummy_u {
            char data[ sizeof(V) ];
        } _dummy;

      public:
        void const* address() const { return _dummy.data; }
        void      * address()       { return _dummy.data; }

        V const* ptr_ref() const { return static_cast<V const*>(address()); }
        V *      ptr_ref()       { return static_cast<V *>     (address()); }
        V const& ref() const { return *ptr_ref(); }
        V &      ref()       { return *ptr_ref(); }
    };

    class IState {
    public:
        virtual ~IState() = default;

        virtual bool isSome() const = 0;
        virtual bool isNone() const = 0;

        virtual const T& orElse(const T& t) const = 0;

        virtual const T& ref() const = 0;
        virtual T&       ref()       = 0;
    };

    class NoneState: public IState {
    public:
        bool isSome() const override { return false; }
        bool isNone() const override { return true; }

        const T& orElse(const T& t) const override { return t; }

        const T& ref() const override   { raiseInvalidStateError(); return *reinterpret_cast<T*>(NULL); }
        T& ref() override               { raiseInvalidStateError(); return *reinterpret_cast<T*>(NULL); }
    };

    class SomeState: public IState {
    public:

        SomeState(const T& val) {
            ::new (_storage.address()) T(val);
        }

        SomeState(T&& val) {
            ::new (_storage.address()) T(std::move(val));
        }

        ~SomeState() {
            _storage.ref().T::~T();
        }

        bool isSome() const override { return true; }
        bool isNone() const override { return false; }

        const T& orElse(const T&) const override { return ref(); }

        T const& ref() const override   { return _storage.ref(); }
        T& ref() override               { return _storage.ref(); }

    private:

        AlignedStorage<T> _storage;
    };


    void makeNone() {
        _state->~IState();
        _state = ::new (_stateBuffer.noneSpace) NoneState();
    }

    void makeSome(const T& t) {
        _state->~IState();
        _state = ::new (_stateBuffer.someSpace) SomeState(t);
    }

    void makeSome(T&& t) {
        _state->~IState();
        _state = ::new (_stateBuffer.someSpace) SomeState(std::move(t));
    }


private:


    /**
     * Well, honestly it should have been called Schrodinger Cat's State 0_0
     */
    union SchrodingerState {
        char noneSpace[ sizeof(NoneState) ];
        char someSpace[ sizeof(SomeState) ];
    } _stateBuffer;

    IState* _state;
};



/** "None" is a syntactic sugar for options
 * Noe type can be converted to any Optional<T>::none()
 */
class None {
public:

    template <typename T>
    operator Optional<T> () const {
        return Optional<T>::none();
    }

};


}  // namespace Solace

// TODO(abbyssoul): Should be in a separate file, if at all
template <typename T>
std::ostream& operator<< (std::ostream& ostr, const Solace::Optional<T>& anOptional) {
    return (anOptional.isNone())
            ? ostr.write("None", 4)
            : ostr << (const T&)anOptional.get();
}

#endif  // SOLACE_OPTIONAL_HPP
