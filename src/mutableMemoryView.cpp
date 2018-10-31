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
    return *dataAddress(index);
}


MutableMemoryView::value_type*
MutableMemoryView::dataAddress(size_type offset) {
    return const_cast<value_type*>(MemoryView::dataAddress(offset));
}


void
MutableMemoryView::write(const MemoryView& source, size_type offset) {
    auto const thisSize = size();
    auto const srcSize = source.size();

    if (offset > thisSize) {  // Make sure that offset is within [0, size())
        raise<IndexOutOfRangeException>("offset", offset, 0, thisSize);
    }

    if (srcSize > thisSize - offset) {  // Make sure that source is writing no more then there is room.
        raise<OverflowException>("source", srcSize, 0, thisSize - offset);
    }

    if (srcSize > 0) {
        memmove(dataAddress(offset), source.dataAddress(), srcSize);
    }

    // TODO(abbyssoul): return Result<>;
}


void
MutableMemoryView::read(MutableMemoryView& dest) {
    auto const thisSize = size();
    auto const destSize = dest.size();

    if (thisSize < destSize) {
        raise<OverflowException>("dest", thisSize, 0, destSize);
    }

    memmove(dest.dataAddress(), dataAddress(), destSize);
    // TODO(abbyssoul): return Result<>;
}


void
MutableMemoryView::read(MutableMemoryView& dest, size_type bytesToRead, size_type offset) {
    auto const thisSize = size();

    // Check if there is enough room in the dest to hold `bytesToRead` bytes.
    assertIndexInRange(bytesToRead, 0, dest.size() + 1, "dest.size()");
    // Make sure that offset is within [0, size())
    assertIndexInRange(offset, 0, thisSize, "offset");
    // Make sure there is enough data in this buffer to read
    assertIndexInRange(offset + bytesToRead, 0, thisSize + 1, "bytesToRead");

    memmove(dest.dataAddress(), dataAddress(offset), bytesToRead);

    // TODO(abbyssoul): return Result<>;
}


MutableMemoryView&
MutableMemoryView::fill(byte value) noexcept {
    memset(dataAddress(), value, size());

    return (*this);
}


MutableMemoryView&
MutableMemoryView::fill(byte value, size_type from, size_type to) {
    from = assertIndexInRange(from, 0, size(), "from");
    to = assertIndexInRange(to, from, size() + 1, "to");

    memset(dataAddress(from), value, to - from);

    return (*this);
}


MutableMemoryView&
MutableMemoryView::lock() {
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
MutableMemoryView::slice(size_type from, size_type to) noexcept {
    auto const thisSize = size();

    // `from` is constrained to [0, size())
    // `to` is constrained to [from, size())
    from = std::min(from, thisSize);
    to = std::min(thisSize, std::max(to, from));
    size_type const newSize = to - from;

//    _data + from
    return {dataAddress(from), newSize};
}
