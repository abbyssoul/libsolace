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
 * @author: soultaker
 *
 * Created on: 23 May 2016
*******************************************************************************/
#include <solace/uuid.hpp>			// Class being tested
#include <solace/array.hpp>
#include <solace/exception.hpp>

#include <gtest/gtest.h>


using namespace Solace;

static constexpr size_t RandomSampleSize = 100;


TEST(TestUUID, testStaticConstraints) {
    EXPECT_EQ(static_cast<UUID::size_type>(16), UUID::StaticSize);
    EXPECT_EQ(static_cast<UUID::size_type>(36), UUID::StringSize);
}

TEST(TestUUID, testRandom) {
    UUID ids[RandomSampleSize];

    for (auto& id : ids) {
        id = UUID::random();
    }

    for (uint i = 0; i < RandomSampleSize; ++i) {
        for (uint j = i + 1; j < RandomSampleSize; ++j) {
            EXPECT_TRUE(ids[i] != ids[j]);
        }
    }
}

TEST(TestUUID, testConstruction) {

    // Random UUID using default constructor
    UUID uid;
    EXPECT_EQ(UUID::StaticSize, uid.size());
    EXPECT_EQ(false, uid.isNull());

    // Copy construction
    UUID uid3(uid);
    EXPECT_EQ(uid, uid3);

    // Move construction
    {
        byte buff[] = {7, 5, 3, 4, 8, 6, 7, 8, 3, 7, 3, 4, 5, 6, 7, 8};

        UUID uid4(wrapMemory(buff, sizeof(buff)));
        for (UUID::size_type i = 0; i < sizeof(buff); ++i) {
            EXPECT_EQ(buff[i], uid4[i]);
        }
    }

    byte bytes[] = {1, 0, 3, 4, 5, 6, 7, 8, 1, 0, 3, 4, 5, 6, 7, 8};
    {
        UUID uid4x({1, 0, 3, 4, 5, 6, 7, 8, 1, 0, 3, 4, 5, 6, 7, 8});
        for (UUID::size_type i = 0; i < sizeof(bytes); ++i) {
            EXPECT_EQ(bytes[i], uid4x[i]);
        }
    }
    EXPECT_THROW(auto x = UUID({1, 0, 3, 4, 5, 6, 7, 8}), IllegalArgumentException);


    UUID uid5(wrapMemory(bytes, sizeof(bytes)));
    for (UUID::size_type i = 0; i < sizeof(bytes); ++i) {
        EXPECT_EQ(bytes[i], uid5[i]);
    }

    EXPECT_THROW(auto x = UUID(wrapMemory(bytes, 7)), IllegalArgumentException);
}

TEST(TestUUID, testComparable) {
    EXPECT_EQ(
                UUID({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}),
                UUID({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}));

    EXPECT_EQ(false,
                UUID({1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}) ==
                UUID({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}));
}

TEST(TestUUID, testIterable) {

    byte startValue = 15;
    UUID uuid({15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0});

    for (auto v : uuid) {
        EXPECT_EQ(startValue, v);
        --startValue;
    }
    EXPECT_EQ(static_cast<byte>(-1), startValue);

    EXPECT_THROW(UUID().operator [](UUID::StaticSize), IndexOutOfRangeException);
}

TEST(TestUUID, testFormattable) {
    EXPECT_EQ(StringView("123e4567-e89b-12d3-a456-426655440000"),
                            UUID({0x12, 0x3e, 0x45, 0x67, 0xe8, 0x9b, 0x12, 0xd3,
                                0xa4, 0x56, 0x42, 0x66, 0x55, 0x44, 0x0, 0x0})
                            .toString());

    EXPECT_EQ(StringView("00000000-0000-0000-0000-000000000000"),
                            UUID({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0})
                            .toString());

}

TEST(TestUUID, testParsable) {
    auto nullParseResult = UUID::parse("00000000-0000-0000-0000-000000000000");
    EXPECT_TRUE(nullParseResult.isOk());
    EXPECT_TRUE(nullParseResult.unwrap().isNull());

    EXPECT_EQ(UUID({0x12, 0x3e, 0x45, 0x67, 0xe8, 0x9b, 0x12, 0xd3,
                                0xa4, 0x56, 0x42, 0x66, 0x55, 0x44, 0x0, 0x0}),
                            UUID::parse("123e4567-e89b-12d3-a456-426655440000").unwrap());

    EXPECT_TRUE(UUID::parse("SOMEHTING").isError());
    EXPECT_TRUE(UUID::parse("1203045e-X054-Y000-3e3d-000000000000").isError());
}

TEST(TestUUID, testParsing_and_ToString_are_consistent) {
    for (uint i = 0; i < RandomSampleSize; ++i) {
        UUID r0 = UUID::random();
        auto parseResult = UUID::parse(r0.toString().view());
        EXPECT_TRUE(parseResult.isOk());
        EXPECT_EQ(r0, parseResult.unwrap());
    }
}

TEST(TestUUID, testContainerReq) {
    {
        std::vector<UUID> uids(2);
        EXPECT_EQ(UUID::StaticSize, uids[0].size());
        EXPECT_EQ(false, uids[0].isNull());
    }

    {
        std::vector<UUID> uids{
            UUID({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}),
            UUID({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}),
            UUID({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}),
            UUID({15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0})
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
