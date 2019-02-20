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
 * libSolace: MD5 hash algorithm producing a 128-bit hash value
 *	@file		solace/hashing/MD5.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	@brief		Defines a hash functions MD5
 ******************************************************************************/
#pragma once
#ifndef SOLACE_HASHING_MD5_HPP
#define SOLACE_HASHING_MD5_HPP

#include "solace/hashing/digestAlgorithm.hpp"


namespace Solace {
namespace hashing {

/**
 * MD5 hash algorithm implementation.
 * It is widely used hash function producing a 128-bit hash value.
 *
 * Please note that MD5 is considered "cryptographically broken and unsuitable for further use".
 * This implementation provided for compatibility with legacy systems only.
 * Please @see Sha3 for a better option of hash function.
 *
 */
class MD5 :
        public HashingAlgorithm {
public:
    using HashingAlgorithm::size_type;

    struct State {
        uint32  bits[2];                /*!< number of bytes processed  */
        uint32  state[4];               /*!< intermediate digest state  */
        byte    buffer[64];             /*!< data block being processed */
    };

public:

    using HashingAlgorithm::update;

    MD5() noexcept;

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
#endif  // SOLACE_HASHING_MD5_HPP
