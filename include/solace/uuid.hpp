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
 * libSolace: Universally unique identifier (UUID)
 *	@file		solace/uuid.hpp
 ******************************************************************************/
#pragma once
#ifndef SOLACE_UUID_HPP
#define SOLACE_UUID_HPP

#include "solace/types.hpp"

#include "solace/memoryView.hpp"
#include "solace/string.hpp"
#include "solace/result.hpp"
#include "solace/error.hpp"


namespace Solace {

/**
 * Universally Unique Identifier - as per RFC 4122, eg. unique 128 bit number.
 * UUID is a collection of bytes or a single unique 128bit nubmer.
 */
class UUID {
public:
    using size_type = MemoryView::size_type;
    using value_type = byte;

    using reference = value_type &;
    using const_reference = const value_type &;

    using iterator = value_type *;
    using const_iterator = const value_type *;

public:

    /** Size of a uuid data in bytes */
    static constexpr size_type StaticSize = 16;

    /** Size of string representation of the UUID
     * e.g. strlen("123e4567-e89b-12d3-a456-426655440000"); */
    static constexpr size_type StringSize = 36;

    /**
     * Parse a UUID object from a string.
     *
     * @param str A string representation of the UUID to parse
     * @return Parsed UUID object or an error.
     */
    static Result<UUID, Error> parse(StringView const& str);

public:

    /** Construct default randomly generated UUID */
    UUID() noexcept;

    /** Move-Construct the UUID */
    UUID(UUID&& rhs) noexcept;

    /** Copy Construct the UUID */
    UUID(UUID const& rhs) noexcept;

    UUID(byte const bytes[StaticSize]) noexcept;

public:

    UUID& swap(UUID& rhs) noexcept;

    UUID& operator= (UUID&& rhs) noexcept {
        return swap(rhs);
    }

    UUID& operator= (UUID const& rhs) noexcept {
        UUID(rhs).swap(*this);

        return *this;
    }

    bool equals(UUID const& rhs) const noexcept;

    /**
     * Test if this is a 'special' case of a nil UUID
     * @return True is this is a nil UUID
     */
    bool isNull() const noexcept;

    /** Test if the UUID is empty
     * UUID is never empty. It can be nil but it always has bytes
     * @return Always False as UUID is never empty.
     */
    bool empty() const noexcept {
        return false;
    }

    /** Get the size in bytes of this UUID.
     * UUID has a fixed size of 16 bytes (128bit) as per RFC.
     *
     * @return The size of UUID which is always 16 bytes.
     */
    size_type size() const noexcept {
        return StaticSize;
    }

    const_iterator begin() const noexcept {
        return _bytes;
    }

    iterator begin() noexcept {
        return _bytes;
    }

    const_iterator end() const noexcept {
        // cppcheck-suppress pointerOutOfBounds
        return _bytes + size();
    }

    iterator end() noexcept {
        // cppcheck-suppress pointerOutOfBounds
        return _bytes + size();
    }

    value_type first() const noexcept { return _bytes[0]; }
    value_type last()  const noexcept { return _bytes[size() - 1]; }

    reference  operator[] (size_type index);
    value_type operator[] (size_type index) const;

    [[nodiscard]]
    constexpr MemoryView view() const noexcept {
        return wrapMemory(_bytes);
    }

    [[nodiscard]]
    constexpr MutableMemoryView view() noexcept {
        return wrapMemory(_bytes);
    }

    /**
     * Return string representation of this path
     * @return String representation of this path
     */
    String toString() const;
    StringView toString(MutableMemoryView buffer) const;

    friend bool operator < (UUID const& lhs, UUID const& rhs) noexcept;

private:

    byte _bytes[StaticSize];

};


bool operator< (UUID const& lhs, UUID const& rhs) noexcept;


inline bool operator == (UUID const& lhs, UUID const& rhs) noexcept {
    return lhs.equals(rhs);
}

inline bool operator != (UUID const& lhs, UUID const& rhs) noexcept {
    return !(lhs == rhs);
}

inline bool operator > (UUID const& lhs, UUID const& rhs) noexcept {
    return rhs < lhs;
}

inline bool operator <= (UUID const& lhs, UUID const& rhs) noexcept {
    return !(rhs < lhs);
}

inline bool operator >= (UUID const& lhs, UUID const& rhs) noexcept {
    return !(lhs < rhs);
}

inline void swap(UUID& lhs, UUID& rhs) noexcept {
    lhs.swap(rhs);
}


/** Create the UUID from a byte buffer
 * Try to construct the UUID from individual bytes
 * @note This can throw if number of byte given is less then expected
 */
[[nodiscard]] UUID makeUUID(MemoryView s);

[[nodiscard]] UUID makeUUID(uint32 a0, uint32 a1, uint32 a2, uint32 a3) noexcept;


/** Create random UUID
 * This method uses system's random number generator to generate a new random UUID.
 */
[[nodiscard]] UUID makeRandomUUID() noexcept;

}  // namespace Solace
#endif  // SOLACE_UUID_HPP
