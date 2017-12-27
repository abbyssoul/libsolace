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
 * libSolace: Base64 encoder/decoder
 *	@file		solace/base64.hpp
 *	@brief		Base64 encoder/decoder classes
 ******************************************************************************/
#pragma once
#ifndef SOLACE_BASE64_HPP
#define SOLACE_BASE64_HPP

#include "solace/encoder.hpp"

namespace Solace {

/**
 * RFC-4648 compatible Base64 encoder.
 */
class Base64Encoder : public Encoder {
public:
    using Encoder::size_type;

    static size_type encodedSize(size_type len);

public:

    Base64Encoder(ByteBuffer& dest) :
        Encoder(dest)
    {}


    size_type encodedSize(const ImmutableMemoryView& data) const override {
        return encodedSize(data.size());
    }

    using Encoder::encode;

    void encode(const ImmutableMemoryView& src) override;
};


/**
 * RFC-4648 compatible Base64 decoder.
 */
class Base64Decoder : public Encoder {
public:
    using Encoder::size_type;

    static size_type decodedSize(const ImmutableMemoryView& data);

public:

    Base64Decoder(ByteBuffer& dest) :
        Encoder(dest)
    {}

    size_type encodedSize(const ImmutableMemoryView& data) const override;

    using Encoder::encode;

    void encode(const ImmutableMemoryView& src) override;

};


/**
 * URL safe variant of Base64 encoder.
 */
class Base64UrlEncoder : public Base64Encoder {
public:
    using Base64Encoder::size_type;
    using Base64Encoder::encodedSize;

public:

    Base64UrlEncoder(ByteBuffer& dest) :
        Base64Encoder(dest)
    {}

    using Base64Encoder::encode;

    void encode(const ImmutableMemoryView& src) override;
};


/**
 * URL safe variant of Base64 decoder.
 */
class Base64UrlDecoder : public Base64Decoder {
public:
    using Base64Decoder::size_type;

    using Base64Decoder::decodedSize;
    using Base64Decoder::encodedSize;

public:

    Base64UrlDecoder(ByteBuffer& dest) :
        Base64Decoder(dest)
    {}

    using Base64Decoder::encode;

    void encode(const ImmutableMemoryView& src) override;
};

}  // End of namespace Solace
#endif  // SOLACE_BASE64_HPP
