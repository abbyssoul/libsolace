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
 * libSolace Unit Test Suit
 * @file: test/hashing/test_sha1.cpp
*******************************************************************************/
#include <solace/hashing/sha1.hpp>  // Class being tested
#include <solace/output_utils.hpp>

#include <gtest/gtest.h>

using namespace Solace;
using namespace Solace::hashing;


TEST(TestHashingSHA1, testAlgorithmName) {
    EXPECT_EQ(StringLiteral("SHA1"), Sha1{}.getAlgorithm());
}

TEST(TestHashingSHA1, hashEmptyMessage) {
    Sha1 hash;

    char message[] = "";
    hash.update(wrapMemory(message, sizeof(message) - 1));

    EXPECT_EQ(std::initializer_list<byte>({0xda, 0x39, 0xa3, 0xee, 0x5e, 0x6b, 0x4b, 0x0d, 0x32, 0x55,
                                        0xbf, 0xef, 0x95, 0x60, 0x18, 0x90, 0xaf, 0xd8, 0x07, 0x09}),
                            hash.digest());
}

TEST(TestHashingSHA1, hashSingleLetter) {
    Sha1 hash;

    char message[] = "a";
    hash.update(wrapMemory(message, sizeof(message) - 1));

    EXPECT_EQ(std::initializer_list<byte>({ 0x86, 0xf7, 0xe4, 0x37, 0xfa, 0xa5, 0xa7, 0xfc, 0xe1, 0x5d,
                                            0x1d, 0xdc, 0xb9, 0xea, 0xea, 0xea, 0x37, 0x76, 0x67, 0xb8}),
                            hash.digest());
}

TEST(TestHashingSHA1, hashAx1000) {
    Sha1 hash;

    char message[] = "a";
    auto messageBuffer = wrapMemory(message, sizeof(message) - 1);
    for (uint i = 0; i < 1000; ++i) {
        hash.update(messageBuffer);
    }

    EXPECT_EQ(std::initializer_list<byte>({ 0x29, 0x1e, 0x9a, 0x6c, 0x66, 0x99, 0x49, 0x49, 0xb5, 0x7b,
                                            0xa5, 0xe6, 0x50, 0x36, 0x1e, 0x98, 0xfc, 0x36, 0xb1, 0xba}),
                            hash.digest());
}

TEST(TestHashingSHA1, hashABC) {
    Sha1 hash;

    char message[] = "abc";
    hash.update(wrapMemory(message, sizeof(message) - 1));

    EXPECT_EQ(std::initializer_list<byte>({0xA9, 0x99, 0x3E, 0x36, 0x47, 0x06, 0x81, 0x6A, 0xBA, 0x3E,
                                        0x25, 0x71, 0x78, 0x50, 0xC2, 0x6C, 0x9C, 0xD0, 0xD8, 0x9D}),
                            hash.digest());
}

TEST(TestHashingSHA1, hashMessageDigest) {
    Sha1 hash;

	EXPECT_EQ(160U, hash.getDigestLength());

    char message[] = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    hash.update(wrapMemory(message, sizeof(message) - 1));

    EXPECT_EQ(std::initializer_list<byte>({0x84, 0x98, 0x3E, 0x44, 0x1C, 0x3B, 0xD2, 0x6E, 0xBA, 0xAE,
                                        0x4A, 0xA1, 0xF9, 0x51, 0x29, 0xE5, 0xE5, 0x46, 0x70, 0xF1}),
                            hash.digest());
}
