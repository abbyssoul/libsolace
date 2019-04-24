/*
*  Copyright 2018 Ivan Ryabov
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
 *	@file test/test_atom.cpp
 *	@brief		Test suit for Solace::atom
 ******************************************************************************/
#include <solace/atom.hpp>    // Class being tested.

#include <gtest/gtest.h>

using namespace Solace;


TEST(TestAtom, testStaticEncodDecoding) {
    char buff[16];
    atomToString(atom("test"), buff);
    EXPECT_STREQ("test", buff);

    atomToString(atom("custom"), buff);
    EXPECT_STREQ("custom", buff);
}


TEST(TestAtom, testEncodDecoding) {
	auto parseResult = tryParseAtom("test");
	ASSERT_TRUE(parseResult);

	EXPECT_EQ(atom("test"), *parseResult);

	char buff[16];
	atomToString(*parseResult, buff);
	EXPECT_STREQ("test", buff);
}


TEST(TestAtom, testParsingFailure) {
	auto parseResult = tryParseAtom("long-ass-atom");
	ASSERT_FALSE(parseResult);
}
