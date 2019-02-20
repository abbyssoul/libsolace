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
 * libSolace
 *	@file		solace/hashing/sha3.cpp
 *	@brief		Implementation of SHA-3 hashing algorithm
 ******************************************************************************/
#include "solace/hashing/sha3.hpp"

using namespace Solace;
using namespace Solace::hashing;


static const StringLiteral SHA_3_NAME = "SHA3";


Sha3::Sha3() noexcept
    : _state {
          {0, 0},
          {0},
          {0}
      }
{
}


StringView
Sha3::getAlgorithm() const {
    return SHA_3_NAME;
}


Sha3::size_type
Sha3::getDigestLength() const {
    return 256;
}


HashingAlgorithm&
Sha3::update(MemoryView SOLACE_UNUSED(input)) {
    // TODO(abbyssoul): Not implemented yet.

    return (*this);
}


MessageDigest
Sha3::digest() {
    byte result[32];
    ByteWriter writer{wrapMemory(result)};

    for (auto s : _state.state) {
        writer.writeBE(s);
    }

    return MessageDigest(writer.viewWritten());
}
