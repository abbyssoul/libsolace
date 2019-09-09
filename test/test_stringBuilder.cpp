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
 * @file: test/test_stringBuilder.cpp
*******************************************************************************/
#include <solace/stringBuilder.hpp>	 // Class being tested
#include <solace/memoryManager.hpp>
#include <solace/exception.hpp>

#include <gtest/gtest.h>
#include <cstring>

using namespace Solace;

class TestStringBuilder: public ::testing::Test  {

public:
    static const char* someConstString;
    MemoryManager _memoryManager;

    StringBuilder moveMe() {
		return { _memoryManager.allocate(strlen(someConstString)).unwrap(), someConstString };
	}

public:

    TestStringBuilder(): _memoryManager(4096)
    {
    }

};


TEST_F(TestStringBuilder, testNullString) {
	auto mem = _memoryManager.allocate(5);
	ASSERT_TRUE(mem.isOk());
	EXPECT_NO_THROW(StringBuilder(mem.moveResult(), nullptr));
}

/**
    * Test construction calls
    */
TEST_F(TestStringBuilder, testConstruction) {
    const StringView constStr { someConstString };

    {   // empty buffer usage
		auto mem = _memoryManager.allocate(0);
		ASSERT_TRUE(mem.isOk());
		StringBuilder const empty{mem.moveResult()};

        EXPECT_TRUE(empty.empty());
        EXPECT_TRUE(empty.toString().empty());
    }

    {   // Empty string post-conditions
		auto mem = _memoryManager.allocate(2 * constStr.size());
		ASSERT_TRUE(mem.isOk());
		StringBuilder const empty{mem.moveResult()};  // No throw?

        EXPECT_TRUE(empty.empty());
        EXPECT_TRUE(empty.toString().empty());
    }

    {
		auto mem = _memoryManager.allocate(strlen(someConstString));
		ASSERT_TRUE(mem.isOk());

		StringBuilder const sb{mem.moveResult(), someConstString};

        EXPECT_TRUE(!sb.empty());
        EXPECT_EQ(constStr, sb.view());
    }

    {
		auto mem = _memoryManager.allocate(2 * constStr.size());
		ASSERT_TRUE(mem.isOk());
		StringBuilder const sb{mem.moveResult(), constStr};

        EXPECT_TRUE(!sb.empty());
        EXPECT_EQ(constStr, sb.view());
    }

    {  // Test move construction
		StringBuilder const sb = moveMe();

        EXPECT_TRUE(!sb.empty());
        EXPECT_EQ(constStr, sb.view());
    }
}

/**
    * Test toString conversion
    */
TEST_F(TestStringBuilder, testToString) {
	auto mem = _memoryManager.allocate(strlen(someConstString));
	ASSERT_TRUE(mem.isOk());

	StringBuilder const ident{mem.moveResult(), someConstString};

    auto const str = ident.toString();

    EXPECT_EQ(str, ident.toString());
    EXPECT_EQ(StringView(someConstString), str);
}

const char* TestStringBuilder::someConstString = "Some static string";
