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
 *	@file		byteBuffer.cpp
 *	@author		$LastChangedBy: soultaker $
 *	@date		$LastChangedDate$
 *	@brief		Implementation of ByteBuffer
 *	ID:			$Id$
 ******************************************************************************/
#include "solace/byteBuffer.hpp"


#include <cstring>  // memcpy


using Solace::MemoryView;
using Solace::byte;
using Solace::ByteBuffer;


ByteBuffer& ByteBuffer::limit(size_type newLimit) {
    if (!(newLimit <= capacity())) {
        raise<IllegalArgumentException>("newLimit");
    }

    _limit = newLimit;

    return *this;
}


ByteBuffer& ByteBuffer::position(size_type newPosition) {
    if (!(newPosition <= limit())) {
        raise<IllegalArgumentException>("newPosition");
    }

    _position = newPosition;

    return *this;
}

ByteBuffer& ByteBuffer::advance(size_type positionIncrement) {
    if (!(position() + positionIncrement <= limit())) {
        raise<IllegalArgumentException>("positionIncrement");
    }

    _position += positionIncrement;

    return *this;
}


ByteBuffer& ByteBuffer::reset() {
    if (!_mark.isSome()) {
        raise<InvalidMarkException>();
    }

    _position = _mark.get();

    return *this;
}


ByteBuffer& ByteBuffer::operator<< (char c) {
    if ( !(_position + 1 <= _limit) ) {
        raise<OverflowException>(_position + 1, _position, _limit);
    }

    _storage[_position++] = c;

    return (*this);
}


ByteBuffer::size_type ByteBuffer::read(byte* bytes, size_type count) {
    if ( !(_position + count <= _limit) ) {
        raise<OverflowException>(_position + count, _position, _limit);
    }

    memcpy(bytes, _storage.dataAddress() + _position, count);
    _position += count;

    return count;
}

ByteBuffer::size_type ByteBuffer::read(size_type offset, byte* bytes, size_type count) const {
    if ( !(offset + count <= _limit) ) {
        raise<OverflowException>(offset + count, 0, _limit);
    }

    memcpy(bytes, _storage.dataAddress() + offset, count);
    return count;
}


byte ByteBuffer::get() {
    if ( !(_position + 1 <= _limit) ) {
        raise<OverflowException>(_position + 1, _position, _limit);
    }

    return _storage[_position++];
}

byte ByteBuffer::get(size_type pos) const {
    if ( !(pos < _limit) ) {
        raise<IllegalArgumentException>("pos");
    }

    return _storage[pos];
}


ByteBuffer& ByteBuffer::write(const MemoryView& memView) {
    return write(memView.dataAddress(), memView.size());
}


ByteBuffer& ByteBuffer::write(const byte* bytes, size_type count) {
    if ( !(count <= remaining()) ) {
         raise<OverflowException>(_position + count, _position, remaining());
    }

    memcpy(_storage.dataAddress() + _position, bytes, count);
    _position += count;

    return (*this);
}


ByteBuffer& ByteBuffer::write(const char* bytes, size_type count) {
    return write(reinterpret_cast<const byte*>(bytes), count);
}
