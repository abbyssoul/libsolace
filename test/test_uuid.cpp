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
 * @file: test/test_uuid.cpp
*******************************************************************************/
#include <solace/uuid.hpp>			// Class being tested
#include <solace/array.hpp>
#include <solace/exception.hpp>

#include <gtest/gtest.h>


using namespace Solace;

static constexpr uint32 RandomSampleSize = 100;


TEST(TestUUID, testStaticConstraints) {
	EXPECT_EQ(16U, UUID::StaticSize);
	EXPECT_EQ(36U, UUID::StringSize);
}

TEST(TestUUID, testRandom) {
    UUID ids[RandomSampleSize];

    for (auto& id : ids) {
        id = makeRandomUUID();
    }

    for (uint i = 0; i < RandomSampleSize; ++i) {
        for (uint j = i + 1; j < RandomSampleSize; ++j) {
			EXPECT_NE(ids[i], ids[j]);
        }
    }
}


TEST(TestUUID, testConstruction) {

    // Random UUID using default constructor
    UUID uid;
    EXPECT_TRUE(uid.isNull());
    EXPECT_EQ(UUID::StaticSize, uid.size());

    // Copy construction
    UUID uid3(uid);
    EXPECT_EQ(uid, uid3);

    // Construct from a byte array
    {
        byte buff[] = {7, 5, 3, 4, 8, 6, 7, 8, 3, 7, 3, 4, 5, 6, 7, 8};
        auto const uid4 = UUID{buff};
        for (UUID::size_type i = 0; i < sizeof(buff); ++i) {
            EXPECT_EQ(buff[i], uid4[i]);
        }
    }

    {
        byte const bytes[] = {1, 0, 3, 4, 5, 6, 7, 8, 1, 0, 3, 4, 5, 6, 7, 8};
        auto const uid4x = UUID{bytes};
        for (UUID::size_type i = 0; i < sizeof(bytes); ++i) {
            EXPECT_EQ(bytes[i], uid4x[i]);
        }
    }
}


TEST(TestUUID, testMakeUUIDwithInsufficientData) {
    byte const fewBytes[] = {1, 0, 3, 4, 5, 6, 7, 8};
	EXPECT_THROW([[maybe_unused]] auto unused = makeUUID(wrapMemory(fewBytes)), Exception);

    byte const bytes[] = {1, 0, 3, 4, 5, 6, 7, 8, 1, 0, 3, 4, 5, 6, 7, 8};
	EXPECT_THROW([[maybe_unused]] auto const x = makeUUID(wrapMemory(bytes, 7)), Exception);
}


TEST(TestUUID, testComparable) {
    byte const bytes[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    auto const memView = wrapMemory(bytes);

    // Same source - same value:
    EXPECT_EQ(UUID{bytes}, makeUUID(memView));

    byte const otherBytes[] = {1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    EXPECT_NE(UUID{bytes}, makeUUID(wrapMemory(otherBytes)));
}


TEST(TestUUID, testIterable) {

    int startValue = 15;
    byte const bytes[] = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    auto const uuid = UUID{bytes};

    for (auto v : uuid) {
        EXPECT_EQ(startValue, v);
        --startValue;
    }
    EXPECT_EQ(-1, startValue);
}


TEST(TestUUID, testIndexOpThrowsOnIndexOutOfRange) {
    EXPECT_THROW(UUID().operator [](UUID::StaticSize), IndexOutOfRangeException);
}

TEST(TestUUID, testFormattable) {
    byte const bytes[] = {0x12, 0x3e, 0x45, 0x67, 0xe8, 0x9b, 0x12, 0xd3,
                          0xa4, 0x56, 0x42, 0x66, 0x55, 0x44, 0x0, 0x0};

    EXPECT_EQ(StringView("123e4567-e89b-12d3-a456-426655440000"),
                            UUID(bytes).toString());

    byte const zeros[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    EXPECT_EQ(StringView("00000000-0000-0000-0000-000000000000"),
                            UUID(zeros).toString());

}

TEST(TestUUID, testParsable) {
    auto nullParseResult = UUID::parse("00000000-0000-0000-0000-000000000000");
    EXPECT_TRUE(nullParseResult.isOk());
    EXPECT_TRUE(nullParseResult.unwrap().isNull());

    byte const bytes[] = {0x12, 0x3e, 0x45, 0x67, 0xe8, 0x9b, 0x12, 0xd3,
                          0xa4, 0x56, 0x42, 0x66, 0x55, 0x44, 0x0, 0x0};
    EXPECT_EQ(UUID(bytes),
              UUID::parse("123e4567-e89b-12d3-a456-426655440000").unwrap());

    EXPECT_TRUE(UUID::parse("SOMEHTING").isError());
    EXPECT_TRUE(UUID::parse("1203045e-X054-Y000-3e3d-000000000000").isError());
}


TEST(TestUUID, testParsing_and_ToString_are_consistent) {
    for (uint i = 0; i < RandomSampleSize; ++i) {
        auto const r0 = makeRandomUUID();
        auto parseResult = UUID::parse(r0.toString().view());
        EXPECT_TRUE(parseResult.isOk());
        EXPECT_EQ(r0, parseResult.unwrap());
    }
}


TEST(TestUUID, testContainerReq) {
    {
        std::vector<UUID> uids(2);
        EXPECT_EQ(UUID::StaticSize, uids[0].size());
        EXPECT_TRUE(uids[0].isNull());
        EXPECT_TRUE(uids[1].isNull());
    }


    {
        byte const bytes[][16] = {
            {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
            {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0}
        };

        auto const uids = std::vector<UUID> {
                    bytes[0],
                    bytes[1],
                    bytes[2],
                    bytes[3]
                };

        EXPECT_EQ(false, uids[0].isNull());
        EXPECT_EQ(true,  uids[1].isNull());
        EXPECT_EQ(false, uids[2].isNull());
        EXPECT_EQ(false, uids[3].isNull());

        EXPECT_EQ(uids[2], uids[0]);

        for (UUID::size_type i = 0; i < UUID::StaticSize; ++i) {
            EXPECT_EQ(uids[3][UUID::StaticSize - 1 - i], uids[0][i]);
        }
    }
}
