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
 * @file: test/hashing/test_md5.cpp
*******************************************************************************/
#include <solace/hashing/md5.hpp>  // Class being tested
#include <solace/output_utils.hpp>

#include <gtest/gtest.h>

using namespace Solace;
using namespace Solace::hashing;

TEST(TestHashingMD5, testAlgorithmName) {
    EXPECT_EQ(StringLiteral("MD5"), MD5{}.getAlgorithm());
}

TEST(TestHashingMD5, hashEmptyMessage) {
    MD5 hash;

    char message[] = "";
    hash.update(wrapMemory(message, sizeof(message) - 1));

    EXPECT_EQ(std::initializer_list<byte>({ 0xD4, 0x1D, 0x8C, 0xD9, 0x8F, 0x00, 0xB2, 0x04,
                            0xE9, 0x80, 0x09, 0x98, 0xEC, 0xF8, 0x42, 0x7E }),
                            hash.digest());
}

TEST(TestHashingMD5, hashSingleLetter) {
    MD5 hash;

    char message[] = "a";
    hash.update(wrapMemory(message, sizeof(message) - 1));

    EXPECT_EQ(std::initializer_list<byte>({ 0x0C, 0xC1, 0x75, 0xB9, 0xC0, 0xF1, 0xB6, 0xA8,
                                                0x31, 0xC3, 0x99, 0xE2, 0x69, 0x77, 0x26, 0x61 }),
                            hash.digest());
}

TEST(TestHashingMD5, hashABC) {
    MD5 hash;

    char message[] = "abc";
    hash.update(wrapMemory(message, sizeof(message) - 1));

    EXPECT_EQ(std::initializer_list<byte>({ 0x90, 0x01, 0x50, 0x98, 0x3C, 0xD2, 0x4F, 0xB0,
                                                0xD6, 0x96, 0x3F, 0x7D, 0x28, 0xE1, 0x7F, 0x72 }),
                            hash.digest());
}

TEST(TestHashingMD5, hashMessageDigest) {
    MD5 hash;

    EXPECT_EQ(128, hash.getDigestLength());

    char message[] = "message digest";
    hash.update(wrapMemory(message, sizeof(message) - 1));

    EXPECT_EQ(std::initializer_list<byte>({ 0xF9, 0x6B, 0x69, 0x7D, 0x7C, 0xB7, 0x93, 0x8D,
                                                0x52, 0x5A, 0x2F, 0x31, 0xAA, 0xF1, 0x61, 0xD0 }),
                            hash.digest());
}

TEST(TestHashingMD5, hashAlphabet) {
    MD5 hash;

    char message[] = "abcdefghijklmnopqrstuvwxyz";
    hash.update(wrapMemory(message, sizeof(message) - 1));

    EXPECT_EQ(std::initializer_list<byte>({ 0xC3, 0xFC, 0xD3, 0xD7, 0x61, 0x92, 0xE4, 0x00,
                                                0x7D, 0xFB, 0x49, 0x6C, 0xCA, 0x67, 0xE1, 0x3B }),
                            hash.digest());
}

TEST(TestHashingMD5, hashAlphanum) {
    MD5 hash;

    char message[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    hash.update(wrapMemory(message, sizeof(message) - 1));

    EXPECT_EQ(std::initializer_list<byte>({ 0xD1, 0x74, 0xAB, 0x98, 0xD2, 0x77, 0xD9, 0xF5,
                                                0xA5, 0x61, 0x1C, 0x2C, 0x9F, 0x41, 0x9D, 0x9F }),
                            hash.digest());
}

TEST(TestHashingMD5, hashNumbers) {
    MD5 hash;

    char message[] = "12345678901234567890123456789012345678901234567890123456789012" \
                        "345678901234567890";
    hash.update(wrapMemory(message, sizeof(message) - 1));

    EXPECT_EQ(std::initializer_list<byte>({ 0x57, 0xED, 0xF4, 0xA2, 0x2B, 0xE3, 0xC9, 0x55,
                                                0xAC, 0x49, 0xDA, 0x2E, 0x21, 0x07, 0xB6, 0x7A }),
                            hash.digest());
}
