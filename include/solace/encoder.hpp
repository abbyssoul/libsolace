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
 * libSolace: Encoder/decoder base class
 *	@file		solace/encoder.hpp
 *	@brief		Base class for decoder/encoder.
 ******************************************************************************/
#pragma once
#ifndef SOLACE_ENCODER_HPP
#define SOLACE_ENCODER_HPP

#include "solace/byteBuffer.hpp"

namespace Solace {

/**
 * Base class for encoders / decoders.
 */
class Encoder {
public:
    using size_type = ByteBuffer::size_type;

public:

    virtual ~Encoder();

    /**
     * Construct a new instance of encoder that will output transformed output into dect buffer.
     * @param dest Destination buffer to write transformed data to.
     */
    Encoder(ByteBuffer& dest) :
        _dest(&dest)
    {}

    /**
     * Get a pointer to the destanation buffer.
     * @return A pointer to the destantion buffer.
     */
    ByteBuffer* getDestBuffer() const noexcept {
        return _dest;
    }

    /**
     * Estimate the storage size for transformed data.
     * @param data Source data to transform.
     * @return Size in bytes of storage necessery for transformed data.
     */
    virtual size_type encodedSize(ImmutableMemoryView const& data) const = 0;

    /**
     * Transform given data and write transformed output into the dest buffer.
     * @param src Read buffer to read data from.
     */
    Result<void, Error>
    encode(ReadBuffer& src);

    /**
     * Transform given data and write transformed output into the dest buffer.
     * @param src Memory view to read data from.
     */
    virtual Result<void, Error>
    encode(ImmutableMemoryView const& src) = 0;

private:

    ByteBuffer* _dest;
};


}  // End of namespace Solace
#endif  // SOLACE_ENCODER_HPP
