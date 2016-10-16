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
 * @file: io/sharedMemory.cpp
 *
 *  Created by soultaker on 03/07/16.
*******************************************************************************/
#include <solace/io/sharedMemory.hpp>


#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <unistd.h>


using Solace::Path;
using Solace::IO::MappedMemoryView;
using Solace::IO::ISelectable;
using Solace::IO::File;
using Solace::IO::SharedMemory;

using Solace::IllegalArgumentException;
using Solace::IO::IOException;



SharedMemory::SharedMemory(const poll_id fd) noexcept : _fd(fd)
{

}


SharedMemory::SharedMemory(SharedMemory&& other): _fd(other._fd) {
    other.invalidateFd();

    // Note: It's ok if we have moved content from a closed file to have this->_fd == InvalidFd
}


SharedMemory::~SharedMemory() {
    // FIXME: This can throw! Is there any way to avoid it?

    if (isOpened()) {
        close();
    }
}

SharedMemory::poll_id SharedMemory::validateFd() const {
    if (!isOpened()) {
        raise<NotOpen>();
    }

    return _fd;
}


bool SharedMemory::isOpened() const {
    return !isClosed();
}


bool SharedMemory::isClosed() const {
    return (_fd == File::InvalidFd);
}




void SharedMemory::close() {
    const auto fd = validateFd();
    const auto result = ::close(fd);

    if (result) {
        raise<IOException>(errno, "close");
    }

    invalidateFd();
}


SharedMemory::size_type SharedMemory::size() const {
    const auto fd = validateFd();

    struct stat sb;
    if (fstat(fd, &sb )) {
        raise<IOException>(errno, "fstat");
    }

    return sb.st_size;
}


SharedMemory::poll_id SharedMemory::invalidateFd() {
    auto oldFd = _fd;

    _fd = File::InvalidFd;

    return oldFd;
}


SharedMemory SharedMemory::fromFd(poll_id fid) {
    return { fid };
}


SharedMemory SharedMemory::create(const Path& pathname, size_type memSize, File::AccessMode mode, int permissionsMode) {

    if (memSize == 0) {
        raise<IOException>("Invalid size");
    }

    int oflags = 0;
    switch (mode) {
    case File::AccessMode::ReadOnly:
        oflags = O_RDONLY;
        break;
    case File::AccessMode::WriteOnly:
        oflags = O_WRONLY;
        break;
    case File::AccessMode::ReadWrite:
        oflags = O_RDWR;
        break;
    }

    mode_t omode = permissionsMode;

    const auto& pathString = pathname.toString();
    auto fd = shm_open(pathString.c_str(), O_CREAT | oflags, omode);

    if (fd == -1) {
        raise<IOException>(errno, "shm_open");
    }

    if (ftruncate(fd, memSize) == -1) {
        raise<IOException>(errno, "ftruncate");
    }

    return { fd };
}


SharedMemory SharedMemory::open(const Path& pathname, File::AccessMode mode) {
    int oflags = 0;
    switch (mode) {
    case File::AccessMode::ReadOnly:
        oflags = O_RDONLY;
        break;
    case File::AccessMode::WriteOnly:
        oflags = O_WRONLY;
        break;
    case File::AccessMode::ReadWrite:
        oflags = O_RDWR;
        break;
    }

    const auto& pathString = pathname.toString();
    auto fd = shm_open(pathString.c_str(), oflags, 0);

    if (fd == -1) {
        raise<IOException>(errno, "shm_open");
    }

    return { fd };
}


void SharedMemory::unlink(const Path& pathname) {
    const auto& pathString = pathname.toString();

    if (shm_unlink(pathString.c_str())) {
        raise<IOException>(errno, "shm_unlink");
    }
}


MappedMemoryView
SharedMemory::map(MappedMemoryView::Access mapping, int access, size_type mapSize) {
    const auto fd = validateFd();

    if (mapSize == 0) {
        mapSize = size();
    }

    return MappedMemoryView::map(fd, mapSize, mapping, access);
}

