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
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_UUID_HPP
#define SOLACE_UUID_HPP


#include "solace/types.hpp"
#include "solace/traits/iformattable.hpp"
#include "solace/traits/icomparable.hpp"

#include "solace/string.hpp"
#include "solace/byteBuffer.hpp"


namespace Solace {

/**
 * Universally Unique Identifier - as per RFC 4122, eg. unique 128 bit number
 */
class UUID :
        public IComparable<UUID>,
        public Iterable<UUID, byte>,
        public IFormattable {
public:
    typedef uint32 size_type;
    typedef byte value_type;

    typedef value_type&         reference;
    typedef const value_type&   const_reference;

    typedef value_type*         iterator;
    typedef const value_type*   const_iterator;

public:

    /** Size of a uuid data in bytes */
    static constexpr size_type StaticSize = 16;

    /** Size of string representation of the UUID
     * e.g. strlen("123e4567-e89b-12d3-a456-426655440000"); */
    static constexpr size_type StringSize = 36;


    /** Create random UUID
     * This method uses system's random number generator
     */
    static UUID random();

    /**
     * Parse a UUID object from a string.
     *
     * @param str A string representation of the UUID to parse
     * @return Parsed UUID object
     *
     * TODO: Parse family of functions should return Result<UUID, ParseError>
     */
    static UUID parse(const String& str);

    static constexpr size_type static_size() {
        return StaticSize;
    }

public:

    /** Construct default randomly generated UUID */
    UUID() noexcept;

    /** Move-Construct the UUID */
    UUID(UUID&& rhs) noexcept;

    /** Copy Construct the UUID */
    UUID(const UUID& rhs) noexcept;

    /**
     * Try to construct the UUID from individual bytes
     * @note This can throw if number of byte given is less then expected
     */
    UUID(const std::initializer_list<byte>& bytes);

    /** Create the UUID from a byte buffer
     * Try to construct the UUID from individual bytes
     * @note This can throw if number of byte given is less then expected
     */
    UUID(const MemoryView& s);

    /** Create the UUID from a byte buffer
     * Try to construct the UUID from individual bytes
     * @note This can throw if number of byte given is less then expected
     */
    UUID(ByteBuffer& s);

public:

    UUID& swap(UUID& rhs) noexcept;

    UUID& operator= (UUID&& rhs) noexcept {
        return swap(rhs);
    }

    UUID& operator= (const UUID& rhs) noexcept {
        UUID(rhs).swap(*this);

        return *this;
    }

    bool equals(const UUID& rhs) const noexcept override;

    /**
     * Test if this is a 'special' case of a nil UUID
     * @return True is this is a nil UUID
     */
    bool isNull() const noexcept;

    /** Test if the UUID is empty
     * UUID is never empty. It can be nil but it always has bytes
     * @return Always False as UUID is never empty
     */
    bool empty() const noexcept {
        return false;
    }

    /** Get the size of UUID.
     * UUID has a fixed size of 32 bytes as per RFC
     * @return The size of UUID which is always 32
     */
    size_type size() const noexcept {
        return static_size();
    }

    const_iterator begin() const noexcept {
        return _bytes;
    }

    iterator begin() noexcept {
        return _bytes;
    }

    const_iterator end() const noexcept {
        return _bytes + size();
    }

    iterator end() noexcept {
        return _bytes + size();
    }

    value_type first() const noexcept { return _bytes[0]; }
    value_type last()  const noexcept { return _bytes[size() - 1]; }

    reference  operator[] (size_type index);
    value_type operator[] (size_type index) const;

    // TODO(abbyssoul): should be ImmutableMemoryView
    const MemoryView view() const;
    MemoryView view();

    /** @see Iterable::forEach */
    const UUID& forEach(const std::function<void(const_reference)> &f) const override;

    /** @see IFormattable::toString() */
    String toString() const override;

    friend bool operator < (const UUID& lhs, const UUID& rhs) noexcept;

private:

    byte _bytes[StaticSize];

};


bool operator< (const UUID& lhs, const UUID& rhs) noexcept;


inline bool operator == (const UUID& lhs, const UUID& rhs) noexcept {
    return lhs.equals(rhs);
}

inline bool operator != (const UUID& lhs, const UUID& rhs) noexcept {
    return !(lhs == rhs);
}

inline bool operator > (const UUID& lhs, const UUID& rhs) noexcept {
    return rhs < lhs;
}

inline bool operator <= (const UUID& lhs, const UUID& rhs) noexcept {
    return !(rhs < lhs);
}

inline bool operator >= (const UUID& lhs, const UUID& rhs) noexcept {
    return !(lhs < rhs);
}

inline void swap(UUID& lhs, UUID& rhs) noexcept {
    lhs.swap(rhs);
}

}  // namespace Solace
#endif  // SOLACE_UUID_HPP
