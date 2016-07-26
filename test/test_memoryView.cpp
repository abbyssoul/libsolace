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
#include <solace/memoryManager.hpp>


#include <solace/exception.hpp>
#include <cppunit/extensions/HelperMacros.h>



using namespace Solace;


class TestMemoryView: public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestMemoryView);
        CPPUNIT_TEST(testConstruction);
        CPPUNIT_TEST(testFill);
        CPPUNIT_TEST(testWrapping);
    CPPUNIT_TEST_SUITE_END();

protected:

    MemoryManager manager;

public:

    TestMemoryView(): manager(4096) {
    }

    void testFill() {
        MemoryView buffer = manager.create(48);

        buffer.fill(0);
        for (const auto& v : buffer) {
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0), v);
        }

        MemoryView::size_type r = 0;
        buffer.fill(1);
        for (const auto& v : buffer) {
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(1), v);
            r += v;
        }
        CPPUNIT_ASSERT_EQUAL(r, buffer.size());

        buffer.fill(64);
        for (const auto& v : buffer) {
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(64), v);
        }

        buffer.fill(36, 20, 40);
        for (MemoryView::size_type i = 0; i < buffer.size(); ++i) {
            if (i >= 20 && i < 40) {
                CPPUNIT_ASSERT_EQUAL(static_cast<byte>(36), buffer[i]);
            } else {
                CPPUNIT_ASSERT_EQUAL(static_cast<byte>(64), buffer[i]);
            }
        }

        // 'To' index is out of range
        CPPUNIT_ASSERT_THROW(buffer.fill(3, 3, 130), IndexOutOfRangeException);
        // 'From' index is out of range
        CPPUNIT_ASSERT_THROW(buffer.fill(3, 100, 30), IndexOutOfRangeException);
        // 'To' index is out of range - less then from
        CPPUNIT_ASSERT_THROW(buffer.fill(3, 30, 3), IndexOutOfRangeException);
        // 'From' and 'To' index is out of range
        CPPUNIT_ASSERT_THROW(buffer.fill(3, 100, 130), IndexOutOfRangeException);
    }

    void testWrapping() {
        const auto nullB = static_cast<void*>(nullptr);
        CPPUNIT_ASSERT_NO_THROW(auto buffer = MemoryView::wrap(nullB, 0));

        {   // Can't wrap nullptr with non-zero size
            CPPUNIT_ASSERT_THROW(auto b = MemoryView::wrap(nullB, 321), IllegalArgumentException);
        }

        {   // Wrapping constructor
            byte example[] = {0, 1, 0, 3, 2, 1};  // size = 6
            auto test = MemoryView::wrap(example, sizeof(example));

            CPPUNIT_ASSERT(!test.empty());
            CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(6), test.size());

            for (size_t i = 0; i < test.size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(example[i], test.dataAddress()[i]);
            }
        }
    }

    void testConstruction() {
        {   // Fixed size constructor
            MemoryView test = manager.create(3102);

            CPPUNIT_ASSERT(!test.empty());
            CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(3102), test.size());
            test[0] = 19;
            test[2] = 17;
            test[1] = 4;
            test[test.size() - 1] = 255;
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(19), test.dataAddress()[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(4), test.dataAddress()[1]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(17), test.dataAddress()[2]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(255), test.dataAddress()[test.size() - 1]);
        }


        {   // Copy-constructor
            byte example[] = {7, 5, 0, 2, 21, 15, 178};  // size = 7
            const MemoryView::size_type exampleSize = sizeof(example);
            auto b1 = MemoryView::wrap(example, exampleSize);
            MemoryView b2(b1);

            CPPUNIT_ASSERT_EQUAL(exampleSize, b1.size());
            CPPUNIT_ASSERT_EQUAL(exampleSize, b2.size());

            for (size_t i = 0; i < b1.size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(example[i], b1.dataAddress()[i]);
                CPPUNIT_ASSERT_EQUAL(example[i], b2.dataAddress()[i]);
            }

//            CPPUNIT_ASSERT_EQUAL(false, b2.isOwner());
        }
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMemoryView);
