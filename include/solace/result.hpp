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
    typedef T value_type;

    Ok(const T& val) : val_(val) { }
    Ok(T&& val) : val_(std::move(val)) { }

    T val_;
};

template<>
struct Ok<void> {
    typedef void value_type;
};



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
         typename CleanT = typename std::decay<T>::type >
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

    typedef T value_type;
};



template <typename T>
struct isSomeResult : std::false_type {
    using std::false_type::value;

    typedef T value_type;
};

template <typename V, typename E>
struct isSomeResult<Result<V, E>> : std::true_type {
    using std::true_type::value;

    using type = Result<V, E>;

    typedef typename type::value_type value_type;
    typedef typename type::error_type error_type;
};

/*
template <typename T>
struct isOkResult : std::false_type {
    using std::false_type::value;

    typedef T value_type;
};


template <typename T>
struct isErrorResult : std::false_type {
    using std::false_type::value;

    typedef T value_type;
};


template <typename V, typename E>
struct isResult<Result<V, E>> : std::true_type {
    using std::true_type::value;

    using type = Result<V, E>;

    typedef typename type::value_type value_type;
    typedef typename type::error_type error_type;
};
*/


template <
        typename OV, typename OE,
        typename V, typename E>
struct isResult<OV, OE, Result<V, E>> : std::true_type {
    using std::true_type::value;

    using type = Result<V, E>;

    typedef typename type::value_type value_type;
    typedef typename type::error_type error_type;
};


template <
        typename OV, typename OE,
        typename V>
struct isResult<OV, OE, types::Ok<V>> : std::true_type {
    using std::true_type::value;

    using type = Result<V, OE>;

    typedef typename type::value_type value_type;
    typedef typename type::error_type error_type;
};

template <
        typename OV, typename OE,
        typename E>
struct isResult<OV, OE, types::Err<E>> : std::true_type {
    using std::true_type::value;

    using type = Result<OV, E>;

    typedef typename type::value_type value_type;
    typedef typename type::error_type error_type;
};


/*
template <typename V>
struct isOkResult<types::Ok<V>> : std::true_type {
    using std::true_type::value;

    using type = types::Ok<V>;

    typedef typename type::value_type value_type;
};

template <typename E>
struct isErrorResult<types::Err<E>> : std::true_type {
    using std::true_type::value;

    using type = types::Err<E>;
    typedef typename type::error_type error_type;
};
*/


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
    typedef V value_type;
    typedef E error_type;

    static_assert(!std::is_same<E, void>::value,
            "Error type must be non-void");

public:

    ~Result() {
        if (_state) {
            _state->~IState();
            _state = nullptr;
        }
    }


    /**
     * Construct Ok result by copying value
     * @param value Ok value to copy
     */
    Result(const types::Ok<V>& value):
        _state( ::new (_stateBuffer.okSpace) OkState(value) )
    {}

    /**
     * Move-Construct Ok result
     * @param value Ok value to move from
     */
    Result(types::Ok<V>&& value):
        _state( ::new (_stateBuffer.okSpace) OkState(std::move(value)) )
    {}

    /**
     * Type convertion Copy-Construct Ok result
     * @param value Ok value to move value from
     */
    template<typename DV>
    Result(types::Ok<DV>&& value):
        _state( ::new (_stateBuffer.okSpace) OkState(std::move(value.val_)) )
    {}

    /**
     * Construct Err result by copying error value
     * @param err Err value to copy from
     */
    Result(const types::Err<E>& err):
        _state( ::new (_stateBuffer.errSpace) ErrorState(err) )
    {}

    /**
     * Move-Construct Err result by moving error value
     * @param err Err value to move from
     */
    Result(types::Err<E>&& err):
        _state( ::new (_stateBuffer.errSpace) ErrorState(std::move(err)) )
    {}


    /**
     * Copy construct Result of the same type
     * @param rhs Source to copy values from
     */
    Result(const Result& rhs) noexcept :
        _state(rhs.isOk()
               ? static_cast<IState*>(::new (_stateBuffer.okSpace) OkState(rhs.unwrap()))
               : static_cast<IState*>(::new (_stateBuffer.errSpace) ErrorState(rhs.getError())))
    {
    }

    /**
     * Move-Construct Result of the same type
     * @param rhs Source to move values from
     */
    Result(Result&& rhs) noexcept :
        _state(rhs.isOk()
               ? static_cast<IState*>(::new (_stateBuffer.okSpace) OkState(rhs.moveResult()))
               : static_cast<IState*>(::new (_stateBuffer.errSpace) ErrorState(rhs.moveError())))
    {
    }

    template<typename DV>
    Result(const Result<DV, E>& rhs) noexcept :
        _state(rhs.isOk()
               ? static_cast<IState*>(::new (_stateBuffer.okSpace) OkState(rhs.unwrap()))
               : static_cast<IState*>(::new (_stateBuffer.errSpace) ErrorState(rhs.getError())))
    {
    }

    template<typename DV>
    Result(Result<DV, E>&& rhs) noexcept :
        _state(rhs.isOk()
               ? static_cast<IState*>(::new (_stateBuffer.okSpace) OkState(rhs.moveResult()))
               : static_cast<IState*>(::new (_stateBuffer.errSpace) ErrorState(rhs.moveError())))
    {
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
             typename R = typename std::result_of<F(V)>::type>
    std::enable_if_t<isResult<V, E, R>::value, typename isResult<V, E, R>::type>
    then(F&& f) {

        if (isOk()) {
            // TODO(abbyssoul): We probably should handle exeptions here
            return f(moveResult());
        }

        return Err(moveError());
    }

//    template<typename F,
//             typename R = typename std::result_of<F(V)>::type>
//    std::enable_if_t<isOkResult<R>::value, Result<typename R::value_type, E>>
//    then(F&& f) {
//        if (isOk()) {
//            // TODO(abbyssoul): We probably should handle exeptions here
//            return f(unwrap());
//        }

//        return Err(getError());
//    }

//    template<typename F,
//             typename R = typename std::result_of<F(V)>::type>
//    std::enable_if_t<isErrorResult<R>::value, Result<V, typename R::error_type>>
//    then(F&& f) {
//        if (isOk()) {
//            // TODO(abbyssoul): We probably should handle exeptions here
//            return f(unwrap());
//        }

//        return Err(getError());
//    }


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
            // TODO(abbyssoul): Handle exeptions and convert then into Error
            f(moveResult());

            return Ok();
        }

        return Err(moveError());
    }


    //------------------------------------------------------------------

    template<typename F,
             typename R = typename std::result_of<F(E)>::type>
    std::enable_if_t<isResult<V, E, R>::value, typename isResult<V, E, R>::type>
    orElse(F&& f) {
        if (isOk()) {
            return *this;
        }

        return f(moveError());
    }


    template<typename F,
             typename RE = typename std::result_of<F(E)>::type>
    std::enable_if_t<!isResult<V, E, RE>::value, Result<RE, E>>
    orElse(F&& f) {
        if (isOk()) {
            return *this;
        }
        // TODO(abbyssoul): Handle exeptions and convert then into Error
        return Ok(f(moveError()));
    }


//    template<typename F,
//             typename RE = typename std::result_of<F(E)>::type>
//    std::enable_if_t<isOkResult<RE>::value, Result<typename RE::value_type, E>>
//    orElse(F&& f) {
//        if (isOk()) {
//            return *this;
//        }

//        return f(getError());
//    }

//    template<typename F,
//             typename RE = typename std::result_of<F(E)>::type>
//    std::enable_if_t<isErrorResult<RE>::value, Result<V, typename RE::error_type>>
//    orElse(F&& f) {
//        if (isOk()) {
//            return *this;
//        }

//        return f(getError());
//    }


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

        if (isOk()) {
            V v{ std::move(unwrap()) };

            if (rhs.isOk())
                _state = ::new (_stateBuffer.okSpace) OkState(std::move(rhs.unwrap()));
            else
                _state = ::new (_stateBuffer.errSpace) ErrorState(std::move(rhs.getError()));

            rhs._state->~IState();
            rhs._state = ::new (rhs._stateBuffer.okSpace) OkState(std::move(v));
        } else {
            E e{ std::move(getError()) };

            if (rhs.isOk())
                _state = ::new (_stateBuffer.okSpace) OkState(std::move(rhs.unwrap()));
            else
                _state = ::new (_stateBuffer.errSpace) ErrorState(std::move(rhs.getError()));

            rhs._state->~IState();
            rhs._state = ::new (rhs._stateBuffer.errSpace) ErrorState(std::move(e));
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

    bool isOk() const noexcept {
        return _state->isOk();
    }

    bool isError() const noexcept {
        return _state->isError();
    }

    const V& unwrap() const& {
        return *_state->getResult();
    }

    V& unwrap() & {
        return *_state->getResult();
    }

    V&& unwrap() && {
        return std::move(*_state->getResult());
    }

    V&& moveResult() {
        return std::move(*_state->getResult());
    }

    E&& moveError() {
        return std::move(*_state->getError());
    }

    const E& getError() const & {
        return *_state->getError();
    }

    E& getError() & {
        return *_state->getError();
    }

    E&& getError() && {
        return std::move(*_state->getError());
    }

private:

    template<typename T>
    class AlignedStorage {
    public:
        ~AlignedStorage() {
            ptr_ref()->T::~T();
        }

        void const* address() const { return _dummy; }
        void      * address()       { return _dummy; }

        T const* ptr_ref() const { return static_cast<T const*>(address()); }
        T *      ptr_ref()       { return static_cast<T *>     (address()); }

    private:
        std::aligned_storage_t<sizeof(T), alignof(T)> _dummy[1];
    };

private:

    class IState {
    public:
        virtual ~IState() = default;

        virtual bool isOk() const = 0;
        virtual bool isError() const = 0;

        virtual const V*  getResult() const = 0;
        virtual V*        getResult()       = 0;

        virtual const E* getError() const = 0;
        virtual E*       getError()       = 0;
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

        bool isOk() const override      { return true; }
        bool isError() const override   { return false; }

        const V* getResult() const override   { return _storage.ptr_ref(); }
        V* getResult() override               { return _storage.ptr_ref(); }

        const E* getError() const override      { raiseInvalidStateError(); return nullptr; }
        E* getError()  override                 { raiseInvalidStateError(); return nullptr; }

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

        bool isOk() const override { return false; }
        bool isError() const override { return true; }

        const V* getResult() const override   { raiseInvalidStateError(); return nullptr; }
        V* getResult() override               { raiseInvalidStateError(); return nullptr; }

        const E* getError() const  override   { return _storage.ptr_ref(); }
        E* getError() override                { return _storage.ptr_ref(); }

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



template <typename E>
// TODO(c++17): [[nodiscard]]
class Result<void, E> {
public:
    typedef void value_type;
    typedef E error_type;

    static_assert(!std::is_same<E, void>::value,
            "Error type must be non-void");

public:

    ~Result() = default;

    /**
     * Construct Ok result by copying value
     * @param value Ok value to copy
     */
    Result(const types::Ok<void>& SOLACE_UNUSED(value)) :
        _maybeError(Optional<E>::none())
    {}

    /**
     * Move-Construct Ok result
     * @param value Ok value to move from
     */
    Result(types::Ok<void>&&):
        _maybeError(Optional<E>::none())
    {}

    /**
     * Construct Err result by copying error value
     * @param err Err value to copy from
     */
    Result(const types::Err<E>& err):
        _maybeError(Optional<E>::of(err.val_))
    {}

    /**
     * Move-Construct Err result by moving error value
     * @param err Err value to move from
     */
    Result(types::Err<E>&& err):
        _maybeError(Optional<E>::of(std::move(err.val_)))
    {}


    /**
     * Copy construct Result of the same type
     * @param rhs Source to copy values from
     */
    Result(const Result& rhs) noexcept :
        _maybeError(rhs._maybeError)
    {
    }

    /**
     * Move-Construct Result of the same type
     * @param rhs Source to move values from
     */
    Result(Result&& rhs) noexcept :
        _maybeError(std::move(rhs._maybeError))
    {
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


//    template<typename F,
//             typename R = typename std::result_of<F(void)>::type
//             >
//    std::enable_if_t<isOkResult<R>::value, Result<typename R::value_type, E>>
//    then(F&& f) const {

//        if (isOk()) {
//            // TODO(abbyssoul): We probably should handle exeptions here
//            return f();
//        }

//        return Err(getError());
//    }


//    template<typename F,
//             typename R = typename std::result_of<F(void)>::type
//             >
//    std::enable_if_t<isErrorResult<R>::value, Result<void, typename R::error_type>>
//    then(F&& f) const {

//        if (isOk()) {
//            // TODO(abbyssoul): We probably should handle exeptions here
//            return f();
//        }

//        return Err(getError());
//    }


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
    std::enable_if_t<std::is_same<void, R>::value, Result<R, E>>
    then(F&& f) {

        if (isOk()) {
            // TODO(abbyssoul): Handle exeptions and convert then into Error
            f();

            return Ok();
        }

        return Err(moveError());
    }

    template <typename F,
              typename R = typename std::result_of<F(void)>::type
              >
    std::enable_if_t<std::is_same<void, R>::value, Result<R, E>>
    then(F&& f) const {

        if (isOk()) {
            // TODO(abbyssoul): Handle exeptions and convert then into Error
            f();

            return Ok();
        }

        return Err(moveError());
    }

    //------------------------------------------------------------------

    template<typename F,
             typename R = typename std::result_of<F(E)>::type>
    std::enable_if_t<isResult<void, E, R>::value, typename isResult<void, E, R>::type>
    orElse(F&& f) {
        if (isOk()) {
            return *this;
        }

        return f(moveError());
    }

    template<typename F,
             typename R = typename std::result_of<F(E)>::type>
    std::enable_if_t<!std::is_same<void, R>::value && !isResult<void, E, R>::value, Result<R, E>>
    orElse(F&& f) {

        if (isOk()) {
            return *this;
        }

        return Ok<R>(f(moveError()));
    }

    template<typename F,
             typename U = typename std::result_of<F(E)>::type>
    std::enable_if_t<std::is_same<void, U>::value, Result<U, E>>
    orElse(F&& f) {

        if (isOk()) {
            return *this;
        }

        f(moveError());

        return Ok();
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
            return Ok();
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

    Result& operator= (const Result& rhs) noexcept = delete;

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


template<typename V, typename E>
bool operator == (const types::Ok<V>& okValue, const Result<V, E>& res) {
    return res.isOk() && (res.unwrap() == okValue.val_);
}

template<typename V, typename E>
bool operator == (const Result<V, E>& res, const types::Ok<V>& okValue) {
    return res.isOk() && (res.unwrap() == okValue.val_);
}

template<typename V, typename E>
bool operator == (const types::Err<E>& errValue, const Result<V, E>& res) {
    return res.isError() && (res.getError() == errValue.val_);
}

template<typename V, typename E>
bool operator == (const Result<V, E>& res, const types::Err<E>& errValue) {
    return res.isError() && (res.getError() == errValue.val_);
}

}  // End of namespace Solace
#endif  // SOLACE_RESULT_HPP
