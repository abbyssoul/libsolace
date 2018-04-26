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
 ******************************************************************************/

#include "solace/uuid.hpp"
#include "solace/exception.hpp"
#include "solace/base16.hpp"


#include <cstring>  // memcmp (should review)
#include <cstdlib>  // rand
#include <ctime>    // time


using namespace Solace;


// GCC Being dickheaded and requires it here, but not Char::max_bytes, WTF?
constexpr UUID::size_type UUID::StaticSize;
constexpr UUID::size_type UUID::StringSize;


UUID UUID::random() noexcept {
    return UUID();
}


UUID::UUID() noexcept {
    for (auto& b : _bytes) {
        b = static_cast<byte>(rand() % 255);
    }
}


UUID::UUID(UUID&& rhs) noexcept {
    memcpy(_bytes, rhs._bytes, size());
}


UUID::UUID(const UUID& rhs) noexcept {
    memcpy(_bytes, rhs._bytes, size());
}


UUID::UUID(const MemoryView& s) {
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

    for (auto& b : _bytes) {
        b = s.get();
    }
}


UUID::UUID(const std::initializer_list<byte>& bytes) {
    if (bytes.size() < size()) {
        raise<IllegalArgumentException>("bytes");
    }

    size_type i = 0;
    for (const auto& v : bytes) {
        _bytes[i++] = v;

        if (i >= size()) {
            break;
        }
    }
}


UUID& UUID::swap(UUID& rhs) noexcept {
    std::swap(_bytes, rhs._bytes);

    return (*this);
}


bool UUID::isNull() const noexcept {
    int summ = 0;
    for (auto v : _bytes) {
        summ += v;
    }

    return (summ == 0);
}

bool UUID::equals(const UUID& rhs) const noexcept {
    return memcmp(_bytes, rhs._bytes, size()) == 0;
}


UUID::reference UUID::operator[] (size_type index) {
    if (index >= size()) {
        Solace::raise<IndexOutOfRangeException>(index, 0, size());
    }

    return _bytes[index];
}

UUID::value_type UUID::operator[] (size_type index) const {
    if (index >= size()) {
        Solace::raise<IndexOutOfRangeException>(index, 0, size());
    }

    return _bytes[index];
}


bool Solace::operator < (const UUID& lhs, const UUID& rhs) noexcept {
    return memcmp(lhs._bytes, rhs._bytes, lhs.size()) < 0;
}


ImmutableMemoryView UUID::view() const noexcept {
    return wrapMemory(const_cast<byte*>(_bytes), size());
}


MemoryView UUID::view() noexcept {
    return wrapMemory(const_cast<byte*>(_bytes), size());
}


String UUID::toString() const {
    char buffer[StringSize];

    ByteBuffer dest(wrapMemory(buffer));
    Base16Encoder encoder(dest);

    auto dataView = view();
    // 123e4567-e89b-12d3-a456-426655440000
    // 8-4-4-4-12
    encoder.encode(dataView.slice(0,   4));
    dest << '-';
    encoder.encode(dataView.slice(4,   6));
    dest << '-';
    encoder.encode(dataView.slice(6,   8));
    dest << '-';
    encoder.encode(dataView.slice(8,  10));
    dest << '-';
    encoder.encode(dataView.slice(10, 16));

    return String(buffer, StringSize);
}


// Here we are stealing a function from base16.cpp
byte charToBin(byte c);


UUID UUID::parse(const String& str) {
    if (str.size() != StringSize) {
        raise<IllegalArgumentException>("string size");
    }

    byte data[StaticSize];
    byte* dest = data;
    const char* src = str.c_str();
    for (size_type i = 0; i < StringSize; ++i) {
        if (src[i] != '-') {
            const byte high = charToBin(src[i]);
            const byte low =  charToBin(src[i + 1]);

            *(dest++) = static_cast<byte>(high << 4) + static_cast<byte>(low);
            i += 1;
        }
    }

    return UUID(wrapMemory(data));
}



ReadBuffer& Solace::operator >> (ReadBuffer& buffer, UUID& id) {
    auto view = id.view();
    return buffer.read(view);
}


ByteBuffer& Solace::operator << (ByteBuffer& buffer, const UUID& id) {
    return buffer.write(id.view());
}
