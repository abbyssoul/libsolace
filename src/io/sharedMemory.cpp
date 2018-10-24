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
#include <solace/exception.hpp>


#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <unistd.h>


using namespace Solace;
using namespace Solace::IO;

#ifndef NAME_MAX
static constexpr size_t NAME_MAX = 255;
#endif


const int SharedMemory::Protection::None = PROT_NONE;
const int SharedMemory::Protection::Read = PROT_READ;
const int SharedMemory::Protection::Write = PROT_WRITE;
const int SharedMemory::Protection::Exec = PROT_EXEC;


namespace {

MemoryView::MemoryAddress
mapMemory(SharedMemory::size_type memSize, int protection, SharedMemory::Access mapping, int fd) {
    if (memSize == 0) {
        Solace::raise<IllegalArgumentException>("size");
    }

    int flags = (fd == -1) ? MAP_ANONYMOUS : 0;
    switch (mapping) {
    case SharedMemory::Access::Private: flags |= MAP_PRIVATE; break;
    case SharedMemory::Access::Shared: flags |= MAP_SHARED; break;
    }

    auto addr = mmap(nullptr, memSize, protection, flags, fd, 0);
    if (addr == MAP_FAILED) {
        Solace::raise<IOException>(errno, "mmap");
    }

    return addr;
}



/**
 * Memory disposer to unmap mapped memory.
 */
class MappedMemoryDisposer :
        public MemoryResource::Disposer {
public:

    void dispose(MemoryView* view) const override {
        // FIXME(abbyssoul): Some return result check might help.
        auto const size = view->size();
        auto addr = const_cast<MemoryView::value_type*>(view->dataAddress());
        if (addr && size > 0) {
            auto const result = munmap(addr, size);
            if (result != 0) {
                raise<IOException>(errno, "munmap");
            }
        }
    }
};

static MappedMemoryDisposer g_mappedMemoryDisposer;

}  // namespace

SharedMemory::~SharedMemory() {
    if ((_fd != File::InvalidFd)) {
        ::close(_fd);
        _fd = File::InvalidFd;

        if (_linkedPath) {
            // Unlinking quietly - if the file has been unlinked alredy - we don't care.
            auto const pathString = _linkedPath.toString();
            auto const pathView = pathString.view();
            auto const pathCString = std::string{pathView.data(), pathView.size()};
            shm_unlink(pathCString.c_str());
        }
    }
}


SharedMemory::SharedMemory() noexcept :
    SharedMemory(File::InvalidFd)
{
}


SharedMemory::SharedMemory(poll_id fd) noexcept :
    _fd(fd),
    _linkedPath()
{
}


SharedMemory::SharedMemory(poll_id fd, Path&& path) noexcept :
    _fd(fd),
    _linkedPath(std::move(path))
{

}


SharedMemory::SharedMemory(SharedMemory&& other) noexcept
    : _fd(other._fd)
    , _linkedPath(std::move(other._linkedPath))
{
    other.invalidateFd();

    // Note: It's ok if we have moved content from a closed file to have this->_fd == InvalidFd
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
    auto const fd = validateFd();
    auto const result = ::close(fd);

    if (result) {
        raise<IOException>(errno, "close");
    }

    invalidateFd();
}


SharedMemory::size_type
SharedMemory::size() const {
    auto const fd = validateFd();

    struct stat sb;
    if (fstat(fd, &sb )) {
        raise<IOException>(errno, "fstat");
    }

    if (sb.st_size < 0) {
        raise<IOException>(errno, "fstat: negative size");
    }

    return static_cast<size_type>(sb.st_size);
}


SharedMemory::poll_id SharedMemory::invalidateFd() {
    auto oldFd = _fd;

    _fd = File::InvalidFd;

    return oldFd;
}


SharedMemory SharedMemory::fromFd(poll_id fid) {
    return { fid };
}


MemoryResource
SharedMemory::mapMem(int fd, size_type memSize, SharedMemory::Access mapping, int protection) {
    auto addr = mapMemory(memSize, protection, mapping, fd);

    return {wrapMemory(addr, memSize), &g_mappedMemoryDisposer};
}


SharedMemory
SharedMemory::create(Path&& pathname, size_type memSize, File::AccessMode mode, int permissionsMode) {

    if (memSize == 0) {
        raise<IOException>("Invalid size");
    }

    if (pathname.length() >= NAME_MAX) {
        raise<IOException>("Name too long");
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

    auto const pathString = pathname.toString();
    auto const pathView = pathString.view();
    auto const pathCString = std::string{pathView.data(), pathView.size()};
    auto fd = shm_open(pathCString.c_str(), O_CREAT | oflags, omode);

    if (fd == -1) {
        raise<IOException>(errno, "shm_open");
    }

    if (ftruncate(fd, memSize) == -1) {
        raise<IOException>(errno, "ftruncate");
    }

    return SharedMemory{ fd, std::move(pathname) };
}


SharedMemory
SharedMemory::open(const Path& pathname, File::AccessMode mode) {
    if (pathname.length() >= NAME_MAX) {
        raise<IOException>("Name too long");
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

    auto const pathString = pathname.toString();
    auto const pathView = pathString.view();
    auto const pathCString = std::string{pathView.data(), pathView.size()};
    auto fd = shm_open(pathCString.c_str(), oflags, 0);

    if (fd == -1) {
        raise<IOException>(errno, "shm_open");
    }

    return SharedMemory{ fd };
}


void SharedMemory::unlink(Path const& pathname) {
    auto const pathString = pathname.toString();
    auto const pathView = pathString.view();
    auto const pathCString = std::string{pathView.data(), pathView.size()};

    if (shm_unlink(pathCString.c_str())) {
        raise<IOException>(errno, "shm_unlink");
    }
}


MemoryResource SharedMemory::map(SharedMemory::Access mapping, int access, size_type mapSize) {
    auto const fd = validateFd();

    if (mapSize == 0) {
        mapSize = size();
    }

    return mapMem(fd, mapSize, mapping, access);
}

