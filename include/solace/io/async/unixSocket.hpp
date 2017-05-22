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
 *	@file		solace/io/async/pipe.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IO_ASYNC_UNIXSOCKET_HPP
#define SOLACE_IO_ASYNC_UNIXSOCKET_HPP

#include "solace/io/selectable.hpp"
#include "solace/io/async/future.hpp"
#include "solace/io/async/channel.hpp"


namespace Solace { namespace IO { namespace async {

class UnixSocket: public Channel {
public:

    ~UnixSocket();

    UnixSocket(EventLoop& ioContext);

    UnixSocket(UnixSocket&& rhs);

    UnixSocket& operator= (UnixSocket&& rhs) noexcept {
        return swap(rhs);
    }

    UnixSocket& UnixSocket(UnixSocket& rhs) noexcept {
        using std::swap;

        Channel::swap(rhs);
        swap(_duplex, rhs._duplex);

        return *this;
    }

    async::Future<void>& asyncRead(Solace::ByteBuffer& buffer);
    async::Future<void>& asyncWrite(Solace::ByteBuffer& buffer);

private:

    ISelectable::poll_id _fd;
};


inline void swap(UnixSocket& lhs, UnixSocket& rhs) noexcept {
    lhs.swap(rhs);
}

}  // End of namespace async
}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_ASYNC_UNIXSOCKET_HPP
