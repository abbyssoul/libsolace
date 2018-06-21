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
 * libSolace
 *	@file		memoryManger.cpp
 *	@author		$LastChangedBy: soultaker $
 *	@date		$LastChangedDate$
 *	@brief		Implementation of MemoryManger
 *	ID:			$Id$
 ******************************************************************************/
#include "solace/memoryManager.hpp"
#include "solace/exception.hpp"


#include <cstring>  // memcpy
#include <unistd.h>
#include <cerrno>


using namespace Solace;


MemoryManager::MemoryManager(size_type allowedCapacity) :
    _capacity(allowedCapacity),
    _size(0),
    _isLocked(false),
    _disposer(this)
{

    const auto totalAvaliableMemory = getPageSize() * getNbPages();
    if (totalAvaliableMemory < _capacity) {
        Solace::raise<IllegalArgumentException>("allowedCapacity can't be more then total system's memory");
    }
}


MemoryManager& MemoryManager::swap(MemoryManager& rhs) noexcept {
    using std::swap;

    swap(_capacity, rhs._capacity);
    swap(_size, rhs._size);
    swap(_isLocked, rhs._isLocked);

    return (*this);
}


MemoryManager& MemoryManager::operator= (MemoryManager&& rhs) noexcept {
    return swap(rhs);
}


bool MemoryManager::empty() const noexcept {
    return (_size == 0);
}


MemoryManager::size_type MemoryManager::size() const noexcept {
    return _size;
}

MemoryManager::size_type MemoryManager::capacity() const noexcept {
    return _capacity;
}


MemoryManager::size_type MemoryManager::getPageSize() const {
//    const auto res = sysconf(PAGESIZE);
    return getpagesize();
}


MemoryManager::size_type MemoryManager::getNbPages() const {
    const auto res = sysconf(_SC_PHYS_PAGES);
    if (res < 0) {
        Solace::raise<IOException>(errno, "sysconf(_SC_PHYS_PAGES)");
    }

    // By now it is safe to cast to unsigned type as we have checked for negative values above.
    return static_cast<MemoryManager::size_type>(res);
}

MemoryManager::size_type MemoryManager::getNbAvailablePages() const {
#ifdef SOLACE_PLATFORM_LINUX
  const auto res = sysconf(_SC_AVPHYS_PAGES);
    if (res < 0) {
        Solace::raise<IOException>(errno, "sysconf(_SC_AVPHYS_PAGES)");
    }

    // By now it is safe to cast to unsigned type as we have checked for negative values above.
    return static_cast<MemoryManager::size_type>(res);
#else
    return 0;
#endif
}


void
MemoryManager::HeapMemoryDisposer::dispose(ImmutableMemoryView* view) const {
    _self->free(view);
}


void MemoryManager::free(ImmutableMemoryView* view) {
    const auto size = view->size();
    delete [] view->dataAddress();

    _size -= size;
}


MemoryBuffer MemoryManager::create(size_type dataSize) {
    if (size() + dataSize > capacity()) {
        raise<OverflowException>("dataSize", dataSize, 0, capacity() - size());
    }

    if (isLocked()) {
        raise<Exception>("Cannot allocate memory block: allocator is locked.");
    }

    auto data = new MemoryView::value_type[dataSize];

    _size += dataSize;

    return MemoryBuffer(wrapMemory(data, dataSize), &_disposer);
}


void MemoryManager::lock() {
    _isLocked = true;
}

bool MemoryManager::isLocked() const {
    return _isLocked;
}


void MemoryManager::unlock() {
    _isLocked = false;
}

