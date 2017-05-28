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

#include "solace/types.hpp"
#include "solace/traits/icomparable.hpp"
#include "solace/assert.hpp"


#include <functional>   // std::function
#include <ostream>


namespace Solace {


/** Optional monad
 * One can think of optional as a list that contains at most 1 item but can be empty as well.
 * This concept allows for a better expression of situation when value might not be present.
 * That it why using Optional should be prefered to returning null.
 *
 * Optional is a monad that represent a optionality of value returned by a function.
    Parameterized type: Optional<T>
    unit: Optional.of()
    bind: Optional.flatMap()
 */
template<typename T>
class Optional {
public:
    typedef T value_type;

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


    /**
     * Construct an new empty optional value.
     */
    Optional() : _state(::new (_stateBuffer.noneSpace) NoneState())
    {}

    /**
     * Construct an non-empty optional value moving value.
     */
    Optional(T&& t) noexcept(std::is_nothrow_copy_constructible<T>::value)
        : _state(::new (_stateBuffer.someSpace) SomeState(std::move(t)))
    {}

    Optional(Optional<T>&& that)
        noexcept(std::is_nothrow_copy_constructible<T>::value) : Optional() {

        swap(that);
    }

    Optional(const Optional<T>& that) noexcept(std::is_nothrow_copy_constructible<T>::value) {
        if (that.isSome())
            _state = ::new (_stateBuffer.someSpace) SomeState(that.get());
        else
            _state = ::new (_stateBuffer.noneSpace) NoneState();
    }

    virtual ~Optional() noexcept {
        _state->~IState();
    }

    Optional<T>& swap(Optional<T>& rhs) noexcept {
        // TODO(abbyssoul): can we refactor this ugliness?
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

    explicit operator bool() const noexcept {
      return isSome();
    }

    bool isSome() const noexcept { return _state->isSome(); }

    bool isNone() const noexcept { return _state->isNone(); }

    const T& get() const {
        return _state->ref();
    }

    T& get() {
        return _state->ref();
    }

    const T& orElse(const T& t) const noexcept {
        return _state->orElse(t);
    }

    template <typename F,
              typename U = typename std::result_of<F(T)>::type>
    Optional<U> map(F f) const {
        return (isSome()) ? Optional<U>::of(f(_state->ref())) : Optional<U>::none();
    }

    template <typename U>
    Optional<U> flatMap(const std::function<Optional<U> (const T&)>& f) const {
        return (isSome()) ? f(_state->ref()) : Optional<U>::none();
    }

    template <typename F>
    Optional<T> filter(F predicate) const {
        return (isSome())
                ? (predicate(_state->ref()) ? *this : Optional<T>::none())
                : Optional<T>::none();
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
            byte data[ sizeof(V) ];
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

        virtual bool isSome() const noexcept = 0;
        virtual bool isNone() const noexcept = 0;

        virtual const T& orElse(const T& t) const = 0;

        virtual const T& ref() const = 0;
        virtual T&       ref()       = 0;
    };

    class NoneState: public IState {
    public:
        bool isSome() const noexcept override { return false; }
        bool isNone() const noexcept override { return true; }

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

        bool isSome() const noexcept override { return true; }
        bool isNone() const noexcept override { return false; }

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
        byte noneSpace[ sizeof(NoneState) ];
        byte someSpace[ sizeof(SomeState) ];
    } _stateBuffer;

    IState* _state;
};



/** "None" is a syntactic sugar for options
 * None type can be converted to any Optional<T>::none()
 */
class None {
public:

    template <typename T>
    operator Optional<T> () const {
        return Optional<T>::none();
    }

};


template<typename T>
bool operator == (const Optional<T>& a, const Optional<T>& b) {
    if (&a == &b) {
        return true;
    }

    if (a.isNone() && b.isNone()) {
        return true;
    }

    if (a.isSome() && b.isSome()) {
        return a.get() == b.get();
    }

    return false;
}

template<typename T>
void swap(Optional<T>& lhs, Optional<T>& rhs) noexcept {
    lhs.swap(rhs);
}


}  // namespace Solace

// TODO(abbyssoul): Should be in a separate file, if at all
template <typename T>
std::ostream& operator<< (std::ostream& ostr, const Solace::Optional<T>& anOptional) {
    return (anOptional.isNone())
            ? ostr.write("None", 4)
            : ostr << anOptional.get();
}

#endif  // SOLACE_OPTIONAL_HPP
