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
 * @author: soultaker
 *
 * Created on: 1 Jan 2015
*******************************************************************************/
#include <solace/stringBuilder.hpp>	 // Class being tested
#include <solace/memoryManager.hpp>
#include <solace/exception.hpp>

#include <cppunit/extensions/HelperMacros.h>
#include <cstring>


using namespace Solace;


class TestStringBuilder: public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestStringBuilder);
        CPPUNIT_TEST(testConstruction);
        CPPUNIT_TEST(testNullString);
        CPPUNIT_TEST(testToString);
    CPPUNIT_TEST_SUITE_END();

private:
    static const char* someConstString;
    MemoryManager _memoryManager;

protected:

    StringBuilder moveMe() {
        return { _memoryManager.create(strlen(someConstString)), someConstString };
	}

public:

    TestStringBuilder(): _memoryManager(4096)
    {
    }

    void setUp() override {
	}

    void tearDown() override {
	}

	void testNullString() {
        auto mem = _memoryManager.create(25);
        ByteBuffer buffer(mem);

        {   // Null Pointer exception test
            CPPUNIT_ASSERT_THROW(const StringBuilder nullString(buffer, NULL), IllegalArgumentException);
		}

	}
	/**
	 * Test construction calls
	 */
	void testConstruction() {
        const String constStr { someConstString };

        {   // empty buffer usage
            auto mem = _memoryManager.create(0);
            ByteBuffer buffer(mem);
            const StringBuilder empty(buffer);  // No throw?

            CPPUNIT_ASSERT(empty.empty());
            CPPUNIT_ASSERT(empty.toString().empty());
        }

        {   // Empty string post-conditions
            auto mem = _memoryManager.create(2 * constStr.size());
            ByteBuffer buffer(mem);
            const StringBuilder empty(buffer);  // No throw?

            CPPUNIT_ASSERT(empty.empty());
            CPPUNIT_ASSERT(empty.toString().empty());
		}

		{
            auto mem = _memoryManager.create(strlen(someConstString));
            ByteBuffer buffer(mem);
			const StringBuilder sb(buffer, someConstString);

			CPPUNIT_ASSERT(!sb.empty());
			CPPUNIT_ASSERT_EQUAL(constStr, sb.toString());
		}

		{
            auto mem = _memoryManager.create(2 * constStr.size());
            ByteBuffer buffer(mem);
			const StringBuilder sb(buffer, constStr);

			CPPUNIT_ASSERT(!sb.empty());
            CPPUNIT_ASSERT_EQUAL(constStr, sb.toString());
		}

        {  // Test move construction
            const StringBuilder sb = moveMe();

            CPPUNIT_ASSERT(!sb.empty());
            CPPUNIT_ASSERT_EQUAL(constStr, sb.toString());
		}
	}

	/**
	 * Test toString conversion
	 */
	void testToString() {
        auto mem = _memoryManager.create(strlen(someConstString));

        ByteBuffer buffer(mem);
        const StringBuilder ident(buffer, someConstString);

		const auto str = ident.toString();

        CPPUNIT_ASSERT_EQUAL(str, ident.toString());
        CPPUNIT_ASSERT_EQUAL(String(someConstString), str);
    }

};

const char* TestStringBuilder::someConstString = "Some static string";

CPPUNIT_TEST_SUITE_REGISTRATION(TestStringBuilder);
