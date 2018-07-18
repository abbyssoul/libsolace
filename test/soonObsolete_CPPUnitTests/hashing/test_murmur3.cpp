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
 * @file: test/hashing/test_murmur3.cpp
 * @author: soultaker
 *
*******************************************************************************/
#include <solace/hashing/murmur3.hpp>  // Class being tested

#include <cppunit/extensions/HelperMacros.h>


using namespace Solace;
using namespace Solace::hashing;


class TestHashingMurmur3: public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestHashingMurmur3);
        CPPUNIT_TEST(testAlgorithmName);
        CPPUNIT_TEST(hashEmptyMessage32);
        CPPUNIT_TEST(hashEmptyMessage128);
        CPPUNIT_TEST(hashSingleLetter32);
        CPPUNIT_TEST(hashSingleLetter128);
        CPPUNIT_TEST(hashABC32);
        CPPUNIT_TEST(hashABC128);
        CPPUNIT_TEST(hashMessageDigest32);
        CPPUNIT_TEST(hashMessageDigest128);
        CPPUNIT_TEST(hashAlphabet32);
        CPPUNIT_TEST(hashAlphabet128);
        CPPUNIT_TEST(hashAlphanum32);
        CPPUNIT_TEST(hashAlphanum128);
        CPPUNIT_TEST(hashNumbers32);
        CPPUNIT_TEST(hashNumbers128);
    CPPUNIT_TEST_SUITE_END();

public:

    void testAlgorithmName() {
        CPPUNIT_ASSERT_EQUAL(String("MURMUR3-32"), Murmur3_32(0).getAlgorithm());
        CPPUNIT_ASSERT_EQUAL(String("MURMUR3-128"), Murmur3_128(0).getAlgorithm());
    }

    void hashEmptyMessage32() {
        char message[] = "";
        CPPUNIT_ASSERT_EQUAL(MessageDigest({0x0, 0x0, 0x0, 0x0}),
                             Murmur3_32(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
    }

    void hashEmptyMessage128() {
        char message[] = "";
        CPPUNIT_ASSERT_EQUAL(MessageDigest({0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}),
                             Murmur3_128(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
    }

    void hashSingleLetter32() {
        char message[] = "a";
        CPPUNIT_ASSERT_EQUAL(MessageDigest({0x3c, 0x25, 0x69, 0xb2}),
                             Murmur3_32(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
    }

    void hashSingleLetter128() {
        char message[] = "a";
        CPPUNIT_ASSERT_EQUAL(MessageDigest({0x89, 0x78, 0x59, 0xf6, 0x65, 0x55, 0x55, 0x85,
                                            0x5a, 0x89, 0x0e, 0x51, 0x48, 0x3a, 0xb5, 0xe6}),
                             Murmur3_128(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
    }


    void hashABC32() {
        char message[] = "abc";
        CPPUNIT_ASSERT_EQUAL(MessageDigest({0xB3, 0xDD, 0x93, 0xFA}),
                             Murmur3_32(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
    }

    void hashABC128() {
        char message[] = "abc";
        CPPUNIT_ASSERT_EQUAL(MessageDigest({0x67, 0x78, 0xad, 0x3f, 0x3f, 0x3f, 0x96, 0xb4,
                                            0x52, 0x2d, 0xca, 0x26, 0x41, 0x74, 0xa2, 0x3b}),
                             Murmur3_128(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
    }

    void hashMessageDigest32() {
        CPPUNIT_ASSERT_EQUAL(static_cast<Murmur3_32::size_type>(4), Murmur3_32(0).getDigestLength());

        char message[] = "message digest";
        CPPUNIT_ASSERT_EQUAL(MessageDigest({0x63, 0x8F, 0x41, 0x69}),
                             Murmur3_32(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
    }


    void hashMessageDigest128() {
        CPPUNIT_ASSERT_EQUAL(static_cast<Murmur3_128::size_type>(16), Murmur3_128(0).getDigestLength());

        char message[] = "message digest";
        CPPUNIT_ASSERT_EQUAL(MessageDigest({0xfc, 0x7d, 0x14, 0x76, 0x2d, 0x2c, 0x5d, 0x87,
                                            0x39, 0x6f, 0xbc, 0x12, 0x2a, 0xb0, 0x22, 0xf6}),
                             Murmur3_128(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
    }


    void hashAlphabet32() {
        char message[] = "abcdefghijklmnopqrstuvwxyz";
        CPPUNIT_ASSERT_EQUAL(MessageDigest({0xa3, 0x4e, 0x03, 0x6d}),
                             Murmur3_32(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
    }

    void hashAlphabet128() {
        char message[] = "abcdefghijklmnopqrstuvwxyz";
        CPPUNIT_ASSERT_EQUAL(MessageDigest({0xa9, 0x4a, 0x6f, 0x51, 0x7e, 0x9d, 0x9c, 0x74,
                                            0x29, 0xd5, 0xa7, 0xb6, 0x89, 0x9c, 0xad, 0xe9}),
                             Murmur3_128(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
    }

    void hashAlphanum32() {
        char message[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        CPPUNIT_ASSERT_EQUAL(MessageDigest({0xa2, 0x7a, 0xf3, 0x9b}),
                             Murmur3_32(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
    }

    void hashAlphanum128() {
        char message[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        CPPUNIT_ASSERT_EQUAL(MessageDigest({0x3b, 0x3e, 0xd7, 0x5f, 0x32, 0x1f, 0x99, 0x49,
                                            0xbc, 0xb9, 0xce, 0xa9, 0x3c, 0xd2, 0xad, 0xcb}),
                             Murmur3_128(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
    }

    void hashNumbers32() {
        char message[] = "12345678901234567890123456789012345678901234567890123456789012345678901234567890";
        CPPUNIT_ASSERT_EQUAL(MessageDigest({0xf8, 0xe0, 0x52, 0x87}),
                             Murmur3_32(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
    }

    void hashNumbers128() {
        char message[] = "12345678901234567890123456789012345678901234567890123456789012345678901234567890";
        CPPUNIT_ASSERT_EQUAL(MessageDigest({0xee, 0x6a, 0x87, 0xa4, 0x7f, 0x06, 0x63, 0x91,
                                            0xab, 0xf5, 0xd5, 0xa2, 0x27, 0xca, 0x4f, 0x77}),
                             Murmur3_128(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestHashingMurmur3);

