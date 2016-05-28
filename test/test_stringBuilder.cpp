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
#include <cppunit/extensions/HelperMacros.h>
#include <cstring>


using namespace Solace;


class TestStringBuilder: public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestStringBuilder);
        CPPUNIT_TEST(testConstruction);
// TODO(abbyssoul): re-enable        CPPUNIT_TEST(testNullString);
        CPPUNIT_TEST(testToString);
	CPPUNIT_TEST_SUITE_END();

private:
	static const char* someConstString;

protected:

	static StringBuilder moveMe() {
        ByteBuffer buffer(strlen(someConstString));
		return { buffer, someConstString };
	}

public:

    void setUp() override {
	}

    void tearDown() override {
	}

	void testNullString() {
		ByteBuffer buffer(25);

		{// Null string tests
			const StringBuilder nullString(buffer, static_cast<const char*>(nullptr));

            CPPUNIT_ASSERT(nullString.empty());
            CPPUNIT_ASSERT(nullString.toString().empty());
		}

	}
	/**
	 * Test construction calls
	 */
	void testConstruction() {
        const String constStr { someConstString };

        {   // Empty string post-conditions
            ByteBuffer buffer(2 * constStr.size());
            const StringBuilder empty(buffer);  // No throw?

            CPPUNIT_ASSERT(empty.empty());
            CPPUNIT_ASSERT(empty.toString().empty());
		}

		{
            ByteBuffer buffer(strlen(someConstString));
			const StringBuilder sb(buffer, someConstString);

			CPPUNIT_ASSERT(!sb.empty());
			CPPUNIT_ASSERT_EQUAL(constStr, sb.toString());
		}

		{
            ByteBuffer buffer(2 * constStr.size());
			const StringBuilder sb(buffer, constStr);

			CPPUNIT_ASSERT(!sb.empty());
            CPPUNIT_ASSERT_EQUAL(constStr, sb.toString());
		}

		{// Test move construction
			const StringBuilder sb = moveMe();

			CPPUNIT_ASSERT(!sb.empty());
            CPPUNIT_ASSERT_EQUAL(constStr, sb.toString());
		}
	}

	/**
	 * Test toString conversion
	 */
	void testToString() {
        ByteBuffer buffer(strlen(someConstString));
        const StringBuilder ident(buffer, someConstString);

		const auto str = ident.toString();

        CPPUNIT_ASSERT_EQUAL(str, ident.toString());
        CPPUNIT_ASSERT_EQUAL(String(someConstString), str);
    }

};

const char* TestStringBuilder::someConstString = "Some static string";

CPPUNIT_TEST_SUITE_REGISTRATION(TestStringBuilder);
