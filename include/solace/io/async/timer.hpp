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
 * libSolace: Async event
 *	@file		solace/io/async/event.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IO_ASYNC_TIMER_HPP
#define SOLACE_IO_ASYNC_TIMER_HPP


#include "solace/future.hpp"
#include "solace/io/selectable.hpp"
#include "solace/io/async/channel.hpp"

#include <chrono>


namespace Solace { namespace IO { namespace async {

/**
 * An async wrapper for the POSIX eventFd
 */
class Timer :
        public Channel {
public:

    using Channel::size_type;
    using time_type = std::chrono::milliseconds;

public:

    ~Timer();

    Timer(const Timer& rhs) = delete;
    Timer& operator= (const Timer& rhs) = delete;

    Timer(EventLoop& ioContext);

    Timer(EventLoop& ioContext, const time_type& d);

    Timer(Timer&& rhs);

    Timer& operator= (Timer&& rhs) noexcept {
        return swap(rhs);
    }

    Timer& swap(Timer& rhs) noexcept {
        using std::swap;

        Channel::swap(rhs);
        swap(_fd, rhs._fd);

        return *this;
    }

    Future<int64_t> asyncWait();

    Timer& setTimeout(const time_type& d);
    Timer& setTimeoutInterval(const time_type& initialDelay, const time_type& period);

    time_type getTimeout();

    Timer& cancel();

private:

    ISelectable::poll_id    _fd;

};


inline void swap(Timer& lhs, Timer& rhs) noexcept {
    lhs.swap(rhs);
}

}  // End of namespace async
}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_ASYNC_TIMER_HPP
