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


#include <cppunit/extensions/HelperMacros.h>

using namespace Solace;


class TestChar : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestChar);
        CPPUNIT_TEST(testConstruction);
        CPPUNIT_TEST(testAssignment);
        CPPUNIT_TEST(testEquality);
	CPPUNIT_TEST_SUITE_END();


public:

    void setUp() override {
	}

    void tearDown() override {
	}

public:

	void testConstruction() {
        CPPUNIT_ASSERT(Char::max_bytes >= sizeof(uint32));

        {
            const Char c;
            CPPUNIT_ASSERT_EQUAL(static_cast<Char::size_type>(0), c.getBytesCount());
            CPPUNIT_ASSERT_EQUAL(static_cast<Char::value_type>(0), c.getValue());
        }

        {
            const Char x{'c'};
            CPPUNIT_ASSERT_EQUAL(static_cast<Char::size_type>(1), x.getBytesCount());
            CPPUNIT_ASSERT_EQUAL(static_cast<Char::value_type>('c'), x.getValue());
        }

        {
            byte bytes[] = {0xE2, 0x9D, 0xA4};
            const Char u(bytes, 3);
            CPPUNIT_ASSERT_EQUAL(static_cast<Char::size_type>(3), u.getBytesCount());
            CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xE2), u.c_str()[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x9D), u.c_str()[1]);
            CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xA4), u.c_str()[2]);
            // TODO(abbyssoul): Find a way to test it:
            // CPPUNIT_ASSERT_EQUAL('♡', (int)u.getValue());
            // CPPUNIT_ASSERT_EQUAL(static_cast<Char::value_type>('\u2764'), u.getValue());
        }
    }

	void testAssignment() {
	}

	void testEquality() {
		const Char a{'a'};
		const Char b{'b'};

		CPPUNIT_ASSERT(!a.equals(b));
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestChar);
