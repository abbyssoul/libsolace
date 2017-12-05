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
 * libSolace: Byte buffer
 *	@file		solace/base64.hpp
 *	@brief		Base64 encoder/decoder classes
 ******************************************************************************/
#pragma once
#ifndef SOLACE_BASE64_HPP
#define SOLACE_BASE64_HPP

#include "solace/byteBuffer.hpp"

namespace Solace {

class Base64Encoder {
public:
    using size_type = ByteBuffer::size_type;

    static size_type encodedSize(size_type len);

public:

    Base64Encoder(ByteBuffer& dest) :
        _dest(&dest)
    {}

    void encode(ReadBuffer& src);
    void encode(const ImmutableMemoryView& src);

private:

    ByteBuffer* _dest;
};


class Base64Decoder {
public:
    using size_type = ByteBuffer::size_type;

    static size_type decodedSize(const ImmutableMemoryView& data);

public:

    Base64Decoder(ByteBuffer& dest) :
        _dest(&dest)
    {}

    void decode(ReadBuffer& src);
    void decode(const ImmutableMemoryView& src);

private:

    ByteBuffer* _dest;
};

}  // End of namespace Solace
#endif  // SOLACE_BASE64_HPP
