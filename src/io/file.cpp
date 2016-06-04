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
 * @file: io/file.cpp
 *
 *  Created by soultaker on 27/04/16.
*******************************************************************************/
#include <solace/io/file.hpp>
#include <solace/byteBuffer.hpp>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>


using Solace::Path;
using Solace::ByteBuffer;
using Solace::IO::ISelectable;
using Solace::IO::File;

using Solace::IllegalArgumentException;
using Solace::IO::IOException;


static const ISelectable::poll_id InvalidFd = -1;


File::File(const Path& path, int flags) {

    // TODO(abbyssoul): Should the path be validated?
    const auto pathStr = path.toString();

    bool notDone = true;

    // FIXME(abbyssoul): All loops must have upper bound
    while (notDone) {
        _fd = open(pathStr.c_str(), flags);

        if ((notDone = (_fd == -1))) {
          switch (errno) {
          case EINTR:  // Recoverable error, lest try again!
              break;
          case ENFILE:
          case EMFILE:
            raise<IOException>("Too many opened file descriptors.");
          default:
            raise<IOException>(errno);
          }
        }
    }
}


File::File(File&& other): _fd(other._fd) {
    other._fd = InvalidFd;

    // Note: It's ok if we have moved content from a closed file to have this->_fd == InvalidFd
}


File::~File() {
    // FIXME: This can throw! Is there any way to avoid it?

    if (isOpen()) {
        close();
    }
}

File::poll_id File::validateFd() const {
    if (!isOpen()) {
        raise<NotOpen>();
    }

    return _fd;
}


bool File::isOpen() const {
    return !isClosed();
}


bool File::isClosed() const {
    return (_fd == InvalidFd);
}


File::size_type File::read(ByteBuffer& buffer) {
    return read(buffer, buffer.remaining());
}


File::size_type File::read(ByteBuffer& buffer, ByteBuffer::size_type bytesToRead) {
    if (buffer.remaining() < bytesToRead) {
        raise<IllegalArgumentException>("bytesToRead");
    }

    const auto fd = validateFd();
    const auto bytesRead = ::read(fd, buffer.dataPositiong(), bytesToRead);

    if (bytesRead < 0) {
        raise<IOException>(errno);
    }

    buffer.position(buffer.position() + bytesRead);

    return bytesRead;
}


File::size_type File::write(ByteBuffer& buffer) {
    return write(buffer, buffer.remaining());
}


File::size_type File::write(ByteBuffer& buffer, ByteBuffer::size_type bytesToWrite) {
    if (buffer.remaining() < bytesToWrite) {
        raise<IllegalArgumentException>("bytesToWrite");
    }

    const auto fd = validateFd();
    const auto bytesWritten = ::write(fd, buffer.dataPositiong(), bytesToWrite);
    if (bytesWritten < 0) {
        raise<IOException>(errno);
    }

    buffer.position(buffer.position() + bytesWritten);

    return bytesWritten;
}


File::size_type File::seek(size_type offset, Seek type) {
    const auto fd = validateFd();

    off_t result;
    switch (type) {
        case Seek::Set:     result = lseek(fd, offset, SEEK_SET); break;
        case Seek::Current: result = lseek(fd, offset, SEEK_CUR); break;
        case Seek::End:     result = lseek(fd, offset, SEEK_END); break;
    }

    if (-1 == result) {
        raise<IOException>(errno);
    }

    return result;
}


void File::close() {
    const auto fd = validateFd();
    const auto result = ::close(fd);

    if (result) {
        raise<IOException>(errno);
    }

    _fd = InvalidFd;
}


void File::flush() {
    const auto fd = validateFd();
    const auto result = ::fsync(fd);

    if (result) {
        raise<IOException>(errno);
    }
}

