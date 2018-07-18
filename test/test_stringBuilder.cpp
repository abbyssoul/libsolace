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
        return { _memoryManager.create(strlen(someConstString)), someConstString };
	}

public:

    TestStringBuilder(): _memoryManager(4096)
    {
    }

    void setUp()  {
	}

    void tearDown()  {
	}
};

TEST_F(TestStringBuilder, testNullString) {
    EXPECT_THROW(const StringBuilder nullString(_memoryManager.create(5), nullptr),
                            Exception);
}

/**
    * Test construction calls
    */
TEST_F(TestStringBuilder, testConstruction) {
    const StringView constStr { someConstString };

    {   // empty buffer usage
        const StringBuilder empty(_memoryManager.create(0));  // No throw?

        EXPECT_TRUE(empty.empty());
        EXPECT_TRUE(empty.toString().empty());
    }

    {   // Empty string post-conditions
        const StringBuilder empty(_memoryManager.create(2 * constStr.size()));  // No throw?

        EXPECT_TRUE(empty.empty());
        EXPECT_TRUE(empty.toString().empty());
    }

    {
        const StringBuilder sb(_memoryManager.create(strlen(someConstString)), someConstString);

        EXPECT_TRUE(!sb.empty());
        EXPECT_EQ(constStr, sb.view());
    }

    {
        const StringBuilder sb(_memoryManager.create(2 * constStr.size()), constStr);

        EXPECT_TRUE(!sb.empty());
        EXPECT_EQ(constStr, sb.view());
    }

    {  // Test move construction
        const StringBuilder sb = moveMe();

        EXPECT_TRUE(!sb.empty());
        EXPECT_EQ(constStr, sb.view());
    }
}

/**
    * Test toString conversion
    */
TEST_F(TestStringBuilder, testToString) {
    const StringBuilder ident(_memoryManager.create(strlen(someConstString)), someConstString);

    const auto str = ident.toString();

    EXPECT_EQ(str, ident.toString());
    EXPECT_EQ(String(someConstString), str);
}

const char* TestStringBuilder::someConstString = "Some static string";