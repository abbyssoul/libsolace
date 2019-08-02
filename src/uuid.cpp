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
#include "solace/base16.hpp"
#include "solace/posixErrorDomain.hpp"

#include <random>
#include <cstring>  // memcmp (should review)
#include <cstdlib>  // rand
#include <ctime>    // time


using namespace Solace;


UUID::UUID() noexcept
    : _bytes{0}
{
}

UUID::UUID(UUID&& rhs) noexcept {
    memcpy(_bytes, rhs._bytes, StaticSize);
}


UUID::UUID(UUID const& rhs) noexcept {
    memcpy(_bytes, rhs._bytes, StaticSize);
}


UUID::UUID(byte const bytes[StaticSize]) noexcept {
    memcpy(_bytes, bytes, StaticSize);
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

bool UUID::equals(UUID const& rhs) const noexcept {
    return memcmp(_bytes, rhs._bytes, size()) == 0;
}


UUID::reference
UUID::operator[] (size_type index) {
    index = assertIndexInRange(index, 0, size());

    return _bytes[index];
}

UUID::value_type
UUID::operator[] (size_type index) const {
    index = assertIndexInRange(index, 0, size());

    return _bytes[index];
}


bool Solace::operator < (UUID const& lhs, UUID const& rhs) noexcept {
    return memcmp(lhs._bytes, rhs._bytes, lhs.size()) < 0;
}


String
UUID::toString() const {
    char buffer[StringSize];

    return makeString(toString(wrapMemory(buffer)));
}


StringView
UUID::toString(MutableMemoryView buffer) const {
    ByteWriter dest(buffer);
    Base16Encoder encoder(dest);

    auto dataView = view();
    // 123e4567-e89b-12d3-a456-426655440000
    // 8-4-4-4-12
    encoder.encode(dataView.slice(0,   4));
    dest.write('-');
    encoder.encode(dataView.slice(4,   6));
    dest.write('-');
    encoder.encode(dataView.slice(6,   8));
    dest.write('-');
    encoder.encode(dataView.slice(8,  10));
    dest.write('-');
    encoder.encode(dataView.slice(10, 16));

    return StringView{buffer.dataAs<char>(), StringSize};
}



// Here we are stealing a function from base16.cpp
Result<byte, Error>
charToBin(byte c);


Result<UUID, Error>
UUID::parse(StringView const& str) {
    if (str.size() != StringSize) {
		return makeError(SystemErrors::NODATA, "UUID::parse()");
    }

    byte data[StaticSize];
    byte* dest = data;
    const char* src = str.data();
    for (size_type i = 0; i < StringSize; ++i) {
        if (src[i] != '-') {
            auto high = charToBin(src[i]);
            auto low =  charToBin(src[i + 1]);

            if (!high)
				return high.moveError();
            if (!low)
				return low.moveError();

            *(dest++) = static_cast<byte>(high.unwrap() << 4) + static_cast<byte>(low.unwrap());
            i += 1;
        }
    }

    return Ok(makeUUID(wrapMemory(data)));
}


UUID
Solace::makeUUID(MemoryView view) {
    assertTrue(view.size() >= UUID::StaticSize);

    byte bytes[UUID::StaticSize];
    for (UUID::size_type i = 0; i < UUID::StaticSize; ++i) {
        bytes[i] = view[i];
    }

    return {bytes};
}


UUID
Solace::makeUUID(uint32 a0, uint32 a1, uint32 a2, uint32 a3) noexcept {
    byte data[UUID::StaticSize];
    ByteWriter writer{wrapMemory(data)};

    writer.write(a0);
    writer.write(a1);
    writer.write(a2);
    writer.write(a3);

    return {data};
}

UUID
Solace::makeRandomUUID() noexcept {
	std::random_device rd;
	std::default_random_engine e1(rd());

	byte bytes[UUID::StaticSize];

	for (UUID::size_type i = 0; i < UUID::StaticSize; ++i) {
		auto const rndValue = e1();
		auto const stride = (sizeof(rndValue) + i <  UUID::StaticSize) ? sizeof(rndValue) : UUID::StaticSize - i;
		memcpy(bytes + i, &rndValue, stride);
		i += stride;
    }

    return {bytes};
}
