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
#include "solace/optional.hpp"


namespace Solace {

// Implementation types
namespace types {

struct OkTag {};
struct ErrTag {};

template<typename T>
struct Ok {
    using value_type = T;

    constexpr Ok(T const& val) noexcept(std::is_nothrow_copy_constructible<T>::value)
        : val_{val}
    { }

    constexpr Ok(T&& val) noexcept(std::is_nothrow_move_constructible<T>::value)
        : val_{mv(val)}
    { }

    T val_;
};

template<>
struct Ok<void> {
    using value_type = void;
};



template<typename E>
struct Err {
    using value_type = E;

    constexpr Err(E const& val) noexcept(std::is_nothrow_copy_constructible<E>::value)
        : val_{val}
    { }

    constexpr Err(E&& val) noexcept(std::is_nothrow_move_constructible<E>::value)
        : val_{mv(val)}
    { }

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
         typename CleanT = typename std::decay<T>::type >
inline types::Ok<CleanT> Ok(T&& val) {
    return types::Ok<CleanT>(std::forward<T>(val));
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
    return types::Err<CleanE>(std::forward<E>(val));
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
 * Result class is an 'enum' of two values V and E with V being 'expected'.
 * It is very similar to Either<> monad in some functional languages and
 * is ispired by std::Result<> class from Rust lang.
 *
 * Result class is similar in concept to Future in the sense that it can represent result of the computition or an error
 * with the difference that it is not design for async use case.
 * That is why result is always 'set'.
 *
 * Note: Result<void, Error> is technically equivalent of Optional<Error>
 */
template <typename V, typename E>
class /*[[nodiscard]] */Result {
public:
    using value_type = V;
    using error_type = E;

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
    Result(Result&& rhs) noexcept(std::is_nothrow_move_constructible<V>::value
                                  && std::is_nothrow_move_constructible<E>::value) {
        if (rhs.isOk()) {
            ::new (reinterpret_cast<void *>(std::addressof(_value))) StoredValue_type(mv(rhs._value));
            _engaged = true;
        } else {
            ::new (reinterpret_cast<void *>(std::addressof(_error))) StoredError_type(mv(rhs._error));
            _engaged = false;
        }
    }

    template<typename DV>
    Result(Result<DV, E>&& rhs) noexcept {
        if (rhs.isOk()) {
            ::new (reinterpret_cast<void *>(std::addressof(_value))) StoredValue_type(mv(rhs._value));
            _engaged = true;
        } else {
            ::new (reinterpret_cast<void *>(std::addressof(_error))) StoredError_type(mv(rhs._error));
            _engaged = false;
        }
    }

    Result& operator= (Result&& rhs) noexcept {
        return swap(rhs);
    }



    /**
     * Move-Construct Ok result
     * @param value Ok value to move from
     */
    constexpr Result(types::Ok<V>&& value) noexcept(std::is_nothrow_move_constructible<V>::value)
        : Result(types::OkTag{}, mv(value.val_))
    {}

    /**
     * Type convertion Copy-Construct Ok result
     * @param value Ok value to move value from
     */
    template<typename DV>
    constexpr Result(types::Ok<DV>&& value) noexcept(std::is_nothrow_move_constructible<V>::value)
        : Result(types::OkTag{}, mv(value.val_))
    {}

    /**
     * Move-Construct Err result by moving error value
     * @param err Err value to move from
     */
    constexpr Result(types::Err<E>&& value) noexcept(std::is_nothrow_move_constructible<E>::value)
        : Result(types::ErrTag{}, mv(value.val_))
    {}

    /**
     * Move-Construct Err result from a compatible error type by moving error value
     * @param err Err value to move from
     */
    template<typename DE>
    constexpr Result(types::Err<DE>&& value) noexcept(std::is_nothrow_move_constructible<E>::value)
        : Result(types::ErrTag{}, mv(value.val_))
    {}


    constexpr Result(types::OkTag, V const& value) noexcept(std::is_nothrow_copy_constructible<V>::value)
        : _value{value}
        , _engaged{true}
    {}

    constexpr Result(types::OkTag, V&& value) noexcept(std::is_nothrow_move_constructible<V>::value)
        : _value{mv(value)}
        , _engaged{true}
    {}

    constexpr Result(types::ErrTag, E const& value) noexcept(std::is_nothrow_copy_constructible<E>::value)
        : _error{value}
        , _engaged{false}
    {}

    constexpr Result(types::ErrTag, E&& value) noexcept(std::is_nothrow_move_constructible<E>::value)
        : _error{mv(value)}
        , _engaged{false}
    {}

public:

    Result& swap(Result& rhs) noexcept {
        using std::swap;

        if (isOk()) {
            if (rhs.isOk()) {
//                swap(_value, rhs._value);

                StoredValue_type v(mv(_value));
                constructValue(mv(rhs._value));
                rhs.constructValue(mv(v));
            } else {
                StoredValue_type v(mv(_value));
                constructError(mv(rhs._error));
                rhs.constructValue(mv(v));
            }

        } else {
            if (rhs.isOk()) {
                StoredValue_type v(mv(rhs._value));
                rhs.constructError(mv(_error));
                constructValue(mv(v));
            } else {
//                swap(_error, rhs._error);
                StoredError_type v(mv(_error));
                constructError(mv(rhs._error));
                rhs.constructError(mv(v));
            }
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

        return typename ResT::type{types::ErrTag{}, mv(_error)};
    }

    template<typename F,
             typename R = typename std::result_of<F(V)>::type,
             typename ResT = isResult<V, E, R>>
    std::enable_if_t<ResT::value, typename ResT::type>
    then(F&& f) & {
        if (isOk()) {
            return f(_value);
        }

        return typename ResT::type{types::ErrTag{}, _error};
    }

    template<typename F,
             typename R = typename std::result_of<F(V)>::type,
             typename ResT = isResult<V, E, R>>
    std::enable_if_t<ResT::value, typename ResT::type>
    then(F&& f) const& {
        if (isOk()) {
            return f(_value);
        }

        return typename ResT::type{types::ErrTag{}, _error};
    }


    template <typename F,
              typename R = typename std::result_of<F(V)>::type
              >
    std::enable_if_t<!std::is_same<R, void>::value && !isResult<V, E, R>::value,  Result<R, E>>
    then(F&& f) {
        if (isOk()) {
            return Result<R, E>{types::OkTag{}, f(mv(_value))};
        }

        return Result<R, E>{types::ErrTag{}, mv(_error)};
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
            return typename ResT::type{types::OkTag{}, moveResult()};
        }

        return f(moveError());
    }


    template<typename F,
             typename RE = typename std::result_of<F(E)>::type>
    std::enable_if_t<!isResult<V, E, RE>::value, Result<RE, E>>
    orElse(F&& f) {
        if (isOk()) {
            return Result<RE, E>{types::OkTag{}, moveResult()};
        }

        return Result<RE, E>(types::OkTag{}, f(moveError()));
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
            return Result<V, EE>{types::OkTag{}, mv(_value)};
        }

        return Result<V, EE>{types::ErrTag{}, f(mv(_error))};
    }

    template<typename F,
             typename RE = typename std::result_of<F(E)>::type,
             typename EE = typename error_result_wrapper<RE>::type>
    Result<V, EE> mapError(F&& f) & {
        if (isOk()) {
            return Result<V, EE>{types::OkTag{}, unwrap()};
        }

        return Result<V, EE>{types::ErrTag{}, f(getError())};
    }

    template<typename F,
             typename RE = typename std::result_of<F(E)>::type,
             typename EE = typename error_result_wrapper<RE>::type>
    Result<V, EE> mapError(F&& f) const& {
        if (isOk()) {
            return Result<V, EE>{types::OkTag{}, unwrap()};
        }

        return Result<V, EE>{types::ErrTag{}, f(getError())};
    }

private:

    void destroy() {
        if (_engaged) {
            _value.~StoredValue_type();
        } else {
            _error.~StoredError_type();
        }
    }

    void constructValue(V const& t) {
        destroy();

        ::new (reinterpret_cast<void *>(std::addressof(_value))) StoredValue_type(t);
        _engaged = true;
    }

    void constructValue(V&& t) {
        destroy();

        ::new (reinterpret_cast<void *>(std::addressof(_value))) StoredValue_type(mv(t));
        _engaged = true;
    }


    void constructError(E const& t) {
        destroy();

        ::new (reinterpret_cast<void *>(std::addressof(_error))) StoredError_type(t);
        _engaged = false;
    }

    void constructError(E&& t) {
        destroy();

        ::new (reinterpret_cast<void *>(std::addressof(_error))) StoredError_type(mv(t));
        _engaged = false;
    }

private:

    template<typename DV, typename DE>
    friend class Result;

    using   StoredValue_type = std::remove_const_t<V>;
    using   StoredError_type = std::remove_const_t<E>;

    union {
        StoredValue_type    _value;
        StoredError_type    _error;
    };

    bool _engaged = false;
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

    constexpr Result(types::ErrTag, E const& value) noexcept(std::is_nothrow_copy_constructible<E>::value)
        : _maybeError{value}
    {}

    constexpr Result(types::ErrTag, E&& value) noexcept(std::is_nothrow_move_constructible<E>::value)
        : _maybeError{mv(value)}
    {}

public:

    Result& swap(Result& rhs) noexcept {
        _maybeError.swap(rhs._maybeError);

        return (*this);
    }

    Result& operator= (Result&& rhs) noexcept {
        return swap(rhs);
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


}  // End of namespace Solace
#endif  // SOLACE_RESULT_HPP
