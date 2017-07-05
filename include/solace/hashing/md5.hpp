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

#include "solace/byteBuffer.hpp"
#include "solace/hashing/fixedHash.hpp"


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
class MD5 {
public:
    MD5();

    H128 encode(ByteBuffer& src);
};


}  // End of namespace hashing
}  // End of namespace Solace
#endif  // SOLACE_HASHING_MD5_HPP
