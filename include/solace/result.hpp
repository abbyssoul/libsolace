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

#include <functional>


namespace Solace {

// Implementation types
namespace types {


template<typename T>
struct Ok {
    using value_type = T;

    constexpr Ok(const T& val) noexcept(std::is_nothrow_copy_constructible<T>::value) : val_(val) { }
    constexpr Ok(T&& val) noexcept(std::is_nothrow_move_constructible<T>::value) : val_(std::move(val)) { }

    T val_;
};

template<>
struct Ok<void> {
    using value_type = void;
};



template<typename E>
struct Err {
    constexpr Err(const E& val) noexcept(std::is_nothrow_copy_constructible<E>::value) : val_(val) { }
    constexpr Err(E&& val)noexcept(std::is_nothrow_move_constructible<E>::value) : val_(std::move(val)) { }

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
inline None Ok() {
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
// TODO(c++17): [[nodiscard]]
class Result {
public:
    using value_type = V;
    using error_type = E;

    static_assert(!std::is_same<E, void>::value,
            "Error type must be non-void");

public:

    ~Result() {
        destroy();
    }


    /**
     * Move-Construct Ok result
     * @param value Ok value to move from
     */
    constexpr Result(types::Ok<V>&& value) noexcept(std::is_nothrow_move_constructible<V>::value) :
        _value(std::move(value.val_)),
        _engaged(true)
    {}

    /**
     * Type convertion Copy-Construct Ok result
     * @param value Ok value to move value from
     */
    template<typename DV>
    constexpr Result(types::Ok<DV>&& value) noexcept(std::is_nothrow_move_constructible<V>::value) :
        _value(std::move(value.val_)),
        _engaged(true)
    {}


    /**
     * Move-Construct Err result by moving error value
     * @param err Err value to move from
     */
    constexpr Result(types::Err<E>&& err) noexcept(std::is_nothrow_move_constructible<E>::value) :
        _error(std::move(err.val_)),
        _engaged(false)
    {}


    /**
     * Copy construct Result of the same type
     * @param rhs Source to copy values from
     */
    Result(Result const& rhs) = delete;

//    Result(const Result& rhs) /*noexcept*/ {
//        if (rhs.isOk()) {
//            ::new (reinterpret_cast<void *>(std::addressof(_value))) StoredValue_type(rhs._value);
//            _engaged = true;
//        } else {
//            ::new (reinterpret_cast<void *>(std::addressof(_error))) StoredError_type(rhs._error);
//            _engaged = false;
//        }
//    }

    /**
     * Move-Construct Result of the same type
     * @param rhs Source to move values from
     */
    Result(Result&& rhs) noexcept(std::is_nothrow_move_constructible<V>::value
                                  && std::is_nothrow_move_constructible<E>::value) {
        if (rhs.isOk()) {
            ::new (reinterpret_cast<void *>(std::addressof(_value))) StoredValue_type(std::move(rhs._value));
            _engaged = true;
        } else {
            ::new (reinterpret_cast<void *>(std::addressof(_error))) StoredError_type(std::move(rhs._error));
            _engaged = false;
        }
    }

//    template<typename DV>
//    Result(const Result<DV, E>& rhs) /*noexcept*/ {
//        if (rhs.isOk()) {
//            ::new (reinterpret_cast<void *>(std::addressof(_value))) StoredValue_type(rhs._value);
//            _engaged = true;
//        } else {
//            ::new (reinterpret_cast<void *>(std::addressof(_error))) StoredError_type(rhs._error);
//            _engaged = false;
//        }
//    }

    template<typename DV>
    Result(Result<DV, E>&& rhs) noexcept {
        if (rhs.isOk()) {
            ::new (reinterpret_cast<void *>(std::addressof(_value))) StoredValue_type(std::move(rhs._value));
            _engaged = true;
        } else {
            ::new (reinterpret_cast<void *>(std::addressof(_error))) StoredError_type(std::move(rhs._error));
            _engaged = false;
        }
    }

public:

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
    then(F&& f) {

        if (isOk()) {
            // TODO(abbyssoul): We probably should handle exeptions here
            return f(moveResult());
        }

        return Err<typename ResT::error_type>(moveError());
    }

    template <typename F,
              typename R = typename std::result_of<F(V)>::type
              >
    std::enable_if_t<!std::is_same<R, void>::value && !isResult<V, E, R>::value,
    Result<R, E>>
    then(F&& f) {

        if (isOk()) {
            // TODO(abbyssoul): Handle exeptions and convert then into Error
            return Ok<R>(f(moveResult()));
        }

        return Err(moveError());
    }


    template <typename F,
              typename R = typename std::result_of<F(V)>::type
              >
    std::enable_if_t<std::is_same<void, R>::value, Result<R, E>>
    then(F&& f) {

        if (isOk()) {
            // TODO(abbyssoul): Handle exeptions and convert them into error_type
            f(moveResult());

            return none;
        }

        return Err(moveError());
    }


    //------------------------------------------------------------------

    template<typename F,
             typename R = typename std::result_of<F(E)>::type>
    std::enable_if_t<isResult<V, E, R>::value, typename isResult<V, E, R>::type>
    orElse(F&& f) {
        if (isOk()) {
            return Ok(moveResult());
        }

        return f(moveError());
    }


    template<typename F,
             typename RE = typename std::result_of<F(E)>::type>
    std::enable_if_t<!isResult<V, E, RE>::value, Result<RE, E>>
    orElse(F&& f) {
        if (isOk()) {
            return Ok(moveResult());
        }

        // TODO(abbyssoul): Handle exeptions and convert then into Error
        return Ok(f(moveError()));
    }

    /**
     * Pass through a Ok result but applies a given function to an error value.
     * This can be used to handle errors.
     *
     * @param f - An error mapping function to map Err value.
     */
    template<typename F,
             typename EE = typename std::result_of<F(E)>::type>
    Result<V, EE> mapError(F&& f) {
        if (isOk()) {
            return Ok(moveResult());
        }

        // TODO(abbyssoul): Handle exeptions and convert then into Error
        return Err(f(moveError()));
    }



    Result& swap(Result& rhs) noexcept {
        using std::swap;

        if (isOk()) {
            if (rhs.isOk()) {
//                swap(_value, rhs._value);

                StoredValue_type v(std::move(_value));
                constructValue(std::move(rhs._value));
                rhs.constructValue(std::move(v));
            } else {
                StoredValue_type v(std::move(_value));
                constructError(std::move(rhs._error));
                rhs.constructValue(std::move(v));
            }

        } else {
            if (rhs.isOk()) {
                StoredValue_type v(std::move(rhs._value));
                rhs.constructError(std::move(_error));
                constructValue(std::move(v));
            } else {
//                swap(_error, rhs._error);
                StoredError_type v(std::move(_error));
                constructError(std::move(rhs._error));
                rhs.constructError(std::move(v));
            }
        }

        return (*this);
    }

    Result& operator= (Result&& rhs) noexcept {
        return swap(rhs);
    }

    Result& operator= (const Result& rhs) noexcept = delete;

    explicit operator bool () const noexcept {
        return isOk();
    }

    constexpr bool isOk() const noexcept {
        return _engaged;
    }

    constexpr bool isError() const noexcept {
        return !_engaged;
    }

    const V& unwrap() const {
        if (isError())
            raiseInvalidStateError();

        return _value;
    }

    V& unwrap() {
        if (isError())
            raiseInvalidStateError();

        return _value;
    }

    V&& moveResult() {
        if (isError())
            raiseInvalidStateError();

        return std::move(_value);
    }

    E&& moveError() {
        if (isOk())
            raiseInvalidStateError();

        return std::move(_error);
    }

    const E& getError() const {
        if (isOk())
            raiseInvalidStateError();

        return _error;
    }

private:

    void destroy() {
        if (_engaged) {
            _value.~StoredValue_type();
        } else {
            _error.~StoredError_type();
        }
    }

    void constructValue(const V& t) {
        destroy();

        ::new (reinterpret_cast<void *>(std::addressof(_value))) StoredValue_type(t);
        _engaged = true;
    }

    void constructValue(V&& t) {
        destroy();

        ::new (reinterpret_cast<void *>(std::addressof(_value))) StoredValue_type(std::move(t));
        _engaged = true;
    }


    void constructError(const E& t) {
        destroy();

        ::new (reinterpret_cast<void *>(std::addressof(_error))) StoredError_type(t);
        _engaged = false;
    }

    void constructError(E&& t) {
        destroy();

        ::new (reinterpret_cast<void *>(std::addressof(_error))) StoredError_type(std::move(t));
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
// TODO(c++17): [[nodiscard]]
class Result<void, E> {
public:
    using value_type = void;
    using error_type = E;

    static_assert(!std::is_same<E, void>::value,
            "Error type must be non-void");

public:

    ~Result() noexcept(std::is_nothrow_destructible<E>::value) = default;

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
    constexpr Result(const types::Ok<void>& SOLACE_UNUSED(value)) noexcept
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
    Result(const types::Err<E>& err) noexcept(std::is_nothrow_move_constructible<E>::value)
        : _maybeError{err.val_}
    {}

    /**
     * Move-Construct Err result by moving error value
     * @param err Err value to move from
     */
    Result(types::Err<E>&& err) noexcept(std::is_nothrow_move_constructible<E>::value)
        : _maybeError(std::move(err.val_))
    {}


    /**
     * Move-Construct Err result by moving error value
     * @param err Err value to move from
     */
    Result(E&& err) noexcept(std::is_nothrow_move_constructible<E>::value)
        : _maybeError(std::move(err))
    {}


    /**
     * Move-Construct Result of the same type
     * @param rhs Source to move values from
     */
    Result(Result&& rhs) noexcept(std::is_nothrow_move_constructible<E>::value) :
        _maybeError(std::move(rhs._maybeError))
    {}

public:

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
    then(F&& f) {

        if (isOk()) {
            // TODO(abbyssoul): We probably should handle exeptions here
            return f();
        }

        return Err(moveError());
    }

    template <typename F,
              typename R = typename std::result_of<F(void)>::type
              >
    std::enable_if_t<!std::is_same<R, void>::value && !isResult<void, E, R>::value, Result<R, E>>
    then(F&& f) {

        if (isOk()) {
            // TODO(abbyssoul): Handle exeptions and convert then into Error
            return Ok<R>(f());
        }

        return Err(moveError());
    }


    template <typename F,
              typename R = typename std::result_of<F(void)>::type
              >
    std::enable_if_t<std::is_same<void, R>::value, Result<void, E>>
    then(F&& f) {

        if (isOk()) {
            // TODO(abbyssoul): Handle exeptions and convert then into Error
            f();

            return none;
        }

        return Err(moveError());
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
             typename ER = typename std::result_of<F(E)>::type>
    Result<void, ER> mapError(F&& f) {
        if (isOk()) {
            return none;
        }

        return Err(f(moveError()));
    }

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
bool operator== (const types::Ok<V>& okValue, const Result<V, E>& res) {
    return res.isOk() && (res.unwrap() == okValue.val_);
}

template<typename V, typename E>
bool operator== (const Result<V, E>& res, const types::Ok<V>& okValue) {
    return res.isOk() && (res.unwrap() == okValue.val_);
}

template<typename V, typename E>
bool operator== (const types::Err<E>& errValue, const Result<V, E>& res) {
    return res.isError() && (res.getError() == errValue.val_);
}

template<typename V, typename E>
bool operator== (const Result<V, E>& res, const types::Err<E>& errValue) {
    return res.isError() && (res.getError() == errValue.val_);
}

}  // End of namespace Solace
#endif  // SOLACE_RESULT_HPP
