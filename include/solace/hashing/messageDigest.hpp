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
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_HASHING_MESSAGEDIGEST_HPP
#define SOLACE_HASHING_MESSAGEDIGEST_HPP

#include "solace/types.hpp"
#include "solace/string.hpp"
#include "solace/byteBuffer.hpp"


namespace Solace {
namespace hashing {

/**
 * This is an interface for message digest algorithms.
 * Message digests are secure one-way hash functions that take arbitrary-sized data and output a fixed-length value.
 * Some implementation of MessagDigest are MD5 hash and SHA-2.
 */
class MessageDigest : public IFormattable {
public:

    using value_type = byte;
    using Storage = Array<byte>;
    using size_type = Storage::size_type;

    using const_iterator = Storage::const_iterator;
    using const_reference = Storage::const_reference;
    using const_pointer = Storage::const_pointer;

public:

    ~MessageDigest() override = default;

    MessageDigest(const MessageDigest& rhs) = default;

    MessageDigest(MessageDigest&& rhs) = default;

    MessageDigest(byte* bytes, size_type digestSize) : _storage(digestSize, bytes)
    { }

    MessageDigest(const ImmutableMemoryView& viewBytes) : _storage(viewBytes.size(), viewBytes.dataAddress())
    { }

    MessageDigest(const Storage& bytes) : _storage(bytes)
    { }

    MessageDigest(Storage&& bytes) : _storage(std::move(bytes))
    { }

    MessageDigest(std::initializer_list<byte> bytes) : _storage(bytes)
    { }

    MessageDigest& swap(MessageDigest& rhs) noexcept {
        using std::swap;
        swap(_storage, rhs._storage);

        return (*this);
    }

    MessageDigest& operator= (const MessageDigest& rhs) noexcept {
        MessageDigest(rhs).swap(*this);

        return *this;
    }

    MessageDigest& operator= (MessageDigest&& rhs) noexcept {
        return swap(rhs);
    }

    /**
     * Get a length of the digest in bits.
     * @return Size of the digest in bits.
     */
    size_type getDigestLength() const {
        return size() / 8;
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


    const_reference first() const { return _storage.first(); }
    const_reference last() const { return _storage.last(); }

    const_pointer data() const noexcept {
        return _storage.data();
    }

    // TODO(abbyssoul): should be ImmutableMemoryView
//    MemoryView view() const noexcept {
//        return wrapMemory(_storage.data(), _storage.size()); // _storage.view().viewShallow();
//    }

    String toString() const override;

    friend bool operator== (const MessageDigest& lhs, const MessageDigest& rhs);

private:

    Storage     _storage;

};


inline
void swap(MessageDigest& lhs, MessageDigest& rhs) noexcept {
    lhs.swap(rhs);
}

inline
bool operator== (const MessageDigest& lhs, const MessageDigest& rhs) {
    return (lhs._storage == rhs._storage);
}

}  // End of namespace hashing
}  // End of namespace Solace
#endif  // SOLACE_HASHING_MESSAGEDIGEST_HPP
