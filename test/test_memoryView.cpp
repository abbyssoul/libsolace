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
 * @file: test/test_memoryView.cpp
 * @author: soultaker
 *
 * Created on: 20 Jun 2016
*******************************************************************************/
#include <solace/memoryView.hpp>  // Class being tested

#include <solace/exception.hpp>
#include <cppunit/extensions/HelperMacros.h>



using namespace Solace;


class TestMemoryView: public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestMemoryView);
        CPPUNIT_TEST(testConstruction);
    CPPUNIT_TEST_SUITE_END();

public:

    void testConstruction() {
        {   // NullPointer smoke test
            // CPPUNIT_ASSERT_THROW(Buffer nullbuffer(321, NULL), IllegalArgumentException);
            MemoryView nullbuffer(321, NULL);
            CPPUNIT_ASSERT_EQUAL(static_cast<Solace::MemoryView::size_type>(0), nullbuffer.size());
        }

        {   // Fixed size constructor
            MemoryView test(3102);

            CPPUNIT_ASSERT(!test.empty());
            CPPUNIT_ASSERT_EQUAL(static_cast<Solace::MemoryView::size_type>(3102), test.size());
            test[0] = 19;
            test[2] = 17;
            test[1] = 4;
            test[test.size() - 1] = 255;
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(19), test.dataAddress()[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(4), test.dataAddress()[1]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(17), test.dataAddress()[2]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(255), test.dataAddress()[test.size() - 1]);
        }

        {   // Wrapping constructor
            byte example[] = {0, 1, 0, 3, 2, 1};  // size = 6
            MemoryView test(sizeof(example), example);

            CPPUNIT_ASSERT(!test.empty());
            CPPUNIT_ASSERT_EQUAL(static_cast<Solace::MemoryView::size_type>(6), test.size());

            for (size_t i = 0; i < test.size(); ++i)
                CPPUNIT_ASSERT_EQUAL(example[i], test.dataAddress()[i]);
        }

        {   // Copy-constructor
            byte example[] = {7, 5, 0, 2, 21, 15, 178};  // size = 7
            MemoryView::size_type exampleSize = sizeof(example);
            MemoryView b1(exampleSize, example);
            MemoryView b2(b1);

            CPPUNIT_ASSERT_EQUAL(exampleSize, b1.size());
            CPPUNIT_ASSERT_EQUAL(exampleSize, b2.size());

            for (size_t i = 0; i < b1.size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(example[i], b1.dataAddress()[i]);
                CPPUNIT_ASSERT_EQUAL(example[i], b2.dataAddress()[i]);
            }

            CPPUNIT_ASSERT_EQUAL(false, b2.isOwner());
        }
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMemoryView);
