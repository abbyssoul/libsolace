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
 * libSolace: Channel for event loops
 *	@file		solace/io/eventloop/channel.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IO_EVENTLOOP_ASYNCRESULT_HPP
#define SOLACE_IO_EVENTLOOP_ASYNCRESULT_HPP

#include <functional>

namespace Solace { namespace IO { namespace async {

/**
 * An async future.
 * The Result class is a future that gets resolved once the value has been computed.
 *
 * @example
 * \code{.cpp}
 * io_object.do_something_async().then([](auto value) {
 *      ...
 *      use the value when it become available
 *      ...
 *
 * });
 * \endcode
 */
template <typename T>
class Future {
public:

    typedef T value_type;

public:

    ~Future() = default;

    Future(const Future& ) = delete;
    Future& operator= (const Future& rhs) = delete;

    Future() :
        _completionHandler()
    {}

    Future(Future&& rhs) :
        _completionHandler(std::move(rhs._handler))
    {}

    /**
     * FIXME(abbyssoul): then must return Future<std::result_of<F(value_type)>> to support chainning.
     * Attach completion handler aka callback to this future to call when it's ready.
     *
     * @param completionHandler A completion handler to attach to this futuure.
     */
    void then(std::function<void(const T&)>&& completionHandler) {
        _completionHandler = std::move(completionHandler);
    }

    // Resolve this future and call the handler.
    void resolve(const T& value) {
        if (_completionHandler) {
            _completionHandler(value);
        }
    }

private:

    std::function<void(const T&)> _completionHandler;
};


/** Specialization of AsyncResult for void type
 *
 */
template <>
class Future<void> {
public:
    typedef void value_type;

public:

    ~Future() = default;

    Future(const Future& ) = delete;
    Future& operator= (const Future& rhs) = delete;

    Future() :
        _handler()
    {}

    Future(Future&& rhs) :
        _handler(std::move(rhs._handler))
    {}

    void then(const std::function<void()>& handler) {
        _handler = handler;
    }

    // Resolve this future and call the handler.
    void resolve() {
        if (_handler) {
            // Note: what happenes if _handler throws? Eventloop.run() get intrapted!
            _handler();
        }
    }


private:

    std::function<void()> _handler;
};


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
    Promise();

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
    void setValue(V&& value);

    /**
     * Resolve this promise with a call to a given function
     * (use perfect forwarding for both move and copy)
    */
    template <typename F>
    void setWith(F& func);

};

}  // End of namespace async
}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_EVENTLOOP_ASYNCRESULT_HPP
