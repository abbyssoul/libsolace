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
*******************************************************************************/
#include <solace/memoryManager.hpp>  // Class being tested

#include <solace/exception.hpp>
#include <gtest/gtest.h>

#include <unistd.h>

using namespace Solace;


TEST(TestMemoryManager, testConstruction) {
    {   // NullPointer smoke test
//             EXPECT_THROW(Buffer nullbuffer(321, nullptr), IllegalArgumentException);
        MemoryManager nullManager(0);

        EXPECT_EQ(0, nullManager.size());
        EXPECT_EQ(0, nullManager.capacity());
    }
    {   // Fixed size constructor
        MemoryManager test(1024);

        EXPECT_TRUE(test.empty());
        EXPECT_EQ(1024, test.capacity());
        EXPECT_EQ(0, test.size());
        EXPECT_EQ(1024, test.capacity());

    }
}

TEST(TestMemoryManager, testNativePageSize) {
    MemoryManager test(1024);

    EXPECT_EQ(getpagesize(), test.getPageSize());
    EXPECT_EQ(sysconf(_SC_PAGESIZE), test.getPageSize());
}


TEST(TestMemoryManager, testNativePageCount) {
    MemoryManager test(1024);

    EXPECT_EQ(sysconf(_SC_PHYS_PAGES), test.getNbPages());

    #ifdef SOLACE_PLATFORM_LINUX
    // NOTE: This is a pretty stupid test as number of avaliable pages changes all the time!
    EXPECT_EQ(sysconf(_SC_AVPHYS_PAGES) / 1000,
                            test.getNbAvailablePages() / 1000);
    #endif
}

TEST(TestMemoryManager, testAllocation) {
    MemoryManager test{512};

    {
		auto maybeBlock = test.allocate(512);
		ASSERT_TRUE(maybeBlock.isOk());

		auto& memBlock = maybeBlock.unwrap();
		EXPECT_EQ(512, memBlock.size());
        EXPECT_EQ(512, test.size());
        EXPECT_EQ(0, test.limit());

        memBlock.view().fill(128);
        EXPECT_EQ(128, memBlock.view()[memBlock.size() - 1]);
    }

    EXPECT_EQ(512, test.limit());
    EXPECT_EQ(0, test.size());
}

TEST(TestMemoryManager, testAllocationBeyondCapacity) {
	MemoryManager test{128};


	EXPECT_TRUE(test.allocate(2048).isError());
    {
		auto maybeBlock0 = test.allocate(64);
		ASSERT_TRUE(maybeBlock0.isOk());
		auto& memBlock0 = maybeBlock0.unwrap();

		EXPECT_EQ(64, test.size());
        EXPECT_EQ(64, memBlock0.size());

		auto maybeBlock1 = test.allocate(64);
		ASSERT_TRUE(maybeBlock1.isOk());
		auto& memBlock1 = maybeBlock1.unwrap();

		EXPECT_EQ(2*64, test.size());
        EXPECT_EQ(64, memBlock1.size());

		EXPECT_TRUE(test.allocate(64).isError());
    }

    EXPECT_EQ(128, test.limit());
    EXPECT_EQ(0, test.size());
}


TEST(TestMemoryManager, testAllocationLocking) {
    MemoryManager test(128);

    {
        EXPECT_EQ(false, test.isLocked());

		auto maybeBlock0 = test.allocate(64);
		ASSERT_TRUE(maybeBlock0.isOk());
		auto& memBlock0 = maybeBlock0.unwrap();

        EXPECT_EQ(64, test.size());
        EXPECT_EQ(64, memBlock0.size());

        // Lock allocation
        EXPECT_NO_THROW(test.lock());
        EXPECT_EQ(true, test.isLocked());

		// Allocation should fail as it is prohibited/locked
		EXPECT_TRUE(test.allocate(64).isError());

        EXPECT_NO_THROW(test.unlock());
        EXPECT_EQ(false, test.isLocked());
		EXPECT_TRUE(test.allocate(64).isOk());
    }

    EXPECT_EQ(0, test.size());
}
