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
 * libSolace: Base16 encoder/decoder.
 *	@file		solace/base16.hpp
 *	@brief		Base16 encoder/decoder classes
 ******************************************************************************/
#pragma once
#ifndef SOLACE_BASE16_HPP
#define SOLACE_BASE16_HPP

#include "solace/encoder.hpp"


namespace Solace {

/**
 * RFC-4648 compatible Base16 encoder.
 */
class Base16Encoder : public Encoder {
public:
    using Encoder::size_type;

    static size_type encodedSize(size_type len);

public:

    Base16Encoder(ByteBuffer& dest) :
        Encoder(dest)
    {}

    size_type encodedSize(const ImmutableMemoryView& data) const override;

    using Encoder::encode;

    Result<void, Error>
    encode(const ImmutableMemoryView& src) override;
};


/**
 * RFC-4648 compatible Base16 decoder.
 */
class Base16Decoder : public Encoder {
public:
    using Encoder::size_type;

    static size_type encodedSize(size_type len);

public:

    Base16Decoder(ByteBuffer& dest) :
        Encoder(dest)
    {}

    size_type encodedSize(const ImmutableMemoryView& data) const override;

    using Encoder::encode;

    Result<void, Error>
    encode(const ImmutableMemoryView& src) override;
};

}  // End of namespace Solace
#endif  // SOLACE_BASE16_HPP
