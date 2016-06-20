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

#include <cppunit/extensions/HelperMacros.h>
#include <fmt/format.h>


using namespace Solace;


class TestUUID : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestUUID);
        CPPUNIT_TEST(testStaticConstraints);
        CPPUNIT_TEST(testConstruction);
        CPPUNIT_TEST(testRandom);

        CPPUNIT_TEST(testComparable);
        CPPUNIT_TEST(testIterable);

        CPPUNIT_TEST(testFormattable);
        CPPUNIT_TEST(testParsable);

        CPPUNIT_TEST(testParsing_and_ToString_are_consistent);
        CPPUNIT_TEST(testContainerReq);
    CPPUNIT_TEST_SUITE_END();

protected:
    static constexpr size_t RandomSampleSize = 1000;


    UUID moveMe(const MemoryView& b) {
        return std::move(b);
    }

public:

    void testStaticConstraints() {
        CPPUNIT_ASSERT_EQUAL(static_cast<UUID::size_type>(16), UUID::StaticSize);
        CPPUNIT_ASSERT_EQUAL(static_cast<UUID::size_type>(36), UUID::StringSize);
        CPPUNIT_ASSERT_EQUAL(UUID::StaticSize, UUID::static_size());
    }

    void testRandom() {

        UUID ids[RandomSampleSize];

        for (uint i = 0; i < RandomSampleSize; ++i) {
            ids[i] = UUID::random();
        }

        for (uint i = 0; i < RandomSampleSize; ++i) {
            for (uint j = i + 1; j < RandomSampleSize; ++j) {
                CPPUNIT_ASSERT(ids[i] != ids[j]);
            }
        }
    }

    void testConstruction() {

        // Random UUID using default constructor
        UUID uid;
        CPPUNIT_ASSERT_EQUAL(UUID::StaticSize, uid.size());
        CPPUNIT_ASSERT_EQUAL(false, uid.isNull());

        // Copy construction
        UUID uid3(uid);
        CPPUNIT_ASSERT_EQUAL(uid, uid3);

        // Move construction
        {
            byte buff[] = {7, 5, 3, 4, 8, 6, 7, 8, 3, 7, 3, 4, 5, 6, 7, 8};

            UUID uid4(moveMe(MemoryView::wrap(buff, sizeof(buff))));
            for (UUID::size_type i = 0; i < sizeof(buff); ++i) {
                CPPUNIT_ASSERT_EQUAL(buff[i], uid4[i]);
            }
        }

        byte bytes[] = {1, 0, 3, 4, 5, 6, 7, 8, 1, 0, 3, 4, 5, 6, 7, 8};
        {
            UUID uid4x({1, 0, 3, 4, 5, 6, 7, 8, 1, 0, 3, 4, 5, 6, 7, 8});
            for (UUID::size_type i = 0; i < sizeof(bytes); ++i) {
                CPPUNIT_ASSERT_EQUAL(bytes[i], uid4x[i]);
            }
        }
        CPPUNIT_ASSERT_THROW(auto x = UUID({1, 0, 3, 4, 5, 6, 7, 8}), IllegalArgumentException);


        UUID uid5(MemoryView::wrap(bytes, sizeof(bytes), false));
        for (UUID::size_type i = 0; i < sizeof(bytes); ++i) {
            CPPUNIT_ASSERT_EQUAL(bytes[i], uid5[i]);
        }

        CPPUNIT_ASSERT_THROW(auto x = UUID(MemoryView::wrap(bytes, 7, false)), IllegalArgumentException);

        auto byteBuffer = ByteBuffer(16);
        byteBuffer.write(bytes, sizeof(bytes));

        CPPUNIT_ASSERT_EQUAL(static_cast<ByteBuffer::size_type>(0), byteBuffer.remaining());
        CPPUNIT_ASSERT_THROW(auto x = UUID(byteBuffer), IllegalArgumentException);

        byteBuffer.flip();
        UUID uid6(byteBuffer);
        for (UUID::size_type i = 0; i < sizeof(bytes); ++i) {
            CPPUNIT_ASSERT_EQUAL(bytes[i], uid6[i]);
        }

        auto wb = ByteBuffer(6);
        CPPUNIT_ASSERT_THROW(auto x = UUID(wb), IllegalArgumentException);
    }

    void testComparable() {
        CPPUNIT_ASSERT_EQUAL(
                    UUID({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}),
                    UUID({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}));

        CPPUNIT_ASSERT_EQUAL(false,
                    UUID({1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}) ==
                    UUID({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}));
    }

    void testIterable() {

        byte startValue = 15;
        UUID({15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0}).forEach([&startValue] (const byte& v) {
            CPPUNIT_ASSERT_EQUAL(startValue, v);
            startValue--;
        });

        CPPUNIT_ASSERT_EQUAL(static_cast<byte>(-1), startValue);

        CPPUNIT_ASSERT_THROW(UUID().operator [](UUID::StaticSize), IndexOutOfRangeException);
    }

    void testFormattable() {
        CPPUNIT_ASSERT_EQUAL(String("123e4567-e89b-12d3-a456-426655440000"),
                             UUID({0x12, 0x3e, 0x45, 0x67, 0xe8, 0x9b, 0x12, 0xd3,
                                   0xa4, 0x56, 0x42, 0x66, 0x55, 0x44, 0x0, 0x0})
                             .toString());

        CPPUNIT_ASSERT_EQUAL(String("00000000-0000-0000-0000-000000000000"),
                             UUID({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0})
                             .toString());

    }

    void testParsable() {
        CPPUNIT_ASSERT(UUID::parse("00000000-0000-0000-0000-000000000000").isNull());

        CPPUNIT_ASSERT_EQUAL(UUID({0x12, 0x3e, 0x45, 0x67, 0xe8, 0x9b, 0x12, 0xd3,
                                   0xa4, 0x56, 0x42, 0x66, 0x55, 0x44, 0x0, 0x0}),
                             UUID::parse(String("123e4567-e89b-12d3-a456-426655440000")));

        CPPUNIT_ASSERT_THROW(UUID::parse("SOMEHTING"), IllegalArgumentException);
        CPPUNIT_ASSERT_THROW(UUID::parse("1203045e-X054-Y000-3e3d-000000000000"), IllegalArgumentException);
    }

    void testParsing_and_ToString_are_consistent() {
        {
            UUID r0 = UUID::random();
            CPPUNIT_ASSERT_EQUAL(r0, UUID::parse(r0.toString()));
        }
        {
            UUID r0 = UUID::random();
            CPPUNIT_ASSERT_EQUAL(r0, UUID::parse(r0.toString()));
        }
        {
            UUID r0 = UUID::random();
            CPPUNIT_ASSERT_EQUAL(r0, UUID::parse(r0.toString()));
        }
        }

    void testContainerReq() {
        {
            Array<UUID> uids(2);
            CPPUNIT_ASSERT_EQUAL(UUID::StaticSize, uids[0].size());
            CPPUNIT_ASSERT_EQUAL(false, uids[0].isNull());
        }

        {
            Array<UUID> uids{
                UUID({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}),
                UUID({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}),
                UUID({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}),
                UUID({15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0})
            };

            CPPUNIT_ASSERT_EQUAL(false, uids[0].isNull());
            CPPUNIT_ASSERT_EQUAL(true,  uids[1].isNull());
            CPPUNIT_ASSERT_EQUAL(false, uids[2].isNull());
            CPPUNIT_ASSERT_EQUAL(false, uids[3].isNull());

            CPPUNIT_ASSERT_EQUAL(uids[2], uids[0]);

            for (UUID::size_type i = 0; i < UUID::static_size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(uids[3][UUID::static_size() - 1 - i], uids[0][i]);
            }
        }

    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestUUID);
