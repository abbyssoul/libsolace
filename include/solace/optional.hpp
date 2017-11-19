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
#include <type_traits>  // std::aligned_storage

// Note: with C++17 can use std::optional
// #include <optional>


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
// cppcheck-suppress copyCtorAndEqOperator
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
        return Optional<T>(std::move(t));
    }

public:

    ~Optional() {
        destroy();
    }


    /**
     * Construct an new empty optional value.
     */
    Optional() :
        _empty(),
        _engaged(false)
    {}


    Optional(Optional<T>&& other) noexcept(std::is_nothrow_move_constructible<T>::value) :
        _empty(),
        _engaged(false)
    {
        if (other.isSome()) {
            construct(std::move(other._payload));
        }
    }

    template<typename D>
    Optional(Optional<D>&& other) noexcept(std::is_nothrow_move_constructible<T>::value) :
        _empty(),
        _engaged(false)
    {
        if (other.isSome()) {
            construct(std::move(other._payload));
        }
    }


    Optional<T>& swap(Optional<T>& rhs) noexcept {
        using std::swap;

        // TODO(abbyssoul): can we refactor this ugliness?
        if (isNone() && rhs.isNone()) {
            return *this;
        }

        if (isSome()) {  // This has something inside:
            if (rhs.isNone()) {
                rhs.construct(std::move(_payload));
                destroy();
            } else {
                swap(rhs._payload, _payload);
            }
        } else {  // We got nothing:
            if (rhs.isNone()) {
                // Logic error!
                raiseInvalidStateError("logic error");
            } else {
                construct(std::move(rhs._payload));
                rhs.destroy();
            }
        }

        return (*this);
    }

    Optional<T>& operator= (Optional<T>&& rhs) noexcept {
        return swap(rhs);
    }

    Optional<T>& operator= (T&& rhs) noexcept {
        destroy();
        construct(std::move(rhs));

        return *this;
    }

    constexpr explicit operator bool() const noexcept {
      return isSome();
    }

    constexpr bool isSome() const noexcept { return _engaged; }

    constexpr bool isNone() const noexcept { return !_engaged; }

    const T& get() const {
        if (isNone())
            raiseInvalidStateError();

        return _payload;
    }

    T& get() {
        if (isNone()) {
            raiseInvalidStateError();
        }

        return _payload;
    }


    T&& move() {
        if (isNone()) {
            raiseInvalidStateError();
        }

        return std::move(_payload);
    }

    const T& orElse(const T& t) const noexcept {
        if (isNone()) {
            return t;
        }

        return _payload;
    }

    template <typename F,
              typename U = typename std::result_of<F(T&)>::type>
    Optional<U> map(F&& f) {
        return (isSome())
                ? Optional<U>::of(f(_payload))
                : Optional<U>::none();
    }

    template <typename F,
              typename U = typename std::result_of<F(T)>::type>
    Optional<U> map(F&& f) const {
        return (isSome())
                ? Optional<U>::of(f(_payload))
                : Optional<U>::none();
    }


    template <typename U>
    Optional<U> flatMap(const std::function<Optional<U> (const T&)>& f) const {
        return (isSome())
                ? f(_payload)
                : Optional<U>::none();
    }

    template <typename F>
    const Optional<T>& filter(F&& predicate) const {
        return (isSome())
                ? (predicate(_payload) ? *this : _emptyInstance)
                : _emptyInstance;
    }

protected:

    /**
     * Construct an non-empty optional value by copying-value.
     */
    Optional(const T& t) noexcept(std::is_nothrow_copy_constructible<T>::value) :
        _payload(t),
        _engaged(true)
    {}


    /**
     * Construct an non-empty optional value moving value.
     */
    Optional(T&& t) noexcept(std::is_nothrow_move_constructible<T>::value) :
        _payload(std::move(t)),
        _engaged(true)
    {}


    void construct(const T& t) {
        if (_engaged)
            raiseInvalidStateError("logic error");

        ::new (reinterpret_cast<void *>(std::addressof(_payload))) Stored_type(t);

        _engaged = true;
    }

    void construct(T&& t) {
        if (_engaged)
            raiseInvalidStateError("logic error");

        ::new (reinterpret_cast<void *>(std::addressof(_payload))) Stored_type(std::move(t));
        _engaged = true;
    }

    void destroy() {
        if (_engaged) {
            _engaged = false;
            _payload.~Stored_type();
        } else {
            _empty.~Empty_type();
        }
    }

private:

    static Optional<T> _emptyInstance;

    template <class>
    friend class Optional;

    using   Stored_type = std::remove_const_t<T>;
    struct  Empty_type {};

    union {
        Empty_type  _empty;
        Stored_type _payload;
    };

    bool _engaged = false;
};


template <typename T>
Optional<T> Optional<T>::_emptyInstance = Optional<T>::none();


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
bool operator== (const Optional<T>& a, const Optional<T>& b) {
    if (&a == &b) {
        return true;
    }

    if (a.isNone() && b.isNone()) {
        return true;
    }

    if (a.isSome() && b.isSome()) {
        return (a.get() == b.get());
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
