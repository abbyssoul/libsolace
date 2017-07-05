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
 *	@file		solace/hashing/sha2.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	@brief		Defines a family of cryptographic hash functions SHA-2
 ******************************************************************************/
#pragma once
#ifndef SOLACE_HASHING_SHA2_HPP
#define SOLACE_HASHING_SHA2_HPP

#include "solace/byteBuffer.hpp"
#include "solace/hashing/fixedHash.hpp"


namespace Solace {
namespace hashing {

class Sha2 {
public:
    Sha2();

    H256 encode(ByteBuffer& src);
};


}  // End of namespace hashing
}  // End of namespace Solace
#endif  // SOLACE_HASHING_SHA2_HPP
