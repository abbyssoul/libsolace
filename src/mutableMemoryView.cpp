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
#include "solace/posixErrorDomain.hpp"


#include <cstring>  // std::memmove
#include <algorithm>    // std::min/max

using namespace Solace;


MutableMemoryView::reference
MutableMemoryView::operator[] (size_type index) {
	assertIndexInRange(index, size(), "MutableMemoryView[]");

	return begin()[index];
}


Optional<MutableMemoryView::MutableMemoryAddress>
MutableMemoryView::dataAddress(size_type offset) noexcept {
	return MemoryView::dataAddress(offset)
			.map([](MemoryAddress addr) { return const_cast<MutableMemoryAddress>(addr); });
}


Result<void, Error>
MutableMemoryView::write(MemoryView const& source, size_type offset) {
	auto const thisSize = size();
	auto const srcSize = source.size();

	auto maybeDestAddress = dataAddress(offset);
	if (!maybeDestAddress) {  // Make sure that offset is within [0, size())
		return makeError(GenericError::DOM, "offset: offset outsize of data range");
	}

	// assert that `offset <= thisSize` - otherwise maybeDestAddress is none
    if (srcSize > thisSize - offset) {  // Make sure that source is writing no more then there is room.
		return makeError(GenericError::DOM, "source: overflow");
	}

    if (srcSize > 0) {
		std::memmove(*maybeDestAddress, source.dataAddress(), srcSize);
    }

	return Ok();
}


Result<void, Error>
MutableMemoryView::read(MutableMemoryView& dest) {
	auto const thisSize = size();
	auto const destSize = dest.size();

    if (thisSize < destSize) {
		return makeError(BasicError::Overflow, "dest is too small");
    }

    std::memmove(dest.dataAddress(), dataAddress(), destSize);
	return Ok();
}


Result<void, Error>
MutableMemoryView::read(MutableMemoryView& dest, size_type bytesToRead, size_type offset) {
	auto const thisSize = size();

    // Check if there is enough room in the dest to hold `bytesToRead` bytes.
	if (dest.size() < bytesToRead) {  // Precondition: `bytesToRead <= dest.size()` don't ask to read more then can hold
		return makeError(GenericError::DOM, "bytesToRead: dest is too small");
	}

	auto maybeDestAddr = dataAddress(offset);
    // Make sure that offset is within [0, size())
	if (!maybeDestAddr) {  // Precondition: `offset <= this.size()` - don't go out of bounds
		return makeError(GenericError::DOM, "offset: outside of bounds");
	}

	// Make sure there is enough data in this buffer to read
	if (thisSize < offset + bytesToRead) {  // Precondition, `offset + bytesToRead <= thisSize` don't read more then is
		return makeError(GenericError::DOM, "offset+bytesToRead: outside of bounds");
	}

	if (bytesToRead > 0) {
		std::memmove(dest.dataAddress(), *maybeDestAddr, bytesToRead);
	}

	return Ok();
}


MutableMemoryView&
MutableMemoryView::fill(byte value) noexcept {
    memset(dataAddress(), value, size());

    return (*this);
}


MutableMemoryView
MutableMemoryView::slice(size_type from, size_type to) noexcept {
	if (from == to) {
		return {dataAddress(), 0};
	}

	auto const thisSize = size();

    // `from` is constrained to [0, size())
    // `to` is constrained to [from, size())
    from = std::min(from, thisSize);
	to = std::min(std::max(to, from), thisSize);

	auto destAddress = begin() + from;

	return {destAddress, to - from};
}
