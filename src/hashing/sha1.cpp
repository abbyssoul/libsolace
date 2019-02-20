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
 *	@file		solace/hashing/sha1.cpp
 *	@brief		Implementation of SHA-1 hashing algorithm
 ******************************************************************************/
#include "solace/hashing/sha1.hpp"

#include <memory.h>  // memcpy


using namespace Solace;
using namespace Solace::hashing;

static const StringLiteral SHA_1_NAME = "SHA1";


static const byte sha1_padding[64] = {
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


SOLACE_NO_SANITIZE("unsigned-integer-overflow")
void sha1_process(Sha1::State& ctx, byte const data[64]) {
    uint32 temp, W[16], A, B, C, D, E;

    ByteReader reader{wrapMemory(data, 64)};
    for (auto& x : W) {
        reader.readBE(x);
    }


#define S(x, n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define R(t)                                            \
(                                                       \
    temp = W[(t - 3) & 0x0F] ^ W[(t - 8) & 0x0F] ^ \
           W[(t - 14) & 0x0F] ^ W[ t      & 0x0F],  \
    (W[t & 0x0F] = S(temp, 1))                      \
)

#define P(a, b, c, d, e, x)                                  \
{ e += S(a, 5) + F(b, c, d) + K + x; b = S(b, 30); }

    A = ctx.state[0];
    B = ctx.state[1];
    C = ctx.state[2];
    D = ctx.state[3];
    E = ctx.state[4];

#define F(x, y, z) (z ^ (x & (y ^ z)))
#define K 0x5A827999

    P(A, B, C, D, E, W[0]);
    P(E, A, B, C, D, W[1]);
    P(D, E, A, B, C, W[2]);
    P(C, D, E, A, B, W[3]);
    P(B, C, D, E, A, W[4]);
    P(A, B, C, D, E, W[5]);
    P(E, A, B, C, D, W[6]);
    P(D, E, A, B, C, W[7]);
    P(C, D, E, A, B, W[8]);
    P(B, C, D, E, A, W[9]);
    P(A, B, C, D, E, W[10]);
    P(E, A, B, C, D, W[11]);
    P(D, E, A, B, C, W[12]);
    P(C, D, E, A, B, W[13]);
    P(B, C, D, E, A, W[14]);
    P(A, B, C, D, E, W[15]);
    P(E, A, B, C, D, R(16));
    P(D, E, A, B, C, R(17));
    P(C, D, E, A, B, R(18));
    P(B, C, D, E, A, R(19));

#undef K
#undef F

#define F(x, y, z) (x ^ y ^ z)
#define K 0x6ED9EBA1

    P(A, B, C, D, E, R(20));
    P(E, A, B, C, D, R(21));
    P(D, E, A, B, C, R(22));
    P(C, D, E, A, B, R(23));
    P(B, C, D, E, A, R(24));
    P(A, B, C, D, E, R(25));
    P(E, A, B, C, D, R(26));
    P(D, E, A, B, C, R(27));
    P(C, D, E, A, B, R(28));
    P(B, C, D, E, A, R(29));
    P(A, B, C, D, E, R(30));
    P(E, A, B, C, D, R(31));
    P(D, E, A, B, C, R(32));
    P(C, D, E, A, B, R(33));
    P(B, C, D, E, A, R(34));
    P(A, B, C, D, E, R(35));
    P(E, A, B, C, D, R(36));
    P(D, E, A, B, C, R(37));
    P(C, D, E, A, B, R(38));
    P(B, C, D, E, A, R(39));

#undef K
#undef F

#define F(x, y, z) ((x & y) | (z & (x | y)))
#define K 0x8F1BBCDC

    P(A, B, C, D, E, R(40));
    P(E, A, B, C, D, R(41));
    P(D, E, A, B, C, R(42));
    P(C, D, E, A, B, R(43));
    P(B, C, D, E, A, R(44));
    P(A, B, C, D, E, R(45));
    P(E, A, B, C, D, R(46));
    P(D, E, A, B, C, R(47));
    P(C, D, E, A, B, R(48));
    P(B, C, D, E, A, R(49));
    P(A, B, C, D, E, R(50));
    P(E, A, B, C, D, R(51));
    P(D, E, A, B, C, R(52));
    P(C, D, E, A, B, R(53));
    P(B, C, D, E, A, R(54));
    P(A, B, C, D, E, R(55));
    P(E, A, B, C, D, R(56));
    P(D, E, A, B, C, R(57));
    P(C, D, E, A, B, R(58));
    P(B, C, D, E, A, R(59));

#undef K
#undef F

#define F(x, y, z) (x ^ y ^ z)
#define K 0xCA62C1D6

    P(A, B, C, D, E, R(60));
    P(E, A, B, C, D, R(61));
    P(D, E, A, B, C, R(62));
    P(C, D, E, A, B, R(63));
    P(B, C, D, E, A, R(64));
    P(A, B, C, D, E, R(65));
    P(E, A, B, C, D, R(66));
    P(D, E, A, B, C, R(67));
    P(C, D, E, A, B, R(68));
    P(B, C, D, E, A, R(69));
    P(A, B, C, D, E, R(70));
    P(E, A, B, C, D, R(71));
    P(D, E, A, B, C, R(72));
    P(C, D, E, A, B, R(73));
    P(B, C, D, E, A, R(74));
    P(A, B, C, D, E, R(75));
    P(E, A, B, C, D, R(76));
    P(D, E, A, B, C, R(77));
    P(C, D, E, A, B, R(78));
    P(B, C, D, E, A, R(79));

#undef K
#undef F

    ctx.state[0] += A;
    ctx.state[1] += B;
    ctx.state[2] += C;
    ctx.state[3] += D;
    ctx.state[4] += E;
}


void sha1_update(Sha1::State& ctx, const byte input[], Sha1::size_type ilen) {
    Sha1::size_type fill;
    uint32 left;

    if (ilen == 0)
        return;

    left = ctx.total[0] & 0x3F;
    fill = 64 - left;

    ctx.total[0] += ilen;
    ctx.total[0] &= 0xFFFFFFFF;

    if (ctx.total[0] < ilen)
        ctx.total[1]++;

    if (left && ilen >= fill) {
        memcpy((ctx.buffer + left), input, fill);
        sha1_process(ctx, ctx.buffer);

        input += fill;
        ilen  -= fill;
        left = 0;
    }

    while (ilen >= 64) {
        sha1_process(ctx, input);
        input += 64;
        ilen  -= 64;
    }

    if (ilen > 0) {
        memcpy((ctx.buffer + left), input, ilen);
    }
}


Sha1::Sha1() noexcept {
    /* SHA1 initialization constants */
    _state.state[0] = 0x67452301;
    _state.state[1] = 0xEFCDAB89;
    _state.state[2] = 0x98BADCFE;
    _state.state[3] = 0x10325476;
    _state.state[4] = 0xC3D2E1F0;

    _state.total[0] = 0;
    _state.total[1] = 0;
}


StringView Sha1::getAlgorithm() const {
    return SHA_1_NAME;
}


Sha1::size_type Sha1::getDigestLength() const {
    return 160;
}


HashingAlgorithm& Sha1::update(MemoryView input) {
    sha1_update(_state, input.dataAddress(), input.size());

    return (*this);
}


MessageDigest Sha1::digest() {
    byte result[20];
    ByteWriter writer{wrapMemory(result)};

    uint32 const high = (_state.total[0] >> 29) | (_state.total[1] <<  3);
    uint32 const low = (_state.total[0] <<  3);

    byte msglen[8];
    ByteWriter msgLenWriter{wrapMemory(msglen)};
    msgLenWriter.writeBE(high);
    msgLenWriter.writeBE(low);

    uint32 const last = _state.total[0] & 0x3F;
    uint32 const padn = (last < 56) ? (56 - last) : (120 - last);

    sha1_update(_state, sha1_padding, padn);
    sha1_update(_state, msglen, 8);

    for (auto s : _state.state) {
        writer.writeBE(s);
    }

    return MessageDigest(writer.viewWritten());
}
