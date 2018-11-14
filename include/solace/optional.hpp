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
#include "solace/traits/callable.hpp"     // type-traits: isCallable<>
#include "solace/assert.hpp"

#include <new>  // operator new

// Note: with C++17 can use std::optional
// #include <optional>


namespace Solace {

struct InPlace {
    explicit InPlace() = default;
};

/*inline*/ constexpr InPlace in_place{};

/**
 * A type to represent empty option of any type.
 */
struct None {
    // Used for constructing nullopt.
    enum class _Construct { _Token };

    // Must be constexpr for nullopt_t to be literal.
    explicit constexpr None(_Construct) { }
};


/// Tag to disengage optional objects.
/*inline*/ constexpr None none { None::_Construct::_Token };


// Forward declaration for type-trait
template<typename T>
class Optional;

/// Optional type-trait
template <typename MaybeOptional>
struct is_optional: std::false_type {
    using std::false_type::value;
};

template <typename T>
struct is_optional<Optional<T>>: std::true_type {
    using std::true_type::value;

    using value_type = T;
};


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
    using value_type = T;
    using Stored_type = std::remove_const_t<value_type>;

public:

    ~Optional() {
        destroy();
    }


    /**
     * Construct an new empty optional value.
     */
    constexpr Optional() noexcept
        : _empty{}
    {}

    constexpr Optional(None) noexcept
        : _empty{}
    {}


    Optional(Optional<T>&& other) noexcept(std::is_nothrow_move_constructible<T>::value) :
        _engaged(other.isSome()
                 ? construct(std::move(other._payload))
                 : false)
    {
    }

    Optional(Optional<T> const& other) noexcept(std::is_nothrow_copy_constructible<T>::value) :
        _engaged(other.isSome()
                 ? construct(other._payload)
                 : false)
    {
    }

    template<typename D>
    Optional(Optional<D>&& other) noexcept(std::is_nothrow_move_constructible<T>::value) :
        _engaged(other.isSome()
                 ? construct(std::move(other._payload))
                 : false)
    {
    }


    /**
     * Construct an non-empty optional value by copying-value.
     */
//    template<typename D,
//             typename unused = std::enable_if_t<std::is_copy_assignable<T>::value, void>
//             >
    Optional(T const& t) noexcept(std::is_nothrow_copy_constructible<T>::value) :
        _payload{t},
        _engaged{true}
    {}


    /**
     * Construct an non-empty optional value moving value.
     */
    Optional(T&& t) noexcept(std::is_nothrow_move_constructible<T>::value) :
        _payload{std::move(t)},
        _engaged{true}
    {}

    /**
     * Construct an non-empty optional in place.
     */
    template<typename ...ARGS>
    explicit Optional(InPlace, ARGS&&... args) noexcept(std::is_nothrow_move_constructible<T>::value)
        : _payload{std::forward<ARGS>(args)...}
        , _engaged{true}
    {}


    Optional<T>& swap(Optional<T>& rhs) noexcept {
        using std::swap;

        if (isSome()) {  // This has something inside:
            if (rhs.isNone()) {
                rhs.construct(std::move(_payload));
                destroy();
            } else {
                swap(rhs._payload, _payload);
            }
        } else {  // We got nothing:
            if (rhs.isNone()) {
                return *this;
            }

            construct(std::move(rhs._payload));
            rhs.destroy();
        }

        return (*this);
    }


    Optional<T>& operator= (None) noexcept {
        destroy();

        return *this;
    }


    Optional<T>& operator= (Optional<T>&& rhs) noexcept(std::is_nothrow_move_constructible<T>::value) {
        return swap(rhs);
    }

    Optional<T>& operator= (T&& rhs) noexcept(std::is_nothrow_move_constructible<T>::value) {
        destroy();
        construct(std::move(rhs));

        return *this;
    }

    constexpr explicit operator bool() const noexcept {
      return isSome();
    }

    constexpr bool isSome() const noexcept { return _engaged; }

    constexpr bool isNone() const noexcept { return !_engaged; }

    T& operator* () noexcept { return get(); }
    T const& operator* () const noexcept { return get(); }

    T const& get() const {
        if (isNone()) {
            raiseInvalidStateError();
        }

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

    T const& orElse(T const& t) const noexcept {
        if (isNone()) {
            return t;
        }

        return _payload;
    }

    template <typename F,
              typename U = typename std::result_of<F(T&)>::type>
    Optional<U> map(F&& f) {
        return (isSome())
                ? Optional<U>(f(_payload))
                : none;
    }

    template <typename F,
              typename U = typename std::result_of<F(T)>::type>
    Optional<U> map(F&& f) const {
        return (isSome())
                ? Optional<U>(f(_payload))
                : none;
    }


    template <typename F,
              typename U = typename std::result_of<F(T)>::type>
    std::enable_if_t<is_optional<U>::value && (isCallable<F, T>::value || isCallable<F, const T&>::value),
    U >
    flatMap(F&& f) const {
        return (isSome())
                ? f(_payload)
                : none;
    }


    template <typename F,
              typename U = typename std::result_of<F(T)>::type>
    std::enable_if_t<is_optional<U>::value && (isCallable<F, T&&>::value || isCallable<F, T>::value),
    U >
    flatMap(F&& f) && {
        return (isSome())
                ? f(std::move(_payload))
                : none;
    }


    template <typename F>
    const Optional<T>& filter(F&& predicate) const {
        return (isSome())
                ? (predicate(_payload) ? *this : _emptyInstance)
                : _emptyInstance;
    }

protected:


    template<typename...Args>
    constexpr bool
    construct(Args&&... args)
    noexcept(std::is_nothrow_constructible<Stored_type, Args...>()) {
        :: new (static_cast<void *>(std::addressof(_payload))) Stored_type(std::forward<Args>(args)...);

        _engaged = true;

        return _engaged;
    }

    constexpr void destroy() {
        if (_engaged) {
            _engaged = false;
            _payload.~Stored_type();
        }
    }

private:

    static Optional<T> _emptyInstance;

    template <class>
    friend class Optional;

    struct  Empty_type {};

    union {
        Empty_type  _empty;
        Stored_type _payload;
    };

    bool _engaged {false};
};


template <typename T>
Optional<T> Optional<T>::_emptyInstance {none};


template<typename T>
bool operator== (Optional<T> const& a, None) {
    return a.isNone();
}

template<typename T>
bool operator== (None, Optional<T> const& a) {
    return a.isNone();
}


template<typename T>
bool operator== (Optional<T> const& a, Optional<T> const& b) {
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
#endif  // SOLACE_OPTIONAL_HPP
