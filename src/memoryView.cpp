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
 *	@brief		Implementation of ImmutableMemoryView
 *	ID:			$Id$
 ******************************************************************************/
#include "solace/memoryView.hpp"
#include "solace/exception.hpp"



using namespace Solace;


constexpr int kOne = 1;

bool Solace::isBigendian() noexcept {
    return *reinterpret_cast<const char*>(&kOne) == 0;
}


MemoryView::MemoryView(const void* data, size_type newSize) :
    _size(newSize),
    _dataAddress(reinterpret_cast<const value_type*>(data))
{
    if (!_dataAddress && _size) {
        raise<IllegalArgumentException>("data");
    }
}


MemoryView::value_type
MemoryView::operator[] (size_type index) const {
    assertIndexInRange(index, 0, _size, "index");

    return _dataAddress[index];
}


const MemoryView::value_type*
MemoryView::dataAddress(size_type offset) const {
    if (offset != 0) {
        assertIndexInRange(offset, 0, _size + 1, "offset");
    }

    return _dataAddress + offset;
}


MemoryView
MemoryView::slice(size_type from, size_type to) const noexcept {
    auto const thisSize = size();

    from = std::min(from, thisSize);
    to = std::min(thisSize, std::max(to, from));
    size_type const newSize = to - from;
//    size_type const maxSize = thisSize - from;

    return {_dataAddress + from, newSize};
}
