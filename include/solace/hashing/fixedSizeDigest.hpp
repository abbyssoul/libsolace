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
 * libSolace:
 *  @brief      Fixed size message digest
 *	@file		solace/hashing/messageDigest.hpp
 ******************************************************************************/
#pragma once
#ifndef SOLACE_HASHING_FIXEDSIZEDIGEST_HPP
#define SOLACE_HASHING_FIXEDSIZEDIGEST_HPP

#include "solace/types.hpp"

namespace Solace {
namespace hashing {

template<size_t Size>
class FixedSizeDigest {
public:
    using size_type = uint32;

    /**
     * Get a length of the digest in bits.
     * @return Size of the digest in bits.
     */
    constexpr size_type getDigestLength() const {
        return size() * 8;
    }

    /**
     * Get the size of the digest in bytes.
     * @return The size of the digest in bytes.
     */
    constexpr size_type size() const noexcept {
        return Size;
    }

private:

    byte  _data[Size];
};


FixedSizeDigest<8>
hash(uint32);

}  // End of namespace hashing
}  // End of namespace Solace
#endif  // SOLACE_HASHING_FIXEDSIZEDIGEST_HPP
