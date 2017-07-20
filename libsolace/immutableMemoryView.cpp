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
 *	@file		immutableMemoryView.cpp
 *	@author		$LastChangedBy: soultaker $
 *	@date		$LastChangedDate$
 *	@brief		Implementation of ImmutableMemoryView
 *	ID:			$Id$
 ******************************************************************************/
#include "solace/immutableMemoryView.hpp"
#include "solace/exception.hpp"


#include <cstring>  // memcpy
#include <utility>

#include <sys/mman.h>


using Solace::byte;
using Solace::ImmutableMemoryView;


ImmutableMemoryView::ImmutableMemoryView() noexcept :
    _size(),
    _dataAddress(nullptr),
    _free()
{
}


ImmutableMemoryView::ImmutableMemoryView(ImmutableMemoryView&& rhs) noexcept :
            _size(rhs._size),
            _dataAddress(rhs._dataAddress),
            _free(std::move(rhs._free))
{
    // Stuff up rhs so it won't destruct anything
    rhs._size = 0;
    rhs._dataAddress = nullptr;
    rhs._free = nullptr;
}


ImmutableMemoryView::ImmutableMemoryView(size_type newSize,
                                         const void* data,
                                         const std::function<void(ImmutableMemoryView*)>& freeFunc) :
    _size(newSize),
    _dataAddress(reinterpret_cast<const value_type*>(data)),
    _free(freeFunc)
{
    if (!_dataAddress && _size) {
        raise<IllegalArgumentException>("data");
    }
}



ImmutableMemoryView::~ImmutableMemoryView() {
    if (_free) {
        _free(this);

        _dataAddress = nullptr;
        _size = 0;
    }
}


ImmutableMemoryView::value_type
ImmutableMemoryView::operator[] (size_type index) const {
    if (index >= size()) {
        raise<IndexOutOfRangeException>("index", index, 0, size());
    }

    return _dataAddress[index];
}


const ImmutableMemoryView::value_type*
ImmutableMemoryView::dataAddress(size_type offset) const {
    if (offset >= size()) {
        raise<IndexOutOfRangeException>("offset", offset, 0, size());
    }

    return _dataAddress + offset;
}


ImmutableMemoryView
ImmutableMemoryView::slice(size_type from, size_type to) const {
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


ImmutableMemoryView ImmutableMemoryView::viewShallow() const {
    return wrapMemory(dataAddress(), size());
}
