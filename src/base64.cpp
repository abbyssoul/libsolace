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
 *	@file		base64.cpp
 *	@brief		Implementation of Base64 encoder and decoder.
 ******************************************************************************/
#include "solace/base64.hpp"
#include "solace/exception.hpp"


#include <cstring>  // memcpy
#include <limits>


using namespace Solace;


static const byte kBase64Alphabet[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


Base64Encoder::size_type
Base64Encoder::encodedSize(size_type len) {
    return ((len + 2) / 3 * 4) + 1;
}



void Base64Encoder::encode(const ImmutableMemoryView& src) {
    auto& dest = *getDestBuffer();
    ImmutableMemoryView::size_type i = 0;

    for (; i + 2 < src.size(); i += 3) {
        dest << kBase64Alphabet[ (src[i] >> 2) & 0x3F];
        dest << kBase64Alphabet[((src[i] & 0x3) << 4)     | (static_cast<int>(src[i + 1] & 0xF0) >> 4)];
        dest << kBase64Alphabet[((src[i + 1] & 0xF) << 2) | (static_cast<int>(src[i + 2] & 0xC0) >> 6)];
        dest << kBase64Alphabet[  src[i + 2] & 0x3F];
    }


    if (i < src.size()) {
        dest << kBase64Alphabet[(src[i] >> 2) & 0x3F];
        if (i + 1 == src.size()) {
            dest << kBase64Alphabet[((src[i] & 0x3) << 4)];
            dest << '=';
        } else {
            dest << kBase64Alphabet[((src[i] & 0x3) << 4) | (static_cast<int>(src[i + 1] & 0xF0) >> 4)];
            dest << kBase64Alphabet[((src[i + 1] & 0xF) << 2)];
        }

        dest << '=';
    }
}

/* aaaack but it's fast and const should make it shared text page. */
static const byte pr2six[256] = {
    /* ASCII table */
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};


Base64Decoder::size_type
Base64Decoder::decodedSize(const ImmutableMemoryView& data) {
    const byte *bufin = data.dataAddress();

    while (pr2six[*(bufin++)] <= 63)
    {}

    const size_type nprbytes = (bufin - data.dataAddress()) - 1;

    return ((nprbytes + 3) / 4) * 3 + 1;
}


Base64Decoder::size_type
Base64Decoder::encodedSize(const ImmutableMemoryView& data) const {
    return decodedSize(data);
}


void Base64Decoder::encode(const ImmutableMemoryView& src) {
    const byte* bufin = src.dataAddress();
    while (pr2six[*(bufin++)] <= 63)
    {}

    long nprbytes = (bufin - src.dataAddress()) - 1;  // NOLINT(runtime/int)
    bufin = src.dataAddress();

    auto& dest = *getDestBuffer();
    while (nprbytes > 4) {
        dest << static_cast<byte>(pr2six[bufin[0]] << 2 | pr2six[bufin[1]] >> 4);
        dest << static_cast<byte>(pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
        dest << static_cast<byte>(pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);

        bufin += 4;
        nprbytes -= 4;
    }

    /* Note: (nprbytes == 1) would be an error, so just ingore that case */
    if (nprbytes > 1) {
        dest << static_cast<byte>(pr2six[bufin[0]] << 2 | pr2six[bufin[1]] >> 4);
    }
    if (nprbytes > 2) {
        dest << static_cast<byte>(pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
    }
    if (nprbytes > 3) {
        dest << static_cast<byte>(pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
    }
}
