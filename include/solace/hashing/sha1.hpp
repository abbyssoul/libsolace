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
 * libSolace: Secure Hash Algorithm 1
 *	@file		solace/hashing/sha1.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	@brief		Defines a family of cryptographic hash functions SHA-1
 ******************************************************************************/
#pragma once
#ifndef SOLACE_HASHING_SHA1_HPP
#define SOLACE_HASHING_SHA1_HPP


#include "solace/hashing/digestAlgorithm.hpp"


namespace Solace {
namespace hashing {

/**
 * Implementation of Sha-1 cryptographic hashing algorithm.
 * SHA-1 produces a 160-bit (20-bytes) hash.
 */
class Sha1 :
        public HashingAlgorithm {
public:
    using HashingAlgorithm::size_type;

    struct State {
        uint32  total[2];
        uint32  state[5];               /*!< intermediate digest state  */
        byte    buffer[64];             /*!< data block being processed */
    };

public:

    using HashingAlgorithm::update;

    Sha1();

    /**
     * Get a string name of the hashing algorithm.
     * @return A string name of the hashing algorithm.
     */
    StringView getAlgorithm() const override;

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

    State _state;
};


}  // End of namespace hashing
}  // End of namespace Solace
#endif  // SOLACE_HASHING_SHA1_HPP
