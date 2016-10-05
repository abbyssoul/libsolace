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
 *	@file		solace/io/async/channel.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IO_ASYNC_CHANNEL_HPP
#define SOLACE_IO_ASYNC_CHANNEL_HPP


#include "solace/io/async/eventloop.hpp"


namespace Solace { namespace IO { namespace async {


/**
 * Channel is the base of the io objects.
 * Subclasses of this class provide access to the real IO subsystems
 */
class Channel {
public:
    Channel(EventLoop& ioContext) :
        _ioContext(&ioContext)
    {
    }

    Channel(Channel&& rhs) :
        _ioContext(&rhs.getIOContext())
    {}

    virtual ~Channel() = default;

    Channel& operator = (Channel&& rhs) noexcept {
//        _ioContext = std::move(rhs._ioContext);
        _ioContext = rhs._ioContext;
//        rhs._ioContext = nullptr;

        return *this;
    }

    EventLoop& getIOContext() noexcept {
        return *_ioContext;
    }

    const EventLoop& getIOContext() const noexcept {
        return *_ioContext;
    }

private:

    Solace::IO::async::EventLoop*   _ioContext;
};

}  // End of namespace async
}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_ASYNC_CHANNEL_HPP
