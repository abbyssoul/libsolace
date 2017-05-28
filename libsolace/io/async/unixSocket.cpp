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
 * @file: io/async/unixSocket.cpp
 *
*******************************************************************************/
#include <solace/io/async/unixSocket.hpp>
#include <solace/exception.hpp>

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>


using namespace Solace;
using namespace Solace::IO;
using namespace Solace::IO::async;


int createNonblockingSocket() {
    const auto r = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (r < 0) {
        Solace::raise<IOException>(errno, "socket");
    }

    return r;
}



UnixSocket::~UnixSocket() {

}

UnixSocket::UnixSocket(UnixSocket&& rhs):
    Channel(std::move(rhs)),
    _fd(std::move(rhs._fd))
{
}


UnixSocket::UnixSocket(EventLoop& ioContext) :
    Channel(ioContext),
    _fd(createNonblockingSocket())
{
}

void UnixSocket::connect(const endpoint_type& endpoint) {
    sockaddr_un remote;
    socklen_t nameBufferSize = sizeof(remote.sun_path);

    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, endpoint.c_str(), nameBufferSize);

    // It's safe size_t (64bit) to socklen_t (32 bit) conversion as len of sun_path is always less then max<size_t>
    const socklen_t len = static_cast<socklen_t>(strlen(remote.sun_path)) + nameBufferSize;
    const auto r = ::connect(_fd, reinterpret_cast<sockaddr*>(&remote), len);
    if (r < 0) {
        Solace::raise<IOException>(errno, "connect");
    }
}
