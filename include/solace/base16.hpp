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

    Base16Encoder(ByteWriter& dest) :
        Encoder(dest)
    {}

    size_type encodedSize(MemoryView const& data) const override;

    using Encoder::encode;

    Result<void, Error>
    encode(MemoryView const& src) override;
};

class Base16Encoded_Iterator {
public:
    Base16Encoded_Iterator(MemoryView::const_iterator i) :
        _i(i)
    {}

    Base16Encoded_Iterator& operator++ () {
        ++_i;

        return *this;
    }

    StringView operator* () const;

    bool operator!= (Base16Encoded_Iterator const& other) const {
        return (_i != other._i);
    }

    bool operator== (Base16Encoded_Iterator const & other) const {
        return (_i == other._i);
    }

protected:
    MemoryView::const_iterator _i;
};

inline
Base16Encoded_Iterator base16Encode_begin(MemoryView src) {
    return {src.begin()};
}

inline
Base16Encoded_Iterator base16Encode_end(MemoryView src) {
    return {src.end()};
}


/**
 * RFC-4648 compatible Base16 decoder.
 */
class Base16Decoder : public Encoder {
public:
    using Encoder::size_type;

    static size_type encodedSize(size_type len);

public:

    Base16Decoder(ByteWriter& dest) :
        Encoder(dest)
    {}

    size_type encodedSize(const MemoryView& data) const override;

    using Encoder::encode;

    Result<void, Error>
    encode(const MemoryView& src) override;
};



class Base16Decoded_Iterator {
public:
    using value_type = byte;

public:

    Base16Decoded_Iterator(MemoryView::const_iterator i,
                           MemoryView::const_iterator end);

    Base16Decoded_Iterator& operator++ ();

    byte operator* () const {
        return _decodedValue;
    }

    bool operator!= (Base16Decoded_Iterator const& other) const {
        return (_i != other._i);
    }

    bool operator== (Base16Decoded_Iterator const & other) const {
        return (_i == other._i);
    }

protected:
    MemoryView::const_iterator _i;
    MemoryView::const_iterator _end;
    value_type _decodedValue;
};

inline
Base16Decoded_Iterator base16Decode_begin(MemoryView src) {
    return {src.begin(), src.end()};
}

inline
Base16Decoded_Iterator base16Decode_end(MemoryView src) {
    return {src.end(), src.end()};
}

}  // End of namespace Solace
#endif  // SOLACE_BASE16_HPP
