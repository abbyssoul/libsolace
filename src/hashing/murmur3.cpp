//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

// Note - The x86 and x64 versions do _not_ produce the same results, as the
// algorithms are optimized for their respective platforms. You can still
// compile and run any of them on any platform, but your performance with the
// non-native version will be less than optimal.
/*******************************************************************************
 * libSolace
 *	@file		solace/hashing/murmur3.cpp
 *	@brief		Implementation of Murmur3 hashing algorithm.
 ******************************************************************************/
#include "solace/hashing/murmur3.hpp"

using namespace Solace;
using namespace Solace::hashing;

static const StringLiteral MURMUR3_32_NAME = "MURMUR3-32";
static const StringLiteral MURMUR3_128_NAME = "MURMUR3-128";

//-----------------------------------------------------------------------------
// Platform-specific functions and macros
// Microsoft Visual Studio

#if defined(_MSC_VER)
#define FORCE_INLINE	__forceinline
#include <stdlib.h>

#define ROTL32(x, y)	_rotl(x, y)
#define ROTL64(x, y)	_rotl64(x, y)

#define BIG_CONSTANT(x) (x)

// Other compilers

#else  // defined(_MSC_VER)

#define	FORCE_INLINE inline __attribute__((always_inline))

inline uint32 rotl32(uint32 x, int8 r) {
    return (x << r) | (x >> (32 - r));
}

inline uint64 rotl64(uint64 x, int8 r) {
    return (x << r) | (x >> (64 - r));
}

#define	ROTL32(x, y)	rotl32(x, y)
#define ROTL64(x, y)	rotl64(x, y)

#define BIG_CONSTANT(x) (x##LLU)

#endif  // !defined(_MSC_VER)

//-----------------------------------------------------------------------------
// Block read - if your platform needs to do endian-swapping or can only
// handle aligned reads, do the conversion here

FORCE_INLINE uint32 getblock32(const uint32* p, int i) {
    return p[i];
}

FORCE_INLINE uint64 getblock64(const uint64* p, int i) {
    return p[i];
}

//-----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche

SOLACE_NO_SANITIZE("unsigned-integer-overflow")
FORCE_INLINE uint32 fmix32(uint32 h) {
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

//----------
SOLACE_NO_SANITIZE("unsigned-integer-overflow")
FORCE_INLINE uint64 fmix64(uint64 k) {
    k ^= k >> 33;
    k *= BIG_CONSTANT(0xff51afd7ed558ccd);
    k ^= k >> 33;
    k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
    k ^= k >> 33;

    return k;
}

//-----------------------------------------------------------------------------
SOLACE_NO_SANITIZE("unsigned-integer-overflow")
uint32 murmurhash3_x86_32(const byte* data, const Murmur3_128::size_type len, uint32 seed) {
    const int nblocks = static_cast<int>(len / 4);
    uint32 h1 = seed;

    const uint32 c1 = 0xcc9e2d51;
    const uint32 c2 = 0x1b873593;

    //----------
    // body

    const uint32* blocks = reinterpret_cast<const uint32*>(data + nblocks*4);
    for (int i = -nblocks; i; i++) {
        uint32 k1 = getblock32(blocks, i);

        k1 *= c1;
        k1 = ROTL32(k1, 15);
        k1 *= c2;

        h1 ^= k1;
        h1 = ROTL32(h1, 13);
        h1 = h1 * 5 + 0xe6546b64;
    }

    //----------
    // tail

    const byte* tail = static_cast<const byte*>(data + nblocks*4);
    uint32 k1 = 0;
    switch (len & 3) {
    case 3: k1 ^= static_cast<uint32>(tail[2] << 16);  /* Falls through. */
    case 2: k1 ^= static_cast<uint32>(tail[1] << 8);   /* Falls through. */
    case 1: k1 ^= tail[0];
          k1 *= c1; k1 = ROTL32(k1, 15); k1 *= c2; h1 ^= k1;
    }

    //----------
    // finalization
    h1 ^= len;
    h1 = fmix32(h1);

    return h1;
}


//-----------------------------------------------------------------------------
SOLACE_NO_SANITIZE("unsigned-integer-overflow")
void MurmurHash3_x86_128(const byte* data, const Murmur3_128::size_type len, uint32 seed, void* out) {
    const int blockSize = 16;
    const int nblocks = static_cast<int>(len / blockSize);

    uint32 h1 = seed;
    uint32 h2 = seed;
    uint32 h3 = seed;
    uint32 h4 = seed;

    const uint32 c1 = 0x239b961b;
    const uint32 c2 = 0xab0e9789;
    const uint32 c3 = 0x38b34ae5;
    const uint32 c4 = 0xa1e38b93;

    //----------
    // body

    const uint32* blocks = reinterpret_cast<const uint32 *>(data + nblocks*blockSize);
    for (int i = -nblocks; i; i++) {
        uint32 k1 = getblock32(blocks, i*4+0);
        uint32 k2 = getblock32(blocks, i*4+1);
        uint32 k3 = getblock32(blocks, i*4+2);
        uint32 k4 = getblock32(blocks, i*4+3);

        k1 *= c1; k1  = ROTL32(k1, 15); k1 *= c2; h1 ^= k1;

        h1 = ROTL32(h1, 19); h1 += h2; h1 = h1 * 5 + 0x561ccd1b;

        k2 *= c2; k2  = ROTL32(k2, 16); k2 *= c3; h2 ^= k2;

        h2 = ROTL32(h2, 17); h2 += h3; h2 = h2 * 5 + 0x0bcaa747;

        k3 *= c3; k3  = ROTL32(k3, 17); k3 *= c4; h3 ^= k3;

        h3 = ROTL32(h3, 15); h3 += h4; h3 = h3 * 5 + 0x96cd1c35;

        k4 *= c4; k4  = ROTL32(k4, 18); k4 *= c1; h4 ^= k4;

        h4 = ROTL32(h4, 13); h4 += h1; h4 = h4 * 5 + 0x32ac3b17;
    }

    //----------
    // tail

    const byte* tail = (data + nblocks*blockSize);

    uint32 k1 = 0;
    uint32 k2 = 0;
    uint32 k3 = 0;
    uint32 k4 = 0;

    switch (len & 15) {
    case 15: k4 ^= static_cast<uint32>(tail[14]) << 16;  /* Falls through. */
    case 14: k4 ^= static_cast<uint32>(tail[13]) << 8;   /* Falls through. */
    case 13: k4 ^= static_cast<uint32>(tail[12]) << 0;   /* Falls through. */
           k4 *= c4; k4  = ROTL32(k4, 18); k4 *= c1; h4 ^= k4;
    /* Falls through. */
    case 12: k3 ^= static_cast<uint32>(tail[11]) << 24;   /* Falls through. */
    case 11: k3 ^= static_cast<uint32>(tail[10]) << 16;   /* Falls through. */
    case 10: k3 ^= static_cast<uint32>(tail[ 9]) << 8;   /* Falls through. */
    case  9: k3 ^= static_cast<uint32>(tail[ 8]) << 0;   /* Falls through. */
           k3 *= c3; k3  = ROTL32(k3, 17); k3 *= c4; h3 ^= k3;
    /* Falls through. */
    case  8: k2 ^= static_cast<uint32>(tail[ 7]) << 24;   /* Falls through. */
    case  7: k2 ^= static_cast<uint32>(tail[ 6]) << 16;   /* Falls through. */
    case  6: k2 ^= static_cast<uint32>(tail[ 5]) << 8;   /* Falls through. */
    case  5: k2 ^= static_cast<uint32>(tail[ 4]) << 0;   /* Falls through. */
           k2 *= c2; k2  = ROTL32(k2, 16); k2 *= c3; h2 ^= k2;
    /* Falls through. */
    case  4: k1 ^= static_cast<uint32>(tail[ 3]) << 24;   /* Falls through. */
    case  3: k1 ^= static_cast<uint32>(tail[ 2]) << 16;   /* Falls through. */
    case  2: k1 ^= static_cast<uint32>(tail[ 1]) << 8;   /* Falls through. */
    case  1: k1 ^= static_cast<uint32>(tail[ 0]) << 0;   /* Falls through. */
           k1 *= c1; k1  = ROTL32(k1, 15); k1 *= c2; h1 ^= k1;
    }

    //----------
    // finalization

    h1 ^= len; h2 ^= len; h3 ^= len; h4 ^= len;

    h1 += h2; h1 += h3; h1 += h4;
    h2 += h1; h3 += h1; h4 += h1;

    h1 = fmix32(h1);
    h2 = fmix32(h2);
    h3 = fmix32(h3);
    h4 = fmix32(h4);

    h1 += h2; h1 += h3; h1 += h4;
    h2 += h1; h3 += h1; h4 += h1;

    uint32* outAlias = reinterpret_cast<uint32*>(out);
    outAlias[0] = h1;
    outAlias[1] = h2;
    outAlias[2] = h3;
    outAlias[3] = h4;
}

//-----------------------------------------------------------------------------
SOLACE_NO_SANITIZE("unsigned-integer-overflow")
void MurmurHash3_x64_128(const byte* data, const Murmur3_128::size_type len, uint32 seed, uint64 out[2]) {
    const int blockSize = 16;
    const int nblocks = static_cast<int>(len / blockSize);

    uint64 h1 = seed;
    uint64 h2 = seed;

    const uint64 c1 = BIG_CONSTANT(0x87c37b91114253d5);
    const uint64 c2 = BIG_CONSTANT(0x4cf5ad432745937f);

    //----------
    // body
    const uint64* blocks = reinterpret_cast<const uint64*>(data);
    for (int i = 0; i < nblocks; i++) {
        uint64 k1 = getblock64(blocks, i*2 + 0);
        uint64 k2 = getblock64(blocks, i*2 + 1);

        k1 *= c1; k1  = ROTL64(k1, 31); k1 *= c2; h1 ^= k1;

        h1 = ROTL64(h1, 27); h1 += h2; h1 = h1 * 5 + 0x52dce729;

        k2 *= c2; k2  = ROTL64(k2, 33); k2 *= c1; h2 ^= k2;

        h2 = ROTL64(h2, 31); h2 += h1; h2 = h2 * 5 + 0x38495ab5;
    }

    //----------
    // tail

    const byte* tail = (data + nblocks*blockSize);
    uint64 k1 = 0;
    uint64 k2 = 0;

    switch (len & 15) {
    case 15: k2 ^= static_cast<uint64>(tail[14]) << 48; /* Falls through. */
    case 14: k2 ^= static_cast<uint64>(tail[13]) << 40; /* Falls through. */
    case 13: k2 ^= static_cast<uint64>(tail[12]) << 32; /* Falls through. */
    case 12: k2 ^= static_cast<uint64>(tail[11]) << 24; /* Falls through. */
    case 11: k2 ^= static_cast<uint64>(tail[10]) << 16; /* Falls through. */
    case 10: k2 ^= static_cast<uint64>(tail[ 9]) << 8; /* Falls through. */
    case  9: k2 ^= static_cast<uint64>(tail[ 8]) << 0; /* Falls through. */
           k2 *= c2; k2  = ROTL64(k2, 33); k2 *= c1; h2 ^= k2;
    /* Falls through. */
    case  8: k1 ^= static_cast<uint64>(tail[ 7]) << 56; /* Falls through. */
    case  7: k1 ^= static_cast<uint64>(tail[ 6]) << 48; /* Falls through. */
    case  6: k1 ^= static_cast<uint64>(tail[ 5]) << 40; /* Falls through. */
    case  5: k1 ^= static_cast<uint64>(tail[ 4]) << 32; /* Falls through. */
    case  4: k1 ^= static_cast<uint64>(tail[ 3]) << 24; /* Falls through. */
    case  3: k1 ^= static_cast<uint64>(tail[ 2]) << 16; /* Falls through. */
    case  2: k1 ^= static_cast<uint64>(tail[ 1]) << 8; /* Falls through. */
    case  1: k1 ^= static_cast<uint64>(tail[ 0]) << 0; /* Falls through. */
           k1 *= c1; k1  = ROTL64(k1, 31); k1 *= c2; h1 ^= k1;
    }

    //----------
    // finalization

    h1 ^= len; h2 ^= len;

    h1 += h2;
    h2 += h1;

    h1 = fmix64(h1);
    h2 = fmix64(h2);

    h1 += h2;
    h2 += h1;

    out[0] = h1;
    out[1] = h2;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


Murmur3_32::Murmur3_32(uint32 seed) :
    _seed(seed),
    _hash{0}
{
}


StringView Murmur3_32::getAlgorithm() const {
    return MURMUR3_32_NAME;
}


Murmur3_32::size_type
Murmur3_32::getDigestLength() const {
    return 32;
}


HashingAlgorithm& Murmur3_32::update(MemoryView input) {
    _hash[0] = murmurhash3_x86_32(input.dataAddress(), input.size(), _seed);

    return (*this);
}


MessageDigest Murmur3_32::digest() {
    byte result[4];

    putUint32_BE(_hash[0], result, 0);

    return MessageDigest(wrapMemory(result));
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


Murmur3_128::Murmur3_128(uint32 seed)
    : _hash{0, 0}
    , _seed(seed)

{
}


StringView Murmur3_128::getAlgorithm() const {
    return MURMUR3_128_NAME;
}


Murmur3_128::size_type Murmur3_128::getDigestLength() const {
    return 128;
}


HashingAlgorithm& Murmur3_128::update(MemoryView input) {
#if  defined(__i386__) || defined(__arm__)
    MurmurHash3_x86_128(input.dataAddress(), input.size(), _seed, _hash);
#elif  defined(__x86_64__) ||  defined(__aarch64__)
    MurmurHash3_x64_128(input.dataAddress(), input.size(), _seed, _hash);
#else
#error "Unsupported CPU architecture"
#endif

    return (*this);
}


MessageDigest Murmur3_128::digest() {
    return MessageDigest(wrapMemory(reinterpret_cast<byte*>(_hash), sizeof(_hash)));
}

