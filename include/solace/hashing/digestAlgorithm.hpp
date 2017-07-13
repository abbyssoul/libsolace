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
#ifndef SOLACE_HASHING_DIGESTALGORITHM_HPP
#define SOLACE_HASHING_DIGESTALGORITHM_HPP

#include "solace/hashing/messageDigest.hpp"


namespace Solace {
namespace hashing {

/**
 * This is an interface for message digest algorithms.
 * Message digests are secure one-way hash functions that take arbitrary-sized data and output a fixed-length value.
 * Some implementation of MessagDigest are MD5 hash and SHA-2.
 */
class HashingAlgorithm {
public:

    typedef uint32 size_type;

public:

    virtual ~HashingAlgorithm() = default;

    /**
     * Get a string name of the hashing algorithm.
     * @return A string name of the hashing algorithm.
     */
    virtual String getAlgorithm() const = 0;

    /**
     * Get a length of the digest in bytes.
     * @return Length of the digest produced by this algorithm.
     */
    virtual size_type getDigestLength() const = 0;

    /**
     * Update the digest with the given input.
     * @param input A memory view to read data from.
     * @return A reference to self for a fluent interface.
     */
    virtual HashingAlgorithm& update(const MemoryView& input) = 0;

    /**
     * Update the digest with the given input.
     * @param input A byte buffer to read message from.
     * @return A reference to self for a fluent interface.
     */
    virtual HashingAlgorithm& update(ByteBuffer& input) {

        auto memView = input.viewRemaining();
        update(memView);
        input.advance(memView.size());

        return *this;
    }

    /*
     * Completes the hash computation by performing final operations such as padding.
     * @return An array of bytes representing message digest.
     */
    virtual MessageDigest digest() = 0;
};

}  // End of namespace hashing
}  // End of namespace Solace
#endif  // SOLACE_HASHING_DIGESTALGORITHM_HPP
