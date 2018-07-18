#include <solace/memoryManager.hpp>  // Class being tested

#include <solace/exception.hpp>
#include <gtest/gtest.h>

#include <unistd.h>

using namespace Solace;

class TestMemoryManager: public ::testing::Test  {

public:

    void setUp() {
	}

    void tearDown() {
	}
};

TEST_F(TestMemoryManager, testConstruction) {
    {   // NullPointer smoke test
//             EXPECT_THROW(Buffer nullbuffer(321, nullptr), IllegalArgumentException);
        MemoryManager nullManager(0);

        EXPECT_EQ(static_cast<MemoryManager::size_type>(0), nullManager.size());
        EXPECT_EQ(static_cast<MemoryManager::size_type>(0), nullManager.capacity());
    }
    {   // Fixed size constructor
        MemoryManager test(1024);

        EXPECT_TRUE(test.empty());
        EXPECT_EQ(static_cast<MemoryView::size_type>(1024), test.capacity());
        EXPECT_EQ(static_cast<MemoryView::size_type>(0), test.size());
        EXPECT_EQ(static_cast<MemoryView::size_type>(1024), test.capacity());

    }
}

TEST_F(TestMemoryManager, testNativePageSize) {
    MemoryManager test(1024);

    EXPECT_EQ(static_cast<MemoryView::size_type>(getpagesize()), test.getPageSize());
    EXPECT_EQ(static_cast<MemoryView::size_type>(sysconf(_SC_PAGESIZE)), test.getPageSize());
}


TEST_F(TestMemoryManager, testNativePageCount) {
    MemoryManager test(1024);

    EXPECT_EQ(static_cast<MemoryView::size_type>(sysconf(_SC_PHYS_PAGES)), test.getNbPages());

    #ifdef SOLACE_PLATFORM_LINUX
    // NOTE: This is a pretty stupid test as number of avaliable pages changes all the time!
    EXPECT_EQ(static_cast<MemoryView::size_type>(sysconf(_SC_AVPHYS_PAGES)) / 1000,
                            test.getNbAvailablePages() / 1000);
    #endif
}

TEST_F(TestMemoryManager, testAllocation) {
    MemoryManager test(1024);

    {
        auto memBlock = test.create(128);
        EXPECT_EQ(static_cast<MemoryView::size_type>(128), test.size());
        EXPECT_EQ(static_cast<MemoryView::size_type>(128), memBlock.size());

        memBlock.view().fill(128);
        EXPECT_EQ(static_cast<MemoryView::value_type>(128), memBlock.view().last());
    }

    EXPECT_EQ(static_cast<MemoryView::size_type>(0), test.size());
}

TEST_F(TestMemoryManager, testAllocationBeyondCapacity) {
    MemoryManager test(128);
    EXPECT_THROW(auto memBlock = test.create(2048), OverflowException);
    {
        auto memBlock0 = test.create(64);
        EXPECT_EQ(static_cast<MemoryView::size_type>(64), test.size());
        EXPECT_EQ(static_cast<MemoryView::size_type>(64), memBlock0.size());

        auto memBlock1 = test.create(64);
        EXPECT_EQ(static_cast<MemoryView::size_type>(2*64), test.size());
        EXPECT_EQ(static_cast<MemoryView::size_type>(64), memBlock1.size());

        EXPECT_THROW(auto memBlock2 = test.create(64), OverflowException);
    }

    EXPECT_EQ(static_cast<MemoryView::size_type>(0), test.size());
}


TEST_F(TestMemoryManager, testAllocationLocking) {
    MemoryManager test(128);

    {
        EXPECT_EQ(false, test.isLocked());

        auto memBlock0 = test.create(64);
        EXPECT_EQ(static_cast<MemoryView::size_type>(64), test.size());
        EXPECT_EQ(static_cast<MemoryView::size_type>(64), memBlock0.size());

        // Lock allocation
        EXPECT_NO_THROW(test.lock());
        EXPECT_EQ(true, test.isLocked());

        // Create should throw as allocation is prohibited
        EXPECT_THROW(auto memBlock2 = test.create(64), Exception);

        EXPECT_NO_THROW(test.unlock());
        EXPECT_EQ(false, test.isLocked());
        EXPECT_NO_THROW(auto memBlock2 = test.create(64));
    }

    EXPECT_EQ(static_cast<MemoryView::size_type>(0), test.size());
}
