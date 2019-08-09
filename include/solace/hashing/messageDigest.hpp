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
 * libSolace: A base class for message digest algorithms
 *	@file		solace/hashing/messageDigest.hpp
 ******************************************************************************/
#pragma once
#ifndef SOLACE_HASHING_MESSAGEDIGEST_HPP
#define SOLACE_HASHING_MESSAGEDIGEST_HPP

#include "solace/types.hpp"
#include "solace/array.hpp"
#include "solace/string.hpp"


namespace Solace {
namespace hashing {

/**
 * This is an interface for message digest algorithms.
 * Message digests are secure one-way hash functions that take arbitrary-sized data and output a fixed-length value.
 * Some implementation of MessagDigest are MD5 hash and SHA-2.
 */
class MessageDigest {
public:

    using value_type = byte;
    using Storage = Array<byte>;
    using size_type = Storage::size_type;

    using const_iterator = Storage::const_iterator;
    using const_reference = Storage::const_reference;
    using const_pointer = Storage::const_pointer;

public:

    MessageDigest(MessageDigest&& rhs) = default;

    MessageDigest(Storage&& bytes)
        : _storage{std::move(bytes)}
    { }

    MessageDigest(MemoryView viewBytes);

    MessageDigest& swap(MessageDigest& rhs) noexcept {
        using std::swap;
        swap(_storage, rhs._storage);

        return (*this);
    }

    MessageDigest& operator= (MessageDigest&& rhs) noexcept {
        return swap(rhs);
    }

    /**
     * Get a length of the digest in bits.
     * @return Size of the digest in bits.
     */
    size_type getDigestLength() const {
        return size() * 8;
    }

    /**
     * Get the size of the digest in bytes.
     * @return The size of the digest in bytes.
     */
    size_type size() const noexcept {
        return _storage.size();
    }

    const_reference operator[] (size_type index) const {
        index = assertIndexInRange(index, 0, size());

        return _storage[index];
    }

    /**
     * Return iterator to beginning of the collection
     * @return iterator to beginning of the collection
     */
    const_iterator begin() const noexcept {
        return _storage.begin();
    }

    /**
     * Return iterator to end of the collection
     * @return iterator to end of the collection
     */
    const_iterator end() const noexcept {
        return _storage.end();
    }

    const_pointer data() const noexcept {
        return _storage.data();
    }

    MemoryView view() const noexcept {
        return _storage.view().view();
    }

    String toString() const;

    friend bool operator== (MessageDigest const& lhs, MessageDigest const& rhs);
    friend bool operator== (MessageDigest const& lhs, std::initializer_list<byte> rhs);
    friend bool operator== (std::initializer_list<byte> lhs, MessageDigest const& rhs);

private:

    Storage     _storage;

};


inline
void swap(MessageDigest& lhs, MessageDigest& rhs) noexcept {
    lhs.swap(rhs);
}

inline
bool operator== (MessageDigest const& lhs, MessageDigest const& rhs) {
    return (lhs._storage == rhs._storage);
}

inline
bool operator== (MessageDigest const& lhs, std::initializer_list<byte> rhs) {
    return lhs._storage.equals(rhs);
}

inline
bool operator== (std::initializer_list<byte> lhs, MessageDigest const& rhs) {
    return rhs._storage.equals(lhs);
}


}  // End of namespace hashing
}  // End of namespace Solace
#endif  // SOLACE_HASHING_MESSAGEDIGEST_HPP
