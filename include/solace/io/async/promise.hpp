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
 * libSolace: Promise side of the Future
 *	@file		solace/io/async/promise.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IO_ASYNC_PROMISE_HPP
#define SOLACE_IO_ASYNC_PROMISE_HPP

#include <solace/delegate.hpp>

#include <solace/result.hpp>
#include <solace/error.hpp>
#include <solace/assert.hpp>

#include <atomic>


namespace Solace { namespace IO { namespace async {

namespace details  {

template<typename T>
struct CallbackBase : public std::enable_shared_from_this<CallbackBase<T>> {
    virtual ~CallbackBase() = default;

    using std::enable_shared_from_this<CallbackBase<T>>::shared_from_this;

    virtual void operator() (Result<T, Error>&& result) = 0;
};

}  // namespace details


template<typename T>
class Core {
public:
    ~Core() = default;

    Core() :
        _completionHandler()
    {}

    Core(const Core&) = delete;
    Core& operator= (const Core&) = delete;

    Core(Core&&) = delete;
    Core& operator= (Core&&) = delete;


    void setCallback(std::shared_ptr<details::CallbackBase<T>>&& func) {
        _completionHandler = std::move(func);
    }

    void setResult(Result<T, Error>&& result) {
        if (_fired.exchange(true)) {
            raiseInvalidStateError("Future value already set");
        }

        if (_completionHandler) {
            // TODO(abbyssoul): Handle exceptions! What happenes when completion handler throws?
            (*_completionHandler)(std::move(result));
            _completionHandler.reset();
        }
    }

private:

//    delegate<void(Result<T, Error>&&)> _completionHandler;
    std::atomic<bool> _fired {false};
    std::shared_ptr<details::CallbackBase<T>> _completionHandler;

};


// Forward declation so Promise could reference it
template<typename T>
class Future;


/**
 * Promise is the 'push' side of a future
 */
template<typename T>
class Promise {
public:
    typedef T value_type;

public:
    ~Promise() = default;

    Promise(const Promise& ) = delete;
    Promise& operator= (const Promise& rhs) = delete;

    /**
     * Construct an empty promise
     */
    Promise() noexcept :
        _core(std::make_shared<Core<T>>())
    {}

    Promise(Promise<T>&& rhs) noexcept : Promise() {
        swap(rhs);
    }

    Promise& operator= (Promise<T>&& rhs) noexcept {
        return swap(rhs);
    }

    Promise& swap(Promise& rhs) noexcept {
        using std::swap;
        swap(_core, rhs._core);

        return *this;
    }

    /**
     * Get future associated with this Promise.
     * Note: this is mean to be called only once, thereafter
     * an exception will be raised.
     *
     * @return A Future assocciated with this promise
     */
    Future<T> getFuture();

    /**
     * Resolve this promise with a value.
     * (use perfect forwarding for both move and copy)
    */
    template <typename V>
    void setValue(V&& value) {
        _core->setResult(Ok<V>(std::forward<V>(value)));
    }


    /**
     * Resolve this promise with an error.
     * (use perfect forwarding for both move and copy)
    */
    template <typename E>
    void setError(E&& e) {
        _core->setResult(Err<E>(std::forward<E>(e)));
    }


    /**
     * Fulfill this Promise with the result of a function that takes no
     * arguments and returns something implicitly convertible to T.
     * Captures exceptions. e.g.
     * p.setWith([] { do something that may throw; return a T; });
    */
    template <typename F>
    void setWith(F&& func) {
        // FIXME: Handle excetions!
        setValue(func());
    }

protected:

    template <class> friend class Future;

private:

    std::shared_ptr<Core<T>> _core;

};



template<>
class Promise<void> {
public:
    typedef void value_type;

public:
    ~Promise() = default;

    Promise(const Promise& ) = delete;
    Promise& operator= (const Promise& rhs) = delete;

    /**
     * Construct an empty promise
     */
    Promise() noexcept :
        _core(std::make_shared<Core<void>>())
    {}

    Promise(Promise&& rhs) noexcept : Promise() {
        swap(rhs);
    }

    Promise& operator= (Promise&& rhs) noexcept {
        return swap(rhs);
    }

    Promise& swap(Promise& rhs) noexcept {
        using std::swap;
        swap(_core, rhs._core);

        return *this;
    }

    /**
     * Get future associated with this Promise.
     * Note: this is mean to be called only once, thereafter
     * an exception will be raised.
     *
     * @return A Future assocciated with this promise
     */
    Future<value_type> getFuture();

    /**
     * Resolve this promise with a value.
     * (use perfect forwarding for both move and copy)
    */
    void setValue() {
        _core->setResult(Ok());
    }

    /**
     * Resolve this promise with an error.
     * (use perfect forwarding for both move and copy)
    */
    template <typename E>
    void setError(E&& e) {
        _core->setResult(Err<E>(std::forward<E>(e)));
    }

    /**
     * Fulfill this Promise with the result of a function that takes no
     * arguments and returns something implicitly convertible to T.
     * Captures exceptions. e.g.
     * p.setWith([] { do something that may throw; return a T; });
    */
    template <typename F>
    void setWith(F&& func) {
        // FIXME: Handle excetions!
        func();
        setValue();
    }


private:
    template <class> friend class Future;

    std::shared_ptr<Core<void>> _core;

};



}  // End of namespace async
}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_ASYNC_PROMISE_HPP
