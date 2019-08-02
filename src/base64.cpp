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
#include "solace/posixErrorDomain.hpp"

#include <limits>


using namespace Solace;


static constexpr byte kBase64Alphabet[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static constexpr byte kBase64UrlAlphabet[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";


Result<void, Error>
base64encode(ByteWriter& dest, MemoryView const& src, byte const alphabet[65]) {
    MemoryView::size_type i = 0;

    for (; i + 2 < src.size(); i += 3) {
        byte const encoded[] = {
            alphabet[ (src[i] >> 2) & 0x3F],
            alphabet[((src[i] & 0x3) << 4)     | (static_cast<int>(src[i + 1] & 0xF0) >> 4)],
            alphabet[((src[i + 1] & 0xF) << 2) | (static_cast<int>(src[i + 2] & 0xC0) >> 6)],
            alphabet[  src[i + 2] & 0x3F]
        };

        auto res = dest.write(wrapMemory(encoded));
        if (!res)
			return res.moveError();
    }


    if (i < src.size()) {
        byte encoded[4];
        encoded[0] = alphabet[(src[i] >> 2) & 0x3F];
        if (i + 1 == src.size()) {
            encoded[1] = alphabet[((src[i] & 0x3) << 4)];
            encoded[2] = '=';
        } else {
            encoded[1] = alphabet[((src[i] & 0x3) << 4) | (static_cast<int>(src[i + 1] & 0xF0) >> 4)];
            encoded[2] = alphabet[((src[i + 1] & 0xF) << 2)];
        }

        encoded[3] = '=';
        auto res = dest.write(wrapMemory(encoded));
        if (!res)
			return res.moveError();
    }

    return Ok();
}



/* aaaack but it's fast and const should make it shared text page. */
static const byte pr2six[256] = {
    /* ASCII table */
//  00, 01, 02, 03, 04, 05, 06, 07, 08, 09, 0A, 0B, 0C, 0D, 0E, 0F,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  // 00..0F
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  // 10..1F
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,  // 20..2F
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,  // 30..3F
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,  // 40..4F
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,  // 50..5F
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,  // 60..6F
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,  // 70..7F
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  // 80..8F
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  // 90..9F
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  // A0..AF
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  // B0..BF
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  // C0..CF
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  // D0..DF
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  // E0..EF
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64   // F0..FF
};


static const byte prUrl2six[256] = {
    /* ASCII table */
//  00, 01, 02, 03, 04, 05, 06, 07, 08, 09, 0A, 0B, 0C, 0D, 0E, 0F,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  // 00..0F
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  // 10..1F
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64,  // 20..2F
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,  // 30..3F
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,  // 40..4F
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 63,  // 50..5F
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,  // 60..6F
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,  // 70..7F
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  // 80..8F
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  // 90..9F
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  // A0..AF
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  // B0..BF
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  // C0..CF
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  // D0..DF
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  // E0..EF
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64   // F0..FF
};



Result<void, Error>
base64decode(ByteWriter& dest, MemoryView const& src, byte const* decodingTable) {
    byte const* bufin = src.dataAddress();
    if (!bufin || src.size() == 0) {
		return makeError(SystemErrors::NODATA, "base64decode");
    }

    while (decodingTable[*(bufin++)] <= 63)
    {}

    long nprbytes = (bufin - src.dataAddress()) - 1;  // NOLINT(runtime/int)
    bufin = src.dataAddress();

    while (nprbytes > 4) {
        byte const encoded[] = {
            static_cast<byte>(decodingTable[bufin[0]] << 2 | decodingTable[bufin[1]] >> 4),
            static_cast<byte>(decodingTable[bufin[1]] << 4 | decodingTable[bufin[2]] >> 2),
            static_cast<byte>(decodingTable[bufin[2]] << 6 | decodingTable[bufin[3]])
        };

        auto res = dest.write(wrapMemory(encoded));
        if (!res)
			return res.moveError();

        bufin += 4;
        nprbytes -= 4;
    }

    /* Note: (nprbytes == 1) would be an error, so just ingore that case */
    if (nprbytes > 1) {
        auto res = dest.write(static_cast<byte>(decodingTable[bufin[0]] << 2 | decodingTable[bufin[1]] >> 4));
        if (!res)
			return res.moveError();
    }
    if (nprbytes > 2) {
        auto res = dest.write(static_cast<byte>(decodingTable[bufin[1]] << 4 | decodingTable[bufin[2]] >> 2));
        if (!res)
			return res.moveError();
    }
    if (nprbytes > 3) {
        auto res = dest.write(static_cast<byte>(decodingTable[bufin[2]] << 6 | decodingTable[bufin[3]]));
        if (!res)
			return res.moveError();
    }

    return Ok();
}


Base64Encoder::size_type
Base64Encoder::encodedSize(size_type len) {
    return ((4 * len / 3) + 3) & ~3;
}


Base64Decoder::size_type
Base64Decoder::decodedSize(MemoryView const& data) {
    if (data.empty()) {
        return 0;
    }

    if (data.size() % 4) {
        return 0;  // FIXME: Probably throw!
    }

    size_type nprbytes = 0;
    for (const auto& b : data) {
        if (pr2six[b] <= 63) {
            ++nprbytes;
        } else {
            break;
        }
    }

    return (nprbytes * 3 / 4);
}


Base64Decoder::size_type
Base64Decoder::encodedSize(MemoryView const& data) const {
    return decodedSize(data);
}


Result<void, Error>
Base64Encoder::encode(MemoryView const& src) {
   return base64encode(*getDestBuffer(), src, kBase64Alphabet);
}

Result<void, Error>
Base64UrlEncoder::encode(MemoryView const& src) {
    return base64encode(*getDestBuffer(), src, kBase64UrlAlphabet);
}

Result<void, Error>
Base64Decoder::encode(MemoryView const& src) {
    return base64decode(*getDestBuffer(), src, pr2six);
}

Result<void, Error>
Base64UrlDecoder::encode(MemoryView const& src) {
    return base64decode(*getDestBuffer(), src, prUrl2six);
}
