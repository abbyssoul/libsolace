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
 *	@file		solace/hashing/md5.cpp
 *	@brief		Implementation of MD5 hashing algorithm
 ******************************************************************************/
#include "solace/hashing/md5.hpp"

#include <cstring>  // memcpy

using namespace Solace;
using namespace Solace::hashing;


static const String MD5_NAME = "MD5";

static const byte md5_padding[64] = {
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


SOLACE_NO_SANITIZE("unsigned-integer-overflow")
void md5_process(MD5::State& ctx, const byte data[64]) {
    uint32_t X[16], A, B, C, D;

    getUint32_LE(X[ 0], data,  0);
    getUint32_LE(X[ 1], data,  4);
    getUint32_LE(X[ 2], data,  8);
    getUint32_LE(X[ 3], data, 12);
    getUint32_LE(X[ 4], data, 16);
    getUint32_LE(X[ 5], data, 20);
    getUint32_LE(X[ 6], data, 24);
    getUint32_LE(X[ 7], data, 28);
    getUint32_LE(X[ 8], data, 32);
    getUint32_LE(X[ 9], data, 36);
    getUint32_LE(X[10], data, 40);
    getUint32_LE(X[11], data, 44);
    getUint32_LE(X[12], data, 48);
    getUint32_LE(X[13], data, 52);
    getUint32_LE(X[14], data, 56);
    getUint32_LE(X[15], data, 60);

#define S(x, n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))
#define F(x, y, z) (z ^ (x & (y ^ z)))
#define P(a, b, c, d, k, s, t) { a += F(b, c, d) + X[k] + t; a = S(a, s) + b; }

    A = ctx.state[0];
    B = ctx.state[1];
    C = ctx.state[2];
    D = ctx.state[3];

    P(A, B, C, D,  0,  7, 0xD76AA478);
    P(D, A, B, C,  1, 12, 0xE8C7B756);
    P(C, D, A, B,  2, 17, 0x242070DB);
    P(B, C, D, A,  3, 22, 0xC1BDCEEE);
    P(A, B, C, D,  4,  7, 0xF57C0FAF);
    P(D, A, B, C,  5, 12, 0x4787C62A);
    P(C, D, A, B,  6, 17, 0xA8304613);
    P(B, C, D, A,  7, 22, 0xFD469501);
    P(A, B, C, D,  8,  7, 0x698098D8);
    P(D, A, B, C,  9, 12, 0x8B44F7AF);
    P(C, D, A, B, 10, 17, 0xFFFF5BB1);
    P(B, C, D, A, 11, 22, 0x895CD7BE);
    P(A, B, C, D, 12,  7, 0x6B901122);
    P(D, A, B, C, 13, 12, 0xFD987193);
    P(C, D, A, B, 14, 17, 0xA679438E);
    P(B, C, D, A, 15, 22, 0x49B40821);

#undef F

#define F(x, y, z) (y ^ (z & (x ^ y)))

    P(A, B, C, D,  1,  5, 0xF61E2562);
    P(D, A, B, C,  6,  9, 0xC040B340);
    P(C, D, A, B, 11, 14, 0x265E5A51);
    P(B, C, D, A,  0, 20, 0xE9B6C7AA);
    P(A, B, C, D,  5,  5, 0xD62F105D);
    P(D, A, B, C, 10,  9, 0x02441453);
    P(C, D, A, B, 15, 14, 0xD8A1E681);
    P(B, C, D, A,  4, 20, 0xE7D3FBC8);
    P(A, B, C, D,  9,  5, 0x21E1CDE6);
    P(D, A, B, C, 14,  9, 0xC33707D6);
    P(C, D, A, B,  3, 14, 0xF4D50D87);
    P(B, C, D, A,  8, 20, 0x455A14ED);
    P(A, B, C, D, 13,  5, 0xA9E3E905);
    P(D, A, B, C,  2,  9, 0xFCEFA3F8);
    P(C, D, A, B,  7, 14, 0x676F02D9);
    P(B, C, D, A, 12, 20, 0x8D2A4C8A);

#undef F

#define F(x, y, z) (x ^ y ^ z)

    P(A, B, C, D,  5,  4, 0xFFFA3942);
    P(D, A, B, C,  8, 11, 0x8771F681);
    P(C, D, A, B, 11, 16, 0x6D9D6122);
    P(B, C, D, A, 14, 23, 0xFDE5380C);
    P(A, B, C, D,  1,  4, 0xA4BEEA44);
    P(D, A, B, C,  4, 11, 0x4BDECFA9);
    P(C, D, A, B,  7, 16, 0xF6BB4B60);
    P(B, C, D, A, 10, 23, 0xBEBFBC70);
    P(A, B, C, D, 13,  4, 0x289B7EC6);
    P(D, A, B, C,  0, 11, 0xEAA127FA);
    P(C, D, A, B,  3, 16, 0xD4EF3085);
    P(B, C, D, A,  6, 23, 0x04881D05);
    P(A, B, C, D,  9,  4, 0xD9D4D039);
    P(D, A, B, C, 12, 11, 0xE6DB99E5);
    P(C, D, A, B, 15, 16, 0x1FA27CF8);
    P(B, C, D, A,  2, 23, 0xC4AC5665);

#undef F

#define F(x, y, z) (y ^ (x | ~z))

    P(A, B, C, D,  0,  6, 0xF4292244);
    P(D, A, B, C,  7, 10, 0x432AFF97);
    P(C, D, A, B, 14, 15, 0xAB9423A7);
    P(B, C, D, A,  5, 21, 0xFC93A039);
    P(A, B, C, D, 12,  6, 0x655B59C3);
    P(D, A, B, C,  3, 10, 0x8F0CCC92);
    P(C, D, A, B, 10, 15, 0xFFEFF47D);
    P(B, C, D, A,  1, 21, 0x85845DD1);
    P(A, B, C, D,  8,  6, 0x6FA87E4F);
    P(D, A, B, C, 15, 10, 0xFE2CE6E0);
    P(C, D, A, B,  6, 15, 0xA3014314);
    P(B, C, D, A, 13, 21, 0x4E0811A1);
    P(A, B, C, D,  4,  6, 0xF7537E82);
    P(D, A, B, C, 11, 10, 0xBD3AF235);
    P(C, D, A, B,  2, 15, 0x2AD7D2BB);
    P(B, C, D, A,  9, 21, 0xEB86D391);

#undef F

    ctx.state[0] += A;
    ctx.state[1] += B;
    ctx.state[2] += C;
    ctx.state[3] += D;
}


void md5_update(MD5::State& ctx, const byte *input, MD5::size_type inputLen) {
    size_t fill;
    uint32_t left;

    if (inputLen == 0)
        return;

    left = ctx.bits[0] & 0x3F;
    fill = 64 - left;

    ctx.bits[0] += inputLen;
    ctx.bits[0] &= 0xFFFFFFFF;

    if ( ctx.bits[0] < inputLen)
        ctx.bits[1]++;

    if (left && inputLen >= fill) {
        memcpy((ctx.buffer + left), input, fill);
        md5_process(ctx, ctx.buffer);
        input += fill;
        inputLen  -= fill;
        left = 0;
    }

    while (inputLen >= 64) {
        md5_process(ctx, input);
        input += 64;
        inputLen  -= 64;
    }


    if (inputLen > 0) {
        memcpy((ctx.buffer + left), input, inputLen);
    }
}


MD5::MD5() {
    _state.bits[0] = 0;
    _state.bits[1] = 0;

    _state.state[0] = 0x67452301;
    _state.state[1] = 0xEFCDAB89;
    _state.state[2] = 0x98BADCFE;
    _state.state[3] = 0x10325476;
}


String MD5::getAlgorithm() const {
    return MD5_NAME;
}


MD5::size_type MD5::getDigestLength() const {
    return 128;
}


HashingAlgorithm& MD5::update(MemoryView input) {
    md5_update(_state, input.dataAddress(), input.size());

    return (*this);
}


MessageDigest MD5::digest() {
    byte result[16];

    uint32 high = (_state.bits[0] >> 29) | (_state.bits[1] <<  3);
    uint32 low  = (_state.bits[0] <<  3);

    byte msglen[8];
    putUint32_LE(low,  msglen, 0);
    putUint32_LE(high, msglen, 4);

    const uint32 last = _state.bits[0] & 0x3F;
    const uint32 padn = (last < 56) ? (56 - last) : (120 - last);

    md5_update(_state, md5_padding, padn);
    md5_update(_state, msglen, 8);

    putUint32_LE(_state.state[0], result,  0);
    putUint32_LE(_state.state[1], result,  4);
    putUint32_LE(_state.state[2], result,  8);
    putUint32_LE(_state.state[3], result, 12);

    return MessageDigest(wrapMemory(result));
}
