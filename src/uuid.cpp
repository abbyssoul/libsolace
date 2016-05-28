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
 *	@file		uuid.cpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#include "solace/uuid.hpp"
#include "solace/exception.hpp"


#include <cstring>  // memcmp (should review)
#include <cstdlib>  // rand
#include <ctime>    // time


using Solace::byte;
using Solace::UUID;
using Solace::Buffer;
using Solace::String;
using Solace::IndexOutOfRangeException;
using Solace::IllegalArgumentException;



// GCC Being dickheaded and requires it here, but not Char::max_bytes, WTF?
constexpr UUID::size_type UUID::StaticSize;


UUID UUID::random() {
    return UUID();
}


UUID::UUID() noexcept {
    // TODO(abbysoul): Randomly generate bytes

    words._lower = rand();
    words._higher = rand();

    for (size_type i = 0; i < StaticSize; ++i) {
        _bytes[i] = rand() % 255;
    }
}


UUID::UUID(const Buffer& s) {
    if (s.size() < size()) {
        raise<IllegalArgumentException>("bytes");
    }

    for (size_type i = 0; i < size(); ++i) {
        _bytes[i] = s[i];
    }
}

UUID::UUID(ByteBuffer& s) {
    if (s.remaining() < size()) {
        raise<IllegalArgumentException>("bytes");
    }

    for (size_type i = 0; i < size(); ++i) {
        _bytes[i] = s.get();
    }
}


UUID::UUID(std::initializer_list<byte> bytes) {
    if (bytes.size() < size()) {
        raise<IllegalArgumentException>("bytes");
    }

    size_type i = 0;
    for (const auto& v : bytes) {
        _bytes[i++] = v;
        if (i >= size())
            break;
    }
}

bool UUID::isNull() const noexcept {
    return ((words._higher == 0) && (words._higher == 0));
}


UUID::reference UUID::operator[] (size_type index) {
    if (index >= size()) {
        raise<IndexOutOfRangeException>(index, 0, size());
    }

    return _bytes[index];
}

UUID::value_type UUID::operator[] (size_type index) const {
    if (index >= size()) {
        raise<IndexOutOfRangeException>(index, 0, size());
    }

    return _bytes[index];
}


bool operator < (const UUID& lhs, const UUID& rhs) noexcept {
    return memcmp(lhs.data(), rhs.data(), lhs.size()) < 0;
}

Buffer UUID::toBytes() const {
    return Buffer::wrap(const_cast<byte*>(_bytes), size(), true);
}


const UUID& UUID::forEach(const std::function<void(const_reference)> &f) const {

    for (const auto& b : _bytes) {
        f(b);
    }

    return *this;
}

void nibbleData(char* buffer, const byte* data, size_t len) {
//    static const char NibbleToHex[] = "0123456789ABCDEF";
    static const char NibbleToHex[] = "0123456789abcdef";

    for (size_t i = 0; i < len; ++i) {
        buffer[2 * i]       = NibbleToHex[data[i] >> 4 ];
        buffer[2 * i + 1]   = NibbleToHex[data[i] & 0x0F];
    }
}

String UUID::toString() const {
    char buffer[36 + 1];
    buffer[8] = '-';
    buffer[13] = '-';
    buffer[18] = '-';
    buffer[23] = '-';
    buffer[36] = 0;

    // 123e4567-e89b-12d3-a456-426655440000
    // 8-4-4-4-12
    nibbleData(buffer + 0, _bytes + 0, 4);
    nibbleData(buffer + 9, _bytes + 4, 2);
    nibbleData(buffer + 14, _bytes + 6, 2);
    nibbleData(buffer + 19, _bytes + 8, 2);
    nibbleData(buffer + 24, _bytes + 10, 6);

    return String(buffer);
}


int char2int(char input) {
  if (input >= '0' && input <= '9')
    return input - '0';
  if (input >= 'A' && input <= 'F')
    return input - 'A' + 10;
  if (input >= 'a' && input <= 'f')
    return input - 'a' + 10;

  // FIXME(abbyssoul): We should just return an error!
  throw IllegalArgumentException("Invalid input string");
}

// This function assumes src to be a zero terminated sanitized string with
// an even number of [0-9a-f] characters, and target to be sufficiently large
void hex2bin(byte* dest, const char* src) {
    while (*src && src[1]) {
        if (*src == '-') {
            src += 1;
        } else {
            *(dest++) = char2int(*src)*16 + char2int(src[1]);
            src += 2;
        }
    }
}



UUID UUID::parse(const String& str) {
    if (str.size() != 36) {
        raise<IllegalArgumentException>("string");
    }

    byte data[StaticSize];
    hex2bin(data, str.c_str());

    return UUID(Buffer(sizeof(data), data, false));
}
