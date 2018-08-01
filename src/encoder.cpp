/*
*  Copyright 2017 Ivan Ryabov
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
 *	@file		src/encoder.cpp
 *	@brief		Implementation of base encoder class.
 ******************************************************************************/
#include "solace/encoder.hpp"

using namespace Solace;

Encoder::~Encoder() = default;


Result<void, Error> Encoder::encode(ByteReader& src) {
    const auto remaining = src.remaining();
    return encode(src.viewRemaining())
            .then([&src, remaining]() { return src.advance(remaining); });
}
