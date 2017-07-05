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
 * libSolace: Base class for hashes of various fixed sizes
 *	@file		solace/hashing/fixedHash.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_HASHING_FIXEDHASH_HPP
#define SOLACE_HASHING_FIXEDHASH_HPP


#include "solace/types.hpp"

#include <array>

namespace Solace {
namespace hashing {


template<size_t N>
class FixedHash {
public:

private:
    std::array<byte, N>     _data;		///< The binary data.
};


using H2048 = FixedHash<256>;
using H1024 = FixedHash<128>;
using H520 = FixedHash<65>;
using H512 = FixedHash<64>;
using H256 = FixedHash<32>;
using H160 = FixedHash<20>;
using H128 = FixedHash<16>;
using H64 = FixedHash<8>;

}  // End of namespace hashing
}  // End of namespace Solace

#endif  // SOLACE_CRYPT_FIXEDHASH_HPP
