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


using Solace::Buffer;
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

    memcpy(bytes, _storage.data() + _position, count);
    _position += count;

    return count;
}

ByteBuffer::size_type ByteBuffer::read(size_type offset, byte* bytes, size_type count) const {
    if ( !(offset + count <= _limit) ) {
        raise<OverflowException>(offset + count, 0, _limit);
    }

    memcpy(bytes, _storage.data() + offset, count);
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


ByteBuffer& ByteBuffer::write(byte* bytes, size_type count) {
    if ( !(count <= remaining()) ) {
         raise<OverflowException>(_position + count, _position, remaining());
    }

    memcpy(_storage.data() + _position, bytes, count);
    _position += count;
//    for (size_type i = 0; i < count; ++i) {
//      _storage[_position++] = bytes[i];
//    }

    return (*this);
}


ByteBuffer& ByteBuffer::write(const char* bytes, size_type count) {
    if ( !(count <= remaining()) ) {
        raise<OverflowException>(_position + count, _position, remaining());
    }

    memcpy(_storage.data() + _position, bytes, count);
    _position += count;

    return (*this);
}




Buffer::Buffer(size_type newSize): _size(newSize), _ownsData(true), _data(new value_type[newSize]) {

}


Buffer::Buffer(const Buffer& rhs):
            _size(rhs._size),
            _ownsData(rhs._ownsData),
            _data(rhs._data)
{
  if (rhs._data && rhs._ownsData) {
    _data = new value_type[_size];

    memcpy(_data, rhs._data, _size * sizeof(value_type));
  }
}


Buffer::Buffer(size_type size, void* bytes, bool copyData):
    _size(size),
    _ownsData(copyData),
    _data(reinterpret_cast<value_type*>(bytes))
{
    if (!bytes) {
//        raise<IllegalArgumentException>("bytes");
        _size = 0;
    }

    if (bytes && copyData) {
        _data = new value_type[_size];

        memcpy(_data, bytes, _size * sizeof(value_type));
    }
}


Buffer::~Buffer() {
  if (_ownsData && _data) {
    _ownsData = false;

    delete [] _data;

    _data = 0;
    _size = 0;
  }
}


Buffer Buffer::slice(size_type from, size_type to, bool copyData) const {
  return Buffer {to - from, _data + from, copyData};
}
