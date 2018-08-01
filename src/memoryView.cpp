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
#include "solace/memoryView.hpp"
#include "solace/exception.hpp"

#include "solace/base16.hpp"    //  For operator<<

#include <cstring>  // memcpy
#include <utility>

#include <sys/mman.h>


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
    if (index >= size()) {
        raise<IndexOutOfRangeException>("index", index, 0, size());
    }

    return _dataAddress[index];
}


const MemoryView::value_type*
MemoryView::dataAddress(size_type offset) const {
    if ((offset != 0) && (offset > size())) {
        raise<IndexOutOfRangeException>("offset", offset, 0, size());
    }

    return _dataAddress + offset;
}


MemoryView
MemoryView::slice(size_type from, size_type to) const {
    if (to < from) {
        raise<IndexOutOfRangeException>("from", from, 0, to + 1);
    }

    if (to > size()) {
        raise<IndexOutOfRangeException>("to", to, from, size());
    }

    if ((from != to) && (from >= size())) {
        raise<IndexOutOfRangeException>("from", from, 0, size());
    }


    return wrapMemory(dataAddress(from), to - from);
}


MemoryView
MemoryView::viewImmutableShallow() const {
    return wrapMemory(dataAddress(), size());
}


std::ostream& Solace::operator<<(std::ostream& ostr, MemoryView const& view) {
    if (view.size() > 0) {
        // We use custom output printing each byte as \0 bytest and \n are not printable otherwise.
        auto i = base16Encode_begin(view);
        auto end = base16Encode_end(view);
        for (; i != end; ++i) {
            ostr << *i;
        }
    } else {
        ostr << "<null>";
    }

    return ostr;
}
