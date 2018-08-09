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
 * @file: test/hashing/test_sha256.cpp
 * @author: soultaker
 *
*******************************************************************************/
#include <solace/hashing/sha2.hpp>  // Class being tested
#include <solace/output_utils.hpp>

#include <gtest/gtest.h>

using namespace Solace;
using namespace Solace::hashing;


TEST(TestHashingSHA256, testAlgorithmName) {
    Sha256 hash;

    EXPECT_EQ(String("SHA256"), hash.getAlgorithm());
}

TEST(TestHashingSHA256, hashEmptyMessage) {
    Sha256 hash;

    char message[] = "";
    hash.update(wrapMemory(message, sizeof(message) - 1));

    EXPECT_EQ(std::initializer_list<byte>({ 0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14,
                                            0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
                                            0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c,
                                            0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55 }),
                            hash.digest());
}

TEST(TestHashingSHA256, hashSingleLetter) {
    Sha256 hash;

    char message[] = "a";
    hash.update(wrapMemory(message, sizeof(message) - 1));

    EXPECT_EQ(std::initializer_list<byte>({0xca, 0x97, 0x81, 0x12, 0xca, 0x1b, 0xbd, 0xca,
                                        0xfa, 0xc2, 0x31, 0xb3, 0x9a, 0x23, 0xdc, 0x4d,
                                        0xa7, 0x86, 0xef, 0xf8, 0x14, 0x7c, 0x4e, 0x72,
                                        0xb9, 0x80, 0x77, 0x85, 0xaf, 0xee, 0x48, 0xbb}),
                            hash.digest());
}

TEST(TestHashingSHA256, hashABC) {
    Sha256 hash;

    char message[] = "abc";
    hash.update(wrapMemory(message, sizeof(message) - 1));

    EXPECT_EQ(std::initializer_list<byte>({0xBA, 0x78, 0x16, 0xBF, 0x8F, 0x01, 0xCF, 0xEA,
                                        0x41, 0x41, 0x40, 0xDE, 0x5D, 0xAE, 0x22, 0x23,
                                        0xB0, 0x03, 0x61, 0xA3, 0x96, 0x17, 0x7A, 0x9C,
                                        0xB4, 0x10, 0xFF, 0x61, 0xF2, 0x00, 0x15, 0xAD}),
                            hash.digest());
}

TEST(TestHashingSHA256, hashMessageDigest) {
    Sha256 hash;

    EXPECT_EQ(static_cast<Sha256::size_type>(256), hash.getDigestLength());

    char message[] = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    hash.update(wrapMemory(message, sizeof(message) - 1));

    EXPECT_EQ(std::initializer_list<byte>({0x24, 0x8D, 0x6A, 0x61, 0xD2, 0x06, 0x38, 0xB8,
                                        0xE5, 0xC0, 0x26, 0x93, 0x0C, 0x3E, 0x60, 0x39,
                                        0xA3, 0x3C, 0xE4, 0x59, 0x64, 0xFF, 0x21, 0x67,
                                        0xF6, 0xEC, 0xED, 0xD4, 0x19, 0xDB, 0x06, 0xC1}),
                            hash.digest());
}
