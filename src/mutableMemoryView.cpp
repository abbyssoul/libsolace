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
 *	@file		mutableMemoryView.cpp
 *	@brief		Implementation of Mutable MemoryView
 ******************************************************************************/
#include "solace/mutableMemoryView.hpp"
#include "solace/exception.hpp"


#include <cstring>  // memcpy
#include <utility>

#include <sys/mman.h>


using namespace Solace;


MutableMemoryView::reference
MutableMemoryView::operator[] (size_type index) {
    return *const_cast<value_type*>(dataAddress(index));
}


MutableMemoryView::value_type*
MutableMemoryView::dataAddress(size_type offset) {
    return const_cast<value_type*>(MemoryView::dataAddress(offset));
}


MutableMemoryView::value_type*
MutableMemoryView::dataAddress() {
    return const_cast<value_type*>(MemoryView::dataAddress());
}


MutableMemoryView&
MutableMemoryView::fill(byte value, size_type from, size_type to) {
    if (from >= size()) {
        raise<IndexOutOfRangeException>("from", from, 0, size());
    }

    if (to < from) {
        raise<IndexOutOfRangeException>("to", to, from, size());
    }

    if (to > size()) {
        raise<IndexOutOfRangeException>("to", to, from, size());
    }

    memset(const_cast<value_type*>(dataAddress(from)), value, to - from);

    return (*this);
}


void MutableMemoryView::write(const MemoryView& source, size_type offset) {
    const auto thisSize = size();

    if (offset > thisSize) {  // Make sure that offset is within [0, size())
        raise<IndexOutOfRangeException>("offset", offset, 0, thisSize);
    }

    if (source.size() > thisSize - offset) {  // Make sure that source is writing no more then there is room.
        raise<OverflowException>("source", source.size(), 0, thisSize - offset);
    }

    memmove(const_cast<value_type*>(dataAddress(offset)), source.dataAddress(), source.size());

    // TODO(abbyssoul): return Result<>;
}


void MutableMemoryView::read(MutableMemoryView& dest) {
    const auto thisSize = size();

    if (thisSize < dest.size()) {
        raise<OverflowException>("dest", thisSize, 0, dest.size());
    }

    memmove(const_cast<value_type*>(dest.dataAddress()), dataAddress(), dest.size());
    // TODO(abbyssoul): return Result<>;
}


void MutableMemoryView::read(MutableMemoryView& dest, size_type bytesToRead, size_type offset) {
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

    memmove(const_cast<value_type*>(dest.dataAddress()), dataAddress(offset), bytesToRead);

    // TODO(abbyssoul): return Result<>;
}


MutableMemoryView& MutableMemoryView::fill(byte value) {
    memset(const_cast<value_type*>(dataAddress()), value, size());

    return (*this);
}


MutableMemoryView& MutableMemoryView::lock() {
    if (mlock(dataAddress(), size()) < 0) {
        // TODO(abbyssoul): shold use ErrnoException
        raise<Exception>("failed to lock the memory buffer");
    }

    return (*this);
}


MutableMemoryView&
MutableMemoryView::unlock() {
    if (munlock(dataAddress(), size()) < 0) {
        // TODO(abbyssoul): shold use ErrnoException
        raise<Exception>("failed to unlock the memory buffer");
    }

    return (*this);
}


MutableMemoryView
MutableMemoryView::slice(size_type from, size_type to) {
    if (to < from) {
        raise<IndexOutOfRangeException>("from", from, 0, to + 1);
    }

    if (to > size()) {
        raise<IndexOutOfRangeException>("to", to, from, size());
    }

    if ((from != to) && (from >= size())) {
        raise<IndexOutOfRangeException>("from", from, 0, size());
    }


    return wrapMemory(const_cast<value_type*>(dataAddress(from)), to - from);
}
