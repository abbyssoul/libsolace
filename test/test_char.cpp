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
 * @file: test/test_char.cpp
 * @author: soultaker
 *
 * Created on: 18 Apr 2015
*******************************************************************************/
#include <solace/char.hpp>  // Class being tested.

#include <string.h>
#include <gtest/gtest.h>

using namespace Solace;

class TestChar : public ::testing::Test  {

public:

    void setUp() {
	}

    void tearDown() {
	}
};

TEST_F(TestChar, testConstruction) {
    EXPECT_TRUE(Char::max_bytes >= sizeof(uint32));

    {
        const Char c;
        EXPECT_EQ(static_cast<Char::size_type>(0), c.getBytesCount());
        EXPECT_EQ(static_cast<Char::value_type>(0), c.getValue());
    }

    {
        const Char x{'c'};
        EXPECT_EQ(static_cast<Char::size_type>(1), x.getBytesCount());
        EXPECT_EQ(static_cast<Char::value_type>('c'), x.getValue());
    }

    // FIXME(abbyssoul): make unicode work again!
//        {
//            byte bytes[] = {0xE3, 0x81, 0xAA};
//            const Char u(wrapMemory(bytes, sizeof(bytes)));
//            const char* expected = "な";
//            EXPECT_EQ(static_cast<Char::value_type>(14909866), u.getValue());
//            EXPECT_EQ(expected, u.c_str());
//        }
    {
//            byte bytes[] = {0xA4, 0x9D, 0xE2};
        byte bytes[] = {0xE2, 0x9D, 0xA4};
        const Char u(wrapMemory(bytes, sizeof(bytes)));

        EXPECT_EQ(static_cast<Char::size_type>(3), u.getBytesCount());
        EXPECT_EQ(static_cast<char>(0xE2), u.c_str()[0]);
        EXPECT_EQ(static_cast<char>(0x9D), u.c_str()[1]);
        EXPECT_EQ(static_cast<char>(0xA4), u.c_str()[2]);

        // TODO(abbyssoul): Find a way to test it:
//            const char* expect = "♡";
//             EXPECT_EQ(expect, u.c_str());
        // EXPECT_EQ(static_cast<Char::value_type>('\u2764'), u.getValue());
    }
}

TEST_F(TestChar, testAssignment) {
}

TEST_F(TestChar, testEquality) {
    const Char a{'a'};
    const Char b{'b'};

    EXPECT_TRUE(!a.equals(b));
}
