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
 *	@brief		Implementation of ImmutableMemoryView
 ******************************************************************************/
#include "solace/memoryView.hpp"
#include "solace/assert.hpp"

#include "solace/error.hpp"
#include "solace/posixErrorDomain.hpp"

#include <cstring>		// memcmp
#include <algorithm>    // std::min/max
#include <sys/mman.h>   // mlock/munlock


using namespace Solace;


constexpr int kOne = 1;

bool Solace::isBigendian() noexcept {
    return *reinterpret_cast<const char*>(&kOne) == 0;
}


MemoryView::value_type
MemoryView::operator[] (size_type index) const {
    assertIndexInRange(index, 0, _size, "index");

	return begin()[index];
}


Optional<MemoryView::MemoryAddress>
MemoryView::dataAddress(size_type offset) const noexcept {
	if (offset != 0 && offset > _size) {
		return none;
    }

	return begin() + offset;
}


MemoryView
MemoryView::slice(size_type from, size_type to) const noexcept {
	auto const thisSize = size();

    from = std::min(from, thisSize);
    to = std::min(thisSize, std::max(to, from));
    size_type const newSize = to - from;

	return {begin() + from, newSize};
}


Result<MemoryView::Lock, Error>
MemoryView::lock() {
    if (mlock(dataAddress(), size()) < 0) {
		return makeErrno("mlock");
    }

	return {types::okTag, in_place, *this};
}


MemoryView::Lock::~Lock() {
    munlock(_lockedMem.dataAddress(), _lockedMem.size());
	// NOTE(abbyssoul): What can we do if we failed to unlock?
//    if ( res < 0) {
//        // TODO(abbyssoul): shold use ErrnoException
//        raise<Exception>("failed to unlock the memory buffer");
//    }
}


bool MemoryView::equals(MemoryView const& other) const noexcept {
	if (_size != other._size) {
		return false;
	}

	if ((&other == this) ||
		(_dataAddress == other._dataAddress)) {
		return true;
	}

	return (std::memcmp(_dataAddress, other._dataAddress, _size) == 0);
}

