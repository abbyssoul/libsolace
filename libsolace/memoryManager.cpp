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


using Solace::MemoryView;
using Solace::MemoryManager;

using Solace::IllegalArgumentException;
using Solace::OverflowException;
using Solace::IOException;


MemoryManager::MemoryManager(size_type allowedCapacity) :
    _capacity(allowedCapacity),
    _size(0),
    _isLocked(false)
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

    return res;
}

MemoryManager::size_type MemoryManager::getNbAvailablePages() const {
    const auto res = sysconf(_SC_AVPHYS_PAGES);
    if (res < 0) {
        Solace::raise<IOException>(errno, "sysconf(_SC_AVPHYS_PAGES)");
    }

    return res;
}


void MemoryManager::free(MemoryView* view) {
    delete [] view->dataAddress();
    _size -= view->size();
}


MemoryView MemoryManager::create(size_type dataSize) {
    if (size() + dataSize > capacity()) {
        raise<OverflowException>("dataSize", dataSize, 0, capacity() - size());
    }

    if (isLocked()) {
        raise<Exception>("locked");
    }

    auto data = new MemoryView::value_type[dataSize];

    _size += dataSize;

    return wrapMemory(data, dataSize,
                      [this](MemoryView* view) { this->free(view); });
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

