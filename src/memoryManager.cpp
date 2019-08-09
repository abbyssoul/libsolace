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
 *	@brief		Implementation of MemoryManger
 ******************************************************************************/
#include "solace/memoryManager.hpp"


#include <unistd.h>
#include <cstdlib>


using namespace Solace;


MemoryManager::MemoryManager(size_type allowedCapacity)
    : _capacity{allowedCapacity}
    , _size{}
    , _isLocked{false}
    , _disposer(*this)
{
    auto const totalAvaliableMemory = getPageSize() * getNbPages();
    assertTrue(_capacity < totalAvaliableMemory, "allowedCapacity can't be more then total system's memory");
}


MemoryManager& MemoryManager::swap(MemoryManager& rhs) noexcept {
    using std::swap;

    swap(_capacity, rhs._capacity);
    swap(_size, rhs._size);
    swap(_isLocked, rhs._isLocked);

    return (*this);
}


MemoryManager::size_type MemoryManager::getPageSize() const {
//    auto const res = sysconf(PAGESIZE);
    return narrow_cast<size_type>(getpagesize());
}


MemoryManager::size_type MemoryManager::getNbPages() const {
    auto const res = sysconf(_SC_PHYS_PAGES);
    assertErrno(res >= 0, "sysconf(_SC_PHYS_PAGES)");

    // By now it is safe to cast to unsigned type as we have checked for negative values above.
    return static_cast<MemoryManager::size_type>(res);
}

MemoryManager::size_type MemoryManager::getNbAvailablePages() const {
#ifdef SOLACE_PLATFORM_LINUX
    auto const res = sysconf(_SC_AVPHYS_PAGES);
    assertErrno(res >= 0, "sysconf(_SC_AVPHYS_PAGES)");

    // By now it is safe to cast to unsigned type as we have checked for negative values above.
    return static_cast<MemoryManager::size_type>(res);
#else
    return 0;
#endif
}


void
MemoryManager::HeapMemoryDisposer::dispose(MemoryView* view) const {
    _self->free(view);
}


void MemoryManager::free(MemoryView* view) {
    auto const size = view->size();
    ::free(reinterpret_cast<void*>(const_cast<MemoryView::value_type*>(view->dataAddress())));

    _size -= size;
}


MemoryResource
MemoryManager::allocate(size_type dataSize) {
    assertIndexInRange(dataSize, 0, limit() + 1, "dataSize");
    assertTrue(!isLocked(), "Cannot allocate memory block: allocator is locked.");

    auto data = ::malloc(dataSize);

    _size += dataSize;

    return {wrapMemory(data, dataSize), &_disposer};
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


MemoryManager&
Solace::getSystemHeapMemoryManager() {
    // FIXME(abbyssoul): Get actual ammount of memory here
    static MemoryManager globalMemoryManager{16*1024*1024};

    return globalMemoryManager;
}
