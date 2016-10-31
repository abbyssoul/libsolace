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
 *	@file		memoryView.cpp
 *	@author		$LastChangedBy: soultaker $
 *	@date		$LastChangedDate$
 *	@brief		Implementation of MemoryView
 *	ID:			$Id$
 ******************************************************************************/
#include "solace/memoryView.hpp"
#include "solace/exception.hpp"


#include <cstring>  // memcpy
#include <utility>

#include <sys/mman.h>


using Solace::byte;
using Solace::MemoryView;


MemoryView::MemoryView() noexcept :
    _size(),
    _dataAddress(nullptr),
    _free()
{
}


MemoryView::MemoryView(MemoryView&& rhs) noexcept :
            _size(rhs._size),
            _dataAddress(rhs._dataAddress),
            _free(std::move(rhs._free))
{
    // Stuff up rhs so it won't destruct anything
    rhs._size = 0;
    rhs._dataAddress = nullptr;
    rhs._free = nullptr;
}


MemoryView::MemoryView(size_type newSize, void* data, const std::function<void(MemoryView*)>& freeFunc) :
    _size(newSize),
    _dataAddress(reinterpret_cast<value_type*>(data)),
    _free(freeFunc)
{
    if (!_dataAddress && _size) {
        raise<IllegalArgumentException>("data");
    }
}



MemoryView::~MemoryView() {
    if (_free) {
        _free(this);

        _dataAddress = nullptr;
        _size = 0;
    }
}

MemoryView::reference MemoryView::operator[] (size_type index) {
    if (index >= size()) {
        raise<IndexOutOfRangeException>("index", index, 0, size());
    }

    return _dataAddress[index];
}

MemoryView::value_type MemoryView::operator[] (size_type index) const {
    if (index >= size()) {
        raise<IndexOutOfRangeException>("index", index, 0, size());
    }

    return _dataAddress[index];
}


byte* MemoryView::dataAddress(size_type offset) const {
    if (offset >= size()) {
        raise<IndexOutOfRangeException>("offset", offset, 0, size());
    }

    return _dataAddress + offset;
}


MemoryView& MemoryView::swap(MemoryView& rhs) noexcept {
    std::swap(_size, rhs._size);
    std::swap(_dataAddress, rhs._dataAddress);
    std::swap(_free, rhs._free);

    return (*this);
}


MemoryView MemoryView::slice(size_type from, size_type to) const {
    if (from >= size()) {
        raise<IndexOutOfRangeException>("from", from, 0, size());
    }

    if (to < from) {
        raise<IndexOutOfRangeException>("to", to, from, size());
    }

    if (to > size()) {
        raise<IndexOutOfRangeException>("to", to, from, size());
    }

    return wrapMemory(dataAddress(from), to - from);
}


MemoryView& MemoryView::fill(byte value, size_type from, size_type to) {
    if (from >= size()) {
        raise<IndexOutOfRangeException>("from", from, 0, size());
    }

    if (to < from) {
        raise<IndexOutOfRangeException>("to", to, from, size());
    }

    if (to > size()) {
        raise<IndexOutOfRangeException>("to", to, from, size());
    }

    memset(dataAddress(from), value, to - from);

    return (*this);
}


void MemoryView::write(const MemoryView& source, size_type offset) {
    const auto thisSize = size();

    if (offset > thisSize) {  // Make sure that offset is within [0, size())
        raise<IndexOutOfRangeException>("offset", offset, 0, thisSize);
    }

    if (source.size() >= thisSize - offset) {  // Make sure that source is writing no more then there is room.
        raise<OverflowException>("source", source.size(), 0, thisSize - offset);
    }

    memcpy(dataAddress(offset), source.dataAddress(), source.size());

    // TODO(abbyssoul): return Result<>;
}


void MemoryView::read(MemoryView& dest) {
    const auto thisSize = size();

    if (thisSize < dest.size()) {
        raise<OverflowException>("dest", thisSize, 0, dest.size());
    }

    memcpy(dest.dataAddress(), dataAddress(), dest.size());
    // TODO(abbyssoul): return Result<>;
}


void MemoryView::read(MemoryView& dest, size_type bytesToRead, size_type offset) {
    const auto thisSize = size();

    if (thisSize < offset) {  // Make sure that offset is within [0, size())
        raise<IndexOutOfRangeException>("offset", offset, 0, thisSize);
    }

    if (bytesToRead > thisSize - offset) {  // Make sure that bytes to read is within [offset, size())
        raise<IndexOutOfRangeException>("bytesToRead", offset, 0, thisSize - offset);
    }

    if (dest.size() < bytesToRead) {  // Make sure that dest has enough space to store data
        raise<OverflowException>("dest.size()", dest.size(), 0, bytesToRead);
    }

    memcpy(dest.dataAddress(), dataAddress(offset), bytesToRead);

    // TODO(abbyssoul): return Result<>;
}


MemoryView& MemoryView::fill(byte value) {
    memset(_dataAddress, value, _size);

    return (*this);
}


MemoryView& MemoryView::lock() {

    if (mlock(dataAddress(), size()) < 0) {
        // TODO(abbyssoul): shold use ErrnoException
        raise<Exception>("failed to lock memory");
    }

    return (*this);
}


MemoryView& MemoryView::unlock() {
    if (munlock(dataAddress(), size()) < 0) {
        // TODO(abbyssoul): shold use ErrnoException
        raise<Exception>("failed to lock memory");
    }

    return (*this);
}


MemoryView MemoryView::viewShallow() const {
    return wrapMemory(dataAddress(), size());
}
