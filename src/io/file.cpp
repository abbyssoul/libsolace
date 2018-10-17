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
#include <solace/exception.hpp>

#include <memory>   // std::unique_ptr<>
#include <cstring>  // memcpy (should review)

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>


using Solace::IOObject;
using Solace::String;
using Solace::Path;
using Solace::MutableMemoryView;

using Solace::IO::ISelectable;
using Solace::IO::File;

using Solace::IllegalArgumentException;
using Solace::IOException;


const ISelectable::poll_id ISelectable::InvalidFd = -1;


const int File::Flags::Append = O_APPEND;
const int File::Flags::Async = O_ASYNC;
const int File::Flags::CloseExec = O_CLOEXEC;

#ifdef SOLACE_PLATFORM_LINUX
const int File::Flags::DSync = O_DSYNC;
#endif

#ifndef SOLACE_PLATFORM_APPLE
const int File::Flags::Direct = O_DIRECT;
#endif

const int File::Flags::Directory = O_DIRECTORY;
const int File::Flags::Exlusive = O_EXCL;
const int File::Flags::NoCTTY = O_NOCTTY;
const int File::Flags::NonBlock = O_NONBLOCK;
const int File::Flags::Sync = O_SYNC;
const int File::Flags::Trunc = O_TRUNC;


const int File::Mode::IRWXU = S_IRWXU;
const int File::Mode::IRUSR = S_IRUSR;
const int File::Mode::IWUSR = S_IWUSR;
const int File::Mode::IXUSR = S_IXUSR;

const int File::Mode::IRWXG = S_IRWXG;
const int File::Mode::IRGRP = S_IRGRP;
const int File::Mode::IWGRP = S_IWGRP;
const int File::Mode::IXGRP = S_IXGRP;

const int File::Mode::IRWXO = S_IRWXO;
const int File::Mode::IROTH = S_IROTH;
const int File::Mode::IWOTH = S_IWOTH;
const int File::Mode::IXOTH = S_IXOTH;


File::File() : File(ISelectable::InvalidFd) {

}

File::File(const poll_id fd) noexcept : _fd(fd)
{

}

File::File(const Path& path, int flags) {

    // TODO(abbyssoul): Should the path be validated?
    const auto pathStr = path.toString();
    const auto pathStrView = pathStr.view();
    const auto pathCStr = std::string(pathStrView.data(), pathStrView.size());

    bool notDone = true;

    // FIXME(abbyssoul): All loops must have upper bound
    while (notDone) {
        _fd = open(pathCStr.c_str(), flags);

        if ((notDone = (_fd == -1))) {
            switch (errno) {
            case EINTR:  // Recoverable error, lest try again!
                break;
            case ENFILE:
            case EMFILE:
                raise<IOException>("Too many opened file descriptors.");
                break;
            default:
                raise<IOException>(errno);
            }
        }
    }
}


File::File(File&& other): _fd(other._fd) {
    other.invalidateFd();

    // Note: It's ok if we have moved content from a closed file to have this->_fd == InvalidFd
}


File::~File() {
    // FIXME: This can throw! Is there any way to avoid it?

    if (isOpened()) {
        close();
    }
}

File::poll_id File::validateFd() const {
    if (!isOpened()) {
        raise<NotOpen>();
    }

    return _fd;
}


bool File::isOpened() const {
    return (_fd != InvalidFd);
}


IOObject::IOResult
File::read(MutableMemoryView& buffer) {
    const auto fd = validateFd();
    const auto bytesRead = ::read(fd, buffer.dataAddress(), buffer.size());

    if (bytesRead < 0) {
        raise<IOException>(errno);
    }

    return IOObject::IOResult(bytesRead);
}


IOObject::IOResult
File::write(const Solace::MemoryView& buffer) {
    const auto fd = validateFd();
    const auto bytesWritten = ::write(fd, buffer.dataAddress(), buffer.size());

    if (bytesWritten < 0) {
        raise<IOException>(errno);
    }

    return IOObject::IOResult(bytesWritten);
}



File::size_type File::seek(size_type offset, Seek type) {
    const auto fd = validateFd();

    off_t result = 0;
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
    if (isClosed())
        return;

    const auto fd = validateFd();
    const auto result = ::close(fd);

    if (result) {
        raise<IOException>(errno);
    }

    invalidateFd();
}


void File::flush() {
    const auto fd = validateFd();
    const auto result = ::fsync(fd);

    if (result) {
        raise<IOException>(errno);
    }
}


File::poll_id File::invalidateFd() {
    auto oldFd = _fd;

    _fd = InvalidFd;

    return oldFd;
}


File File::fromFd(poll_id fid) {
    return { fid };
}
