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
 ******************************************************************************/
#pragma once
#ifndef SOLACE_RESULT_HPP
#define SOLACE_RESULT_HPP

#include "solace/types.hpp"
#include "solace/assert.hpp"
#include "solace/optional.hpp"


namespace Solace {

// Implementation types
namespace types {

struct OkTag {};
struct ErrTag {};

inline constexpr OkTag okTag;
inline constexpr ErrTag errTag;


template<typename T, typename Tag>
struct ValueWrapper {
    using value_type = T;

	explicit constexpr ValueWrapper(value_type const& val) noexcept(std::is_nothrow_copy_constructible<value_type>::value)
        : val_{val}
	{ }

	explicit constexpr ValueWrapper(value_type&& val) noexcept(std::is_nothrow_move_constructible<value_type>::value)
        : val_{mv(val)}
	{ }

	value_type val_;
};

template<>
struct ValueWrapper<void, OkTag> {
	using value_type = void;
};


template<typename T>
using Ok = ValueWrapper<T, OkTag>;

template<typename T>
using Err = ValueWrapper<T, ErrTag>;

}  // End of namespace types


/**
 * Syntactic sugar to return successful Result
 * Note: this function is a producer / constructor of type T
 *
 * @return type::Ok<T> that can be converted into a successful Result<T, E>
 */
template<typename T,
         typename CleanT = typename std::decay<T>::type >
inline types::Ok<CleanT> Ok(T&& val) {
	return types::Ok<CleanT>{fwd<T>(val)};
}

/**
 * A specialisation of Ok helper function to construct successful Result<Void, E>
 * @return type::Ok<T> that can be converted into a successful Result<T, E>
 */
constexpr None Ok() noexcept {
    return none;
}


/**
 * Syntactic sugar to produce error Result<T, E>
 * Note: this function is a producer / constructor of type E
 *
 * @return type::Err<T> that can be converted into a failed Result<T, E>
 */
template<typename E,
         typename CleanE = typename std::decay<E>::type >
inline types::Err<CleanE> Err(E&& val) {
	return types::Err<CleanE>{fwd<E>(val)};
}


// Forward declaration of Result<> type to create template helpers
template <typename V, typename E>
class Result;


template <typename TV, typename TE, typename T>
struct isResult : std::false_type {
    using std::false_type::value;

    using value_type = T;
};



template <typename T>
struct isSomeResult : std::false_type {
    using std::false_type::value;

    using value_type = T;
};

template <typename V, typename E>
struct isSomeResult<Result<V, E>> : std::true_type {
    using std::true_type::value;

    using type = Result<V, E>;

    using value_type = typename type::value_type;
    using error_type = typename type::error_type;
};


template <
        typename OV, typename OE,
        typename V, typename E>
struct isResult<OV, OE, Result<V, E>> : public std::true_type {
    using std::true_type::value;

    using type = Result<V, E>;

    using value_type = typename type::value_type;
    using error_type = typename type::error_type;
};


template <
        typename OV, typename OE,
        typename V>
struct isResult<OV, OE, types::Ok<V>> : public std::true_type {
    using std::true_type::value;

    using type = Result<V, OE>;

    using value_type = typename type::value_type;
    using error_type = typename type::error_type;
};

template <
        typename OV, typename OE,
        typename E>
struct isResult<OV, OE, types::Err<E>> : public std::true_type {
    using std::true_type::value;

    using type = Result<OV, E>;

    using value_type = typename type::value_type;
    using error_type = typename type::error_type;
};


template<typename E>
struct error_result_wrapper { using type = E; };

template<typename V, typename E>
struct error_result_wrapper<Result<V, E>> { using type = E; };

template<typename E>
struct error_result_wrapper<types::Err<E>> { using type = typename types::Err<E>::value_type; };


/**
 * Result class is a sum type of two values V and E.
 * Type V represents expected value produced by a computation while type E represnts error type produced in case
 * of an error to compute.
 * This concept is known as Either<> monad in some functional languages and
 * is inspired by std::Result<> class from Rust lang.
 *
 * Result is desinged to represent a syncronious computation result and in that sense it is always 'set'.
 * Please see Futute types provided by other libraries to represent asynchronious computation result.
 *
 * Note: A specialisation of Result<void, Error> is technically equivalent of Optional<Error>
 */
template <typename V, typename E>
class
// [[nodiscard]]
Result {
public:
	using value_type = V;
	using error_type = E;

	static_assert(!std::is_same<V, E>::value,
				  "Result types must be destinct");

	static_assert(!std::is_same<E, void>::value,
				  "Error type must be non-void");

public:

	inline ~Result() {
		destroy();
	}

    /** Copy construct of Result is disabled */
    Result(Result const& rhs) = delete;
    Result& operator= (Result const& rhs) noexcept = delete;

    /**
     * Move-Construct Result of the same type.
     * @param rhs Source to move values from
     */
	Result(Result&& rhs) noexcept(std::is_nothrow_move_constructible<V>::value &&
								  std::is_nothrow_move_constructible<E>::value)
		: _engaged(rhs.isOk() ? emplaceValue(mv(rhs._value)) : emplaceError(mv(rhs._error)))
	{}

	template<typename DV,
			 typename x = typename std::enable_if_t<std::is_constructible_v<V, DV>, DV>
			 >
	Result(Result<DV, E>&& rhs) noexcept
		: _engaged(rhs.isOk() ? emplaceValue(mv(rhs._value)) : emplaceError(mv(rhs._error)))
	{
    }

	Result& operator= (Result&& rhs) noexcept {
		return swap(rhs);
	}


    /**
     * Move-Construct Ok result
     * @param value Ok value to move from
     */
	template<typename DV,
			 typename x = typename std::enable_if_t<std::is_constructible_v<V, DV>, DV>
			 >
	constexpr Result(types::Ok<DV>&& value) noexcept(std::is_nothrow_move_constructible<V>::value)
		: Result{types::okTag, mv(value.val_)}
	{}


    /**
     * Move-Construct Err result by moving error value
     * @param err Err value to move from
     */
	template<typename DE,
			 typename x = typename std::enable_if_t<std::is_constructible_v<E, DE>, DE>
			 >
	constexpr Result(types::Err<DE>&& value) noexcept(std::is_nothrow_move_constructible<E>::value)
		: Result{types::errTag, mv(value.val_)}
	{}

    /**
     * Move-Construct Err result from a compatible error type by moving error value
     * @param err Err value to move from
     */
	template<typename VW, typename Tag>
	constexpr Result(types::ValueWrapper<VW, Tag>&& value)  // Can't say much about noexcept
		: Result{Tag{}, mv(value.val_)}
	{}


	template<typename CanBeV>
	constexpr Result(types::OkTag, CanBeV&& value) noexcept(std::is_nothrow_move_constructible<V>::value)
		:_engaged{emplaceValue(fwd<CanBeV>(value))}
    {}


	template<typename CanBeE>
	constexpr Result(types::ErrTag, CanBeE&& value) noexcept(std::is_nothrow_move_constructible<E>::value)
		:_engaged{emplaceError(fwd<CanBeE>(value))}
    {}


	template<typename...Args>
	constexpr Result(types::OkTag, InPlace, Args&&...args)
		: _engaged{emplaceValue(fwd<Args>(args)...)}
	{}

	template<typename...Args>
	constexpr Result(types::ErrTag, InPlace, Args&&...args)
		: _engaged{emplaceError(fwd<Args>(args)...)}
	{}

	constexpr Result(RefOrRValue<V> value) noexcept(std::is_nothrow_move_constructible<V>::value)
		: _engaged{emplaceValue(static_cast<RefOrRValue<V>>(value))}
	{}

	constexpr Result(RefOrRValue<E> value) noexcept(std::is_nothrow_move_constructible<E>::value)
		: _engaged{emplaceError(static_cast<RefOrRValue<E>>(value))}
	{}

public:

	Result& swap(Result& rhs) noexcept {
		using std::swap;

		if (isOk()) {
			StoredValue_type v{mv(_value)};
			if (rhs.isOk()) {
				constructValue(mv(rhs._value));
			} else {
				constructError(mv(rhs._error));
			}
			rhs.constructValue(mv(v));

		} else {
			StoredError_type e{mv(_error)};
			if (rhs.isOk()) {
				constructValue(mv(rhs._value));
			} else {
				constructError(mv(rhs._error));
			}
			rhs.constructError(mv(e));
		}

		return (*this);
	}

    explicit operator bool () const noexcept {
        return isOk();
    }

    constexpr bool isOk() const noexcept {
        return _engaged;
    }

    constexpr bool isError() const noexcept {
        return !_engaged;
    }

    V& operator* () { return unwrap(); }
    V const& operator* () const { return unwrap(); }

    V const& unwrap() const& {
        if (isError()) {
            raiseInvalidStateError();
        }

        return _value;
    }

    V& unwrap() & {
        if (isError()) {
            raiseInvalidStateError();
        }

        return _value;
    }

    V&& unwrap() && {
        if (isError()) {
            raiseInvalidStateError();
        }

        return mv(_value);
    }


    V&& moveResult() {
        if (isError()) {
            raiseInvalidStateError();
        }

        return mv(_value);
    }

    E&& moveError() {
        if (isOk()) {
            raiseInvalidStateError();
        }

        return mv(_error);
    }

    E& getError() {
        if (isOk()) {
            raiseInvalidStateError();
        }

        return _error;
    }

    E const& getError() const {
        if (isOk()) {
            raiseInvalidStateError();
        }

        return _error;
    }


    /**
     * Then combinator.
     * Calls 'f' on the Ok value if the result is Ok, otherwise returns the Err value of self.
     * This is an equivalent of flatMap for Optional value for 'f' returning Result<V, E>
     *
     * @param f callable object to call on the success value. It is only called if this::isOk() is true
     * @return Result<U, E> of the call of 'f' if this::isOk(), Err(this->getError()) otherwise
     */
    template<typename F,
             typename R = typename std::result_of<F(V)>::type,
             typename ResT = isResult<V, E, R>>
    std::enable_if_t<ResT::value, typename ResT::type>
    then(F&& f) && {
        if (isOk()) {
            return f(mv(_value));
        }

		return typename ResT::type{types::errTag, mv(_error)};
    }

    template<typename F,
             typename R = typename std::result_of<F(V)>::type,
             typename ResT = isResult<V, E, R>>
    std::enable_if_t<ResT::value, typename ResT::type>
    then(F&& f) & {
        if (isOk()) {
            return f(_value);
        }

		return typename ResT::type{types::errTag, _error};
    }

    template<typename F,
             typename R = typename std::result_of<F(V)>::type,
             typename ResT = isResult<V, E, R>>
    std::enable_if_t<ResT::value, typename ResT::type>
    then(F&& f) const& {
        if (isOk()) {
            return f(_value);
        }

		return typename ResT::type{types::errTag, _error};
    }


    template <typename F,
              typename R = typename std::result_of<F(V)>::type
              >
    std::enable_if_t<!std::is_same<R, void>::value && !isResult<V, E, R>::value,  Result<R, E>>
    then(F&& f) {
        if (isOk()) {
			return Result<R, E>{types::okTag, f(mv(_value))};
        }

		return Result<R, E>{types::errTag, mv(_error)};
    }


    template <typename F,
              typename R = typename std::result_of<F(V)>::type
              >
    std::enable_if_t<std::is_same<void, R>::value, Result<void, E>>
    then(F&& f) {
        if (isOk()) {
            // TODO(abbyssoul): Handle exeptions and convert them into error_type
            f(moveResult());

            return Result<void, E>{none};
        }

        return Result<void, E>{mv(_error)};
    }


    //------------------------------------------------------------------

    template<typename F,
             typename R = typename std::result_of<F(E)>::type,
             typename ResT = isResult<V, E, R>>
    std::enable_if_t<ResT::value, typename ResT::type>
    orElse(F&& f) {
        if (isOk()) {
			return typename ResT::type{types::okTag, moveResult()};
        }

        return f(moveError());
    }


    template<typename F,
             typename RE = typename std::result_of<F(E)>::type>
    std::enable_if_t<!isResult<V, E, RE>::value, Result<RE, E>>
    orElse(F&& f) {
        if (isOk()) {
			return Result<RE, E>{types::okTag, moveResult()};
        }

		return Result<RE, E>(types::okTag, f(moveError()));
    }

    /**
     * Pass through a Ok result but applies a given function to an error value.
     * This can be used to handle errors.
     *
     * @param f - An error mapping function to map Err value.
     */
    template<typename F,
             typename RE = typename std::result_of<F(E)>::type,
             typename EE = typename error_result_wrapper<RE>::type>
    Result<V, EE> mapError(F&& f) && {
        if (isOk()) {
			return Result<V, EE>{types::okTag, mv(_value)};
        }

		return Result<V, EE>{types::errTag, f(mv(_error))};
    }

    template<typename F,
             typename RE = typename std::result_of<F(E)>::type,
             typename EE = typename error_result_wrapper<RE>::type>
    Result<V, EE> mapError(F&& f) & {
        if (isOk()) {
			return Result<V, EE>{types::okTag, unwrap()};
        }

		return Result<V, EE>{types::errTag, f(getError())};
    }

    template<typename F,
             typename RE = typename std::result_of<F(E)>::type,
             typename EE = typename error_result_wrapper<RE>::type>
    Result<V, EE> mapError(F&& f) const& {
        if (isOk()) {
			return Result<V, EE>{types::okTag, unwrap()};
        }

		return Result<V, EE>{types::errTag, f(getError())};
    }

private:

    void destroy() {
        if (_engaged) {
			dtor(_value);
        } else {
			dtor(_error);
        }
    }

	template<typename ...XV>
	bool emplaceValue(XV&&...args) {
		ctor(_value, fwd<XV>(args)...);
		_engaged = true;

		return _engaged;
	}

	template<typename ...XE>
	bool emplaceError(XE&&...args) {
		ctor(_error, fwd<XE>(args)...);
		_engaged = false;

		return _engaged;
	}

	template<typename XV>
	void constructValue(XV&& t) {
        destroy();
		emplaceValue(fwd<XV>(t));
    }

	template<typename XE>
	void constructError(XE&& t) {
		destroy();
		emplaceError(fwd<XE>(t));
	}

private:

    template<typename DV, typename DE>
    friend class Result;

	using mutable_value = std::remove_cv_t<value_type>;
	using StoredValue_type = std::conditional_t<std::is_reference_v<value_type>,
												std::reference_wrapper<std::decay_t<value_type>>,
												mutable_value>;


	using mutable_error = std::remove_cv_t<error_type>;
	using StoredError_type = std::conditional_t<std::is_reference_v<error_type>,
			  std::reference_wrapper<std::decay_t<error_type>>,
			  mutable_error>;

	bool _engaged{false};
	union {
        StoredValue_type    _value;
        StoredError_type    _error;
    };
};



template <typename E>
class /*[[nodiscard]]*/ Result<void, E> {
public:
    using value_type = void;
    using error_type = E;

    static_assert(!std::is_same<E, void>::value,
            "Error type must be non-void");

public:

    Result(Result const&) = delete;
    Result& operator= (Result const&) = delete;

    /**
      Construct Ok<void> result from `none`.
     * Empty optional state interop
     */
    constexpr Result(None) noexcept
        : _maybeError{}
    {}

    /**
     * Construct Ok result by copying value
     * @param value Ok value to copy
     */
    constexpr Result(types::Ok<void> const& SOLACE_UNUSED(value)) noexcept
        : _maybeError{}
    {}

    /**
     * Move-Construct Ok result
     * @param value Ok value to move from
     */
    constexpr Result(types::Ok<void>&& SOLACE_UNUSED(value)) noexcept
        : _maybeError{}
    {}

    /**
     * Construct Err result by copying error value
     * @param err Err value to copy from
     */
    Result(types::Err<E> const& err) noexcept(std::is_nothrow_move_constructible<E>::value)
        : _maybeError{err.val_}
    {}

    /**
     * Move-Construct Err result by moving error value
     * @param err Err value to move from
     */
    Result(types::Err<E>&& err) noexcept(std::is_nothrow_move_constructible<E>::value)
        : _maybeError{mv(err.val_)}
    {}


    /**
     * Move-Construct Err result by moving error value
     * @param err Err value to move from
     */
    Result(E&& err) noexcept(std::is_nothrow_move_constructible<E>::value)
        : _maybeError{mv(err)}
    {}


    /**
     * Move-Construct Result of the same type
     * @param rhs Source to move values from
     */
    Result(Result&& rhs) noexcept(std::is_nothrow_move_constructible<E>::value)
        : _maybeError{mv(rhs._maybeError)}
    {}


    constexpr Result(types::OkTag) noexcept
        : _maybeError{}
    {}

	constexpr Result(types::OkTag, InPlace) noexcept
		: _maybeError{}
	{}

    constexpr Result(types::ErrTag, E const& value) noexcept(std::is_nothrow_copy_constructible<E>::value)
        : _maybeError{value}
    {}

    constexpr Result(types::ErrTag, E&& value) noexcept(std::is_nothrow_move_constructible<E>::value)
        : _maybeError{mv(value)}
    {}

	template<typename...Args>
	constexpr Result(types::ErrTag, InPlace t, Args&&...args)
		: _maybeError{t, fwd<Args>(args)...}
	{}

public:

    Result& swap(Result& rhs) noexcept {
        _maybeError.swap(rhs._maybeError);

        return (*this);
    }

    Result& operator= (Result&& rhs) noexcept {
		swap(rhs);

		return (*this);
	}

    explicit operator bool () const noexcept {
        return isOk();
    }

    bool isOk() const noexcept {
        return _maybeError.isNone();
    }

    bool isError() const noexcept {
        return _maybeError.isSome();
    }

    const E& getError() const {
        return _maybeError.get();
    }

    E& getError() {
        return _maybeError.get();
    }

    E&& moveError() {
        return _maybeError.move();
    }



    /// Template helper to deduce approaproate return type based on the Completion handler return type.
    template<typename F,
             typename R = typename std::result_of<F(void)>::type
             >
    using ResultType = typename std::conditional<isResult<void, E, R>::value,
                            typename isResult<void, E, R>::type,
                            typename std::conditional<std::is_same<R, void>::value,
                                Result<void, E>,
                                Result<R, E>
                            >::value
                        >::type;

    /**
     * Then combinator.
     * Calls 'f' on the Ok value if the result is Ok, otherwise returns the Err value of self.
     * This is an equivalent of flatMap for Optional value for 'f' returning Result<V, E>
     *
     * @param f callable object to call on the success value. It is only called if this::isOk() is true
     * @return Result<U, E> of the call of 'f' if this::isOk(), Err(this->getError()) otherwise
     */
    template<typename F,
             typename R = typename std::result_of<F(void)>::type
             >
    std::enable_if_t<isResult<void, E, R>::value, typename isResult<void, E, R>::type>
    then(F&& f) && {
        if (isOk()) {
            return f();
        }

        return Err(_maybeError.move());
    }

    template<typename F,
             typename R = typename std::result_of<F(void)>::type
             >
    std::enable_if_t<isResult<void, E, R>::value, typename isResult<void, E, R>::type>
    then(F&& f) const& {
        if (isOk()) {
            return f();
        }

        return Err(_maybeError.get());
    }



    template <typename F,
              typename R = typename std::result_of<F(void)>::type
              >
    std::enable_if_t<!std::is_same<R, void>::value && !isResult<void, E, R>::value, Result<R, E>>
    then(F&& f) && {
        if (isOk()) {
            return Ok<R>(f());
        }

        return Err(_maybeError.move());
    }

    template <typename F,
              typename R = typename std::result_of<F(void)>::type
              >
    std::enable_if_t<!std::is_same<R, void>::value && !isResult<void, E, R>::value, Result<R, E>>
    then(F&& f) const& {
        if (isOk()) {
            return Ok<R>(f());
        }

        return Err(_maybeError.get());
    }


    template <typename F,
              typename R = typename std::result_of<F(void)>::type
              >
    std::enable_if_t<std::is_same<void, R>::value, Result<void, E>>
    then(F&& f) && {
        if (isOk()) {
            f();

            return none;
        }

        return Err(_maybeError.move());
    }

    template <typename F,
              typename R = typename std::result_of<F(void)>::type
              >
    std::enable_if_t<std::is_same<void, R>::value, Result<void, E>>
    then(F&& f) const& {
        if (isOk()) {
            f();

            return none;
        }

        return Err(_maybeError.get());
    }


    //------------------------------------------------------------------

    template<typename F,
             typename R = typename std::result_of<F(E)>::type>
    std::enable_if_t<isResult<void, E, R>::value, typename isResult<void, E, R>::type>
    orElse(F&& f) {
        if (isOk()) {
            return none;
        }

        return f(moveError());
    }

    template<typename F,
             typename R = typename std::result_of<F(E)>::type>
    std::enable_if_t<!std::is_same<void, R>::value && !isResult<void, E, R>::value, Result<R, E>>
    orElse(F&& f) {
        if (isOk()) {
            return none;
        }

        return Ok<R>(f(moveError()));
    }

    template<typename F,
             typename U = typename std::result_of<F(E)>::type>
    std::enable_if_t<std::is_same<void, U>::value, Result<U, E>>
    orElse(F&& f) {
        if (isOk()) {
            return none;
        }

        f(moveError());

        return none;
    }


    /**
     * Pass through an Ok result but applies a given function to an error value.
     * This can be used to map error types.
     *
     * @param f - An error mapping function to map Err value.
     */
    template<typename F,
             typename RE = typename std::result_of<F(E)>::type,
             typename EE = typename error_result_wrapper<RE>::type>
    std::enable_if_t<!isResult<void, E, RE>::value, Result<void, EE>>
    mapError(F&& f) {
        if (isOk()) {
            return none;
        }

        return Err(f(moveError()));
    }

    template<typename F,
             typename RE = typename std::result_of<F(E)>::type,
             typename EE = typename error_result_wrapper<RE>::type,
             typename ResT = isResult<void, E, RE>
             >
    std::enable_if_t<ResT::value, typename ResT::type>
    mapError(F&& f) {
        if (isOk()) {
            return none;
        }

        return f(moveError());
    }

private:
    Optional<error_type> _maybeError;
};


template<typename E>
bool operator== (None, Result<void, E> const& res) noexcept { return res.isOk(); }

template<typename E>
bool operator== (Result<void, E> const& res, None) noexcept { return res.isOk(); }

template<typename E>
bool operator!= (None, Result<void, E> const& res) noexcept { return res.isError(); }

template<typename E>
bool operator!= (Result<void, E> const& res, None) noexcept { return res.isError(); }


template<typename V, typename E>
bool operator== (types::Ok<V> const& okValue, Result<V, E> const& res) {
    return res.isOk() && (res.unwrap() == okValue.val_);
}

template<typename V, typename E>
bool operator== (Result<V, E> const& res, types::Ok<V> const& okValue) {
    return res.isOk() && (res.unwrap() == okValue.val_);
}

template<typename V, typename E>
bool operator== (types::Err<E> const& errValue, Result<V, E> const& res) {
    return res.isError() && (res.getError() == errValue.val_);
}

template<typename V, typename E>
bool operator== (Result<V, E> const& res, types::Err<E> const& errValue) {
    return res.isError() && (res.getError() == errValue.val_);
}


template<typename V, typename E>
bool operator== (Result<V, E> const& lhs, Result<V, E> const& rhs) noexcept {
    return ((lhs.isOk() && rhs.isOk()) && (lhs.unwrap() == rhs.unwrap())) ||
           ((lhs.isError() && rhs.isError()) && (lhs.getError() == rhs.getError()));
}

template<typename E>
bool operator== (Result<void, E> const& lhs, Result<void, E> const& rhs) noexcept {
    return ((lhs.isOk() && rhs.isOk()) ||
           ((lhs.isError() && rhs.isError()) && (lhs.getError() == rhs.getError())));
}

template<typename V, typename E>
std::enable_if_t<!std::is_same_v<V, E>, bool>
operator== (V const& okValue, Result<V, E> const& res) {
	return res.isOk() && (res.unwrap() == okValue);
}

template<typename V, typename E>
std::enable_if_t<!std::is_same_v<V, E>, bool>
operator!= (V const& okValue, Result<V, E> const& res) {
	return res.isError() || (res.unwrap() != okValue);
}



}  // End of namespace Solace
#endif  // SOLACE_RESULT_HPP
