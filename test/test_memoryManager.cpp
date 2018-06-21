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
 * @file: test/test_memoryManager.cpp
 * @author: soultaker
 *
 * Created on: 20 Jun 2016
*******************************************************************************/
#include <solace/memoryManager.hpp>  // Class being tested

#include <solace/exception.hpp>
#include <cppunit/extensions/HelperMacros.h>


#include <unistd.h>


using namespace Solace;


class TestMemoryManager: public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestMemoryManager);
        CPPUNIT_TEST(testConstruction);
        CPPUNIT_TEST(testNativePageSize);
        CPPUNIT_TEST(testNativePageCount);
        CPPUNIT_TEST(testAllocation);
        CPPUNIT_TEST(testAllocationBeyondCapacity);
        CPPUNIT_TEST(testAllocationLocking);
    CPPUNIT_TEST_SUITE_END();

public:

    void testConstruction() {
        {   // NullPointer smoke test
//             CPPUNIT_ASSERT_THROW(Buffer nullbuffer(321, nullptr), IllegalArgumentException);
            MemoryManager nullManager(0);

            CPPUNIT_ASSERT_EQUAL(static_cast<MemoryManager::size_type>(0), nullManager.size());
            CPPUNIT_ASSERT_EQUAL(static_cast<MemoryManager::size_type>(0), nullManager.capacity());
        }
        {   // Fixed size constructor
            MemoryManager test(1024);

            CPPUNIT_ASSERT(test.empty());
            CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(1024), test.capacity());
            CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(0), test.size());
            CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(1024), test.capacity());

        }
    }

    void testNativePageSize() {
        MemoryManager test(1024);

        CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(getpagesize()), test.getPageSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(sysconf(_SC_PAGESIZE)), test.getPageSize());
    }


    void testNativePageCount() {
        MemoryManager test(1024);

        CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(sysconf(_SC_PHYS_PAGES)), test.getNbPages());

        #ifdef SOLACE_PLATFORM_LINUX
        // NOTE: This is a pretty stupid test as number of avaliable pages changes all the time!
        CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(sysconf(_SC_AVPHYS_PAGES)) / 1000,
                             test.getNbAvailablePages() / 1000);
        #endif
    }

    void testAllocation() {
        MemoryManager test(1024);

        {
            auto memBlock = test.create(128);
            CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(128), test.size());
            CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(128), memBlock.size());

            memBlock.view().fill(128);
            CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::value_type>(128), memBlock.view().last());
        }

        CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(0), test.size());
    }

    void testAllocationBeyondCapacity() {
        MemoryManager test(128);
        CPPUNIT_ASSERT_THROW(auto memBlock = test.create(2048), OverflowException);
        {
            auto memBlock0 = test.create(64);
            CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(64), test.size());
            CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(64), memBlock0.size());

            auto memBlock1 = test.create(64);
            CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(2*64), test.size());
            CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(64), memBlock1.size());

            CPPUNIT_ASSERT_THROW(auto memBlock2 = test.create(64), OverflowException);
        }

        CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(0), test.size());
    }


    void testAllocationLocking() {
        MemoryManager test(128);

        {
            CPPUNIT_ASSERT_EQUAL(false, test.isLocked());

            auto memBlock0 = test.create(64);
            CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(64), test.size());
            CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(64), memBlock0.size());

            // Lock allocation
            CPPUNIT_ASSERT_NO_THROW(test.lock());
            CPPUNIT_ASSERT_EQUAL(true, test.isLocked());

            // Create should throw as allocation is prohibited
            CPPUNIT_ASSERT_THROW(auto memBlock2 = test.create(64), Exception);

            CPPUNIT_ASSERT_NO_THROW(test.unlock());
            CPPUNIT_ASSERT_EQUAL(false, test.isLocked());
            CPPUNIT_ASSERT_NO_THROW(auto memBlock2 = test.create(64));
        }

        CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(0), test.size());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMemoryManager);
