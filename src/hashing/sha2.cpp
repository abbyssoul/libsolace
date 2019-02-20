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
 *	@file		solace/hashing/sha2.cpp
 *	@brief		Implementation of SHA-2 hashing algorithm
 ******************************************************************************/
#include "solace/hashing/sha2.hpp"

#include <memory.h>  // memcpy


using namespace Solace;
using namespace Solace::hashing;

static const StringLiteral SHA_256_NAME = "SHA256";


static const byte sha256_padding[64] = {
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/****************************** MACROS ******************************/
#define ROTLEFT(a, b)  (((a) << (b)) | ((a) >> (32 - (b))))
#define ROTRIGHT(a, b) (((a) >> (b)) | ((a) << (32 - (b))))

#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x, 2) ^ ROTRIGHT(x, 13) ^ ROTRIGHT(x, 22))
#define EP1(x) (ROTRIGHT(x, 6) ^ ROTRIGHT(x, 11) ^ ROTRIGHT(x, 25))
#define SIG0(x) (ROTRIGHT(x, 7) ^ ROTRIGHT(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x, 17) ^ ROTRIGHT(x, 19) ^ ((x) >> 10))

/**************************** VARIABLES *****************************/
static const uint32 K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

/*********************** FUNCTION DEFINITIONS ***********************/
#define  SHR(x, n) ((x & 0xFFFFFFFF) >> n)
#define ROTR(x, n) (SHR(x, n) | (x << (32 - n)))

#define S0(x) (ROTR(x,  7) ^ ROTR(x, 18) ^  SHR(x, 3))
#define S1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^  SHR(x, 10))

#define S2(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define S3(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))

#define F0(x, y, z) ((x & y) | (z & (x | y)))
#define F1(x, y, z) (z ^ (x & (y ^ z)))

#define R(t)                                    \
(                                               \
    W[t] = S1(W[t - 2]) + W[t - 7] +            \
           S0(W[t - 15]) + W[t - 16]            \
)

#define P(a, b, c, d, e, f, g, h, x, K)         \
{                                               \
    temp1 = h + S3(e) + F1(e, f, g) + K + x;    \
    temp2 = S2(a) + F0(a, b, c);                \
    d += temp1; h = temp1 + temp2;              \
}


SOLACE_NO_SANITIZE("unsigned-integer-overflow")
void sha256_process(Sha256::State& ctx, byte const data[64]) {
    uint32 temp1, temp2, W[64];
    uint32 A[8];
    uint32 i;

//    memcpy(A, ctx.state, 8 * sizeof(A));
    for (i = 0; i < 8; ++i) {
        A[i] = ctx.state[i];
    }

    ByteReader reader{wrapMemory(data, 64)};

#if defined(SOLACE_SHA256_SMALLER)
    for (i = 0; i < 64; i++) {
        if (i < 16) {
            reader.readBE(W[i]);
        } else {
            R(i);
        }

        P(A[0], A[1], A[2], A[3], A[4], A[5], A[6], A[7], W[i], K[i]);

        temp1 = A[7]; A[7] = A[6]; A[6] = A[5]; A[5] = A[4]; A[4] = A[3];
        A[3] = A[2]; A[2] = A[1]; A[1] = A[0]; A[0] = temp1;
    }
#else /* SOLACE_SHA256_SMALLER */

    for (i = 0; i < 16; ++i) {
        reader.readBE(W[i]);
    }

    for (i = 0; i < 16; i += 8) {
        P(A[0], A[1], A[2], A[3], A[4], A[5], A[6], A[7], W[i+0], K[i+0]);
        P(A[7], A[0], A[1], A[2], A[3], A[4], A[5], A[6], W[i+1], K[i+1]);
        P(A[6], A[7], A[0], A[1], A[2], A[3], A[4], A[5], W[i+2], K[i+2]);
        P(A[5], A[6], A[7], A[0], A[1], A[2], A[3], A[4], W[i+3], K[i+3]);
        P(A[4], A[5], A[6], A[7], A[0], A[1], A[2], A[3], W[i+4], K[i+4]);
        P(A[3], A[4], A[5], A[6], A[7], A[0], A[1], A[2], W[i+5], K[i+5]);
        P(A[2], A[3], A[4], A[5], A[6], A[7], A[0], A[1], W[i+6], K[i+6]);
        P(A[1], A[2], A[3], A[4], A[5], A[6], A[7], A[0], W[i+7], K[i+7]);
    }

    for (i = 16; i < 64; i += 8) {
        P(A[0], A[1], A[2], A[3], A[4], A[5], A[6], A[7], R(i+0), K[i+0]);
        P(A[7], A[0], A[1], A[2], A[3], A[4], A[5], A[6], R(i+1), K[i+1]);
        P(A[6], A[7], A[0], A[1], A[2], A[3], A[4], A[5], R(i+2), K[i+2]);
        P(A[5], A[6], A[7], A[0], A[1], A[2], A[3], A[4], R(i+3), K[i+3]);
        P(A[4], A[5], A[6], A[7], A[0], A[1], A[2], A[3], R(i+4), K[i+4]);
        P(A[3], A[4], A[5], A[6], A[7], A[0], A[1], A[2], R(i+5), K[i+5]);
        P(A[2], A[3], A[4], A[5], A[6], A[7], A[0], A[1], R(i+6), K[i+6]);
        P(A[1], A[2], A[3], A[4], A[5], A[6], A[7], A[0], R(i+7), K[i+7]);
    }
#endif /* SOLACE_SHA256_SMALLER */

    for (i = 0; i < 8; ++i) {
        ctx.state[i] += A[i];
    }
}


void sha256_update(Sha256::State& ctx, const byte input[], Sha256::size_type ilen) {
    Sha256::size_type fill;
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
        sha256_process(ctx, ctx.buffer);

        input += fill;
        ilen  -= fill;
        left = 0;
    }

    while (ilen >= 64) {
        sha256_process(ctx, input);
        input += 64;
        ilen  -= 64;
    }

    if (ilen > 0) {
        memcpy((ctx.buffer + left), input, ilen);
    }
}



Sha256::Sha256() noexcept
    : _state {
          {0, 0},
          {
              0x6a09e667,
              0xbb67ae85,
              0x3c6ef372,
              0xa54ff53a,
              0x510e527f,
              0x9b05688c,
              0x1f83d9ab,
              0x5be0cd19
          },
          {0}
      }
{
}


StringView Sha256::getAlgorithm() const {
    return SHA_256_NAME;
}


Sha256::size_type Sha256::getDigestLength() const {
    return 256;
}


HashingAlgorithm& Sha256::update(MemoryView input) {
    sha256_update(_state, input.dataAddress(), input.size());

    return (*this);
}


MessageDigest Sha256::digest() {
    byte result[32];
    ByteWriter writer{wrapMemory(result)};

    uint32 const high = (_state.total[0] >> 29) | (_state.total[1] <<  3);
    uint32 const low  = (_state.total[0] <<  3);

    byte msglen[8];
    ByteWriter msgLenWriter{wrapMemory(msglen)};
    msgLenWriter.writeBE(high);
    msgLenWriter.writeBE(low);

    uint32 const last = _state.total[0] & 0x3F;
    uint32 const padn = (last < 56) ? (56 - last) : (120 - last);

    sha256_update(_state, sha256_padding, padn);
    sha256_update(_state, msglen, 8);

    for (auto s : _state.state) {
        writer.writeBE(s);
    }

    return MessageDigest(writer.viewWritten());
}
