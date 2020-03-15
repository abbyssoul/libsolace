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
 ******************************************************************************/
#pragma once
#ifndef SOLACE_OPTIONAL_HPP
#define SOLACE_OPTIONAL_HPP

#include "solace/types.hpp"
#include "solace/traits/callable.hpp"     // type-traits: isCallable<>
#include "solace/utils.hpp"
#include "solace/assert.hpp"


#include <memory>  // std::addressof


namespace Solace {

struct InPlace {
    explicit InPlace() = default;
};

inline constexpr InPlace in_place{};

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
inline constexpr None none { None::_Construct::_Token };


// Forward declaration for type-trait
template<typename T>
class Optional;

/// Optional type-trait
template <typename MaybeOptional>
struct is_optional : std::false_type {
    using std::false_type::value;
};

template <typename T>
struct is_optional<Optional<T>> : std::true_type {
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
template<typename T>
class Optional {
public:
    using value_type = T;

public:

    inline ~Optional() noexcept(std::is_nothrow_destructible_v<T>) {
        destroy();
    }

    /**
     * Construct an new empty optional value.
     */
    constexpr Optional() noexcept
		: _engaged{false}
		, _empty{}
    {}

    constexpr Optional(None) noexcept
		: _engaged{false}
		, _empty{}
	{}

	template<typename D>
	constexpr Optional(Optional<D>& other) noexcept(std::is_nothrow_copy_constructible<T>::value)
		: _engaged{other.isSome()
				   ? construct(other._payload)
				   : false}
	{
	}

	template<typename D>
	constexpr Optional(Optional<D> const& other) noexcept(std::is_nothrow_copy_constructible<T>::value)
		: _engaged{other.isSome()
				   ? construct(other._payload)
				   : false}
	{
	}

	template<typename D>
	constexpr Optional(Optional<D>&& other) noexcept(std::is_nothrow_move_constructible<T>::value)
		: _engaged{other.isSome()
				   ? construct(mv(other._payload))
				   : false}
    {
    }

	template<typename CanBeT>
	constexpr Optional(CanBeT&& value) noexcept(std::is_nothrow_move_constructible<T>::value)
		: _engaged{construct(fwd<CanBeT>(value))}
	{}

    /**
     * Construct an non-empty optional in place.
     */
    template<typename ...ARGS>
    explicit Optional(InPlace, ARGS&&... args) noexcept(std::is_nothrow_move_constructible<T>::value)
		: _engaged{true}
		, _payload{fwd<ARGS>(args)...}
    {}


    Optional<T>& swap(Optional<T>& rhs) noexcept {
        using std::swap;

		auto const rhsSome = rhs.isSome();
        if (isSome()) {  // This has something inside:
			if (rhsSome) {
				swap(rhs._payload, _payload);
            } else {
				rhs.construct(mv(_payload));
				destroy();
			}
        } else {  // We got nothing:
			if (rhsSome) {
				construct(mv(rhs._payload));
				rhs.destroy();
			}
			// Note: this.isNone() and rhs.isNone - no-op
        }

        return (*this);
    }


    Optional<T>& operator= (None) noexcept {
        destroy();

        return *this;
    }


    Optional<T>& operator= (Optional<T>&& rhs) noexcept(std::is_nothrow_move_constructible<T>::value) {
		swap(rhs);

		return (*this);
    }

    Optional<T>& operator= (T&& rhs) noexcept(std::is_nothrow_move_constructible<T>::value) {
        destroy();
		construct(mv(rhs));

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
			raiseInvalidStateError("Optional<>::get");
        }

        return _payload;
    }

    T& get() {
        if (isNone()) {
			raiseInvalidStateError("Optional<>::get");
        }

        return _payload;
    }


    T&& move() {
        if (isNone()) {
			raiseInvalidStateError("Optional<>::move");
        }

		return mv(_payload);
    }

    T const& orElse(T const& t) const noexcept {
        if (isNone()) {
            return t;
        }

        return _payload;
    }

	template <typename F,
			  typename U = typename std::invoke_result<F, T&>::type>
	Optional<U>	map(F&& f) {
        return (isSome())
				? Optional<U>{f(_payload)}
                : none;
    }

    template <typename F,
			  typename U = typename std::invoke_result<F, T>::type>
	Optional<U>	map(F&& f) const {
        return (isSome())
				? Optional<U>{f(_payload)}
                : none;
    }


    template <typename F,
			  typename U = typename std::invoke_result<F, T>::type>
	std::enable_if_t<is_optional<U>::value && (isCallable<F, T>::value || isCallable<F, T const&>::value),
    U >
	flatMap(F&& f) const& {
        return (isSome())
                ? f(_payload)
                : none;
    }


    template <typename F,
			  typename U = typename std::invoke_result<F, T&&>::type>
    std::enable_if_t<is_optional<U>::value && (isCallable<F, T&&>::value || isCallable<F, T>::value),
    U >
    flatMap(F&& f) && {
        return (isSome())
				? f(mv(_payload))
                : none;
    }

	 template <typename F,
			   typename U = typename std::invoke_result<F, T&&>::type>
	 std::enable_if_t<is_optional<U>::value && (isCallable<F, T&&>::value || isCallable<F, T>::value),
	 U >
	 flatMap(F&& f) & {
		 return (isSome())
				 ? f(mv(_payload))
				 : none;
	 }

    template <typename F>
	Optional<T> const& filter(F&& predicate) const {
		return (isSome() && predicate(_payload))
				? *this
				: _kEmptyInstance;
    }

protected:
	using mutable_value = std::remove_cv_t<value_type>;
	using StoredValue_type = std::conditional_t<std::is_reference_v<value_type>,
												std::reference_wrapper<std::decay_t<value_type>>,
												mutable_value>;


    template<typename...Args>
    constexpr bool
	construct(Args&&... args) noexcept(std::is_nothrow_constructible<StoredValue_type, Args...>()) {
		ctor(_payload, fwd<Args>(args)...);
        _engaged = true;

        return _engaged;
    }

	constexpr void
	destroy() noexcept(std::is_nothrow_destructible_v<StoredValue_type>) {
        if (_engaged) {
            _engaged = false;
			dtor(_payload);
        }
    }

private:

	static Optional<T> _kEmptyInstance;

    template <class>
    friend class Optional;

	struct Empty_type {};

	bool _engaged;
	union {
		Empty_type			_empty;
		StoredValue_type	_payload;
    };

};


template <typename T>
inline Optional<T> Optional<T>::_kEmptyInstance{};

constexpr bool operator== (None, None) noexcept { return true; }
constexpr bool operator!= (None, None) noexcept { return false; }

template<typename T>
bool operator== (Optional<T> const& a, None) noexcept { return a.isNone(); }

template<typename T>
bool operator== (None, Optional<T> const& a) noexcept { return a.isNone(); }

template<typename T>
bool operator!= (Optional<T> const& a, None) noexcept { return a.isSome(); }

template<typename T>
bool operator!= (None, Optional<T> const& a) noexcept { return a.isSome(); }

template<typename T>
bool operator== (Optional<T> const& a, Optional<T> const& b) noexcept {
    if (&a == &b) {
        return true;
    }

	return (a.isSome() && b.isSome())
			? (a.get() == b.get())
			: (a.isNone() && b.isNone());
}

template<typename T>
bool operator!= (Optional<T> const& a, Optional<T> const& b) {
    if (&a == &b) {
        return false;
    }

	return (a.isSome() && b.isSome())
			? (*a != *b)
			: !(a.isNone() && b.isNone());
}


template<typename T>
bool operator== (Optional<T> const& a, T const& value) { return a.isSome() && (*a == value); }

template<typename T>
bool operator== (T const& value, Optional<T> const& a) { return a.isSome() && (*a == value); }

template<typename T>
bool operator!= (Optional<T> const& a, T const& value) { return a.isNone() || (*a != value); }

template<typename T>
bool operator!= (T const& value, Optional<T> const& a) { return a.isNone() || (*a != value); }


template<typename T>
T operator|| (Optional<T>& maybaValue, T& value) { return maybaValue.orElse(value); }

template<typename T>
const T operator|| (Optional<T> const& maybaValue, T const& value) { return maybaValue.orElse(value); }


template<typename T>
void swap(Optional<T>& lhs, Optional<T>& rhs) noexcept {
    lhs.swap(rhs);
}


}  // namespace Solace
#endif  // SOLACE_OPTIONAL_HPP
