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
 * libSolace: Secure Hash Algorithm 2
 *	@file		solace/hashing/murmur3.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	@brief		Defines cryptographic hash function MURMUR3
 ******************************************************************************/
#pragma once
#ifndef SOLACE_HASHING_MURMUR3_HPP
#define SOLACE_HASHING_MURMUR3_HPP

#include "solace/hashing/digestAlgorithm.hpp"


namespace Solace {
namespace hashing {

/**
 * Implementation of Murmur3 cryptographic hashing algorithm with 32bit digest.
 * Murmur is a family of good general purpose hashing functions, suitable for non-cryptographic usage.
 */
class Murmur3_32 :
        public HashingAlgorithm {
public:
    using HashingAlgorithm::size_type;

public:

    using HashingAlgorithm::update;

    Murmur3_32(uint32 seed);

    /**
     * Get a string name of the hashing algorithm.
     * @return A string name of the hashing algorithm.
     */
    String getAlgorithm() const override;

    /**
     * Get a length of the digest in bytes.
     * @return Length of the digest produced by this algorithm.
     */
    size_type getDigestLength() const override;

    /**
     * Update the digest with the given input.
     * @param input A memory view to read data from.
     * @return A reference to self for a fluent interface.
     */
    HashingAlgorithm& update(MemoryView input) override;

    /*
     * Completes the hash computation by performing final operations such as padding.
     * @return An array of bytes representing message digest.
     */
    MessageDigest digest() override;

private:
    uint32  _seed;
    uint32  _hash[1];
};


/**
 * Implementation of Murmur3 cryptographic hashing algorithm with 128bit digest.
 * Murmur is a family of good general purpose hashing functions, suitable for non-cryptographic usage.
 */
class Murmur3_128 :
        public HashingAlgorithm {
public:
    using HashingAlgorithm::size_type;

public:

    using HashingAlgorithm::update;

    Murmur3_128(uint32 seed);

    /**
     * Get a string name of the hashing algorithm.
     * @return A string name of the hashing algorithm.
     */
    String getAlgorithm() const override;

    /**
     * Get a length of the digest in bytes.
     * @return Length of the digest produced by this algorithm.
     */
    size_type getDigestLength() const override;

    /**
     * Update the digest with the given input.
     * @param input A memory view to read data from.
     * @return A reference to self for a fluent interface.
     */
    HashingAlgorithm& update(MemoryView input) override;

    /*
     * Completes the hash computation by performing final operations such as padding.
     * @return An array of bytes representing message digest.
     */
    MessageDigest digest() override;

private:
    uint64  _hash[2];
    uint32  _seed;
};


}  // End of namespace hashing
}  // End of namespace Solace
#endif  // SOLACE_HASHING_MURMUR3_HPP
