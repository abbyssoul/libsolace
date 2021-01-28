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
*******************************************************************************/
#include <solace/mutableMemoryView.hpp>  // Class being tested

#include <gtest/gtest.h>
#include "mockTypes.hpp"

using namespace Solace;

struct LargePodType {
    SimpleType i1, i2, i3;
};


TEST(TestMemoryView, testFill) {
    byte buff[48];
    MutableMemoryView buffer = wrapMemory(buff);

    buffer.fill(0);
    for (const auto& v : buffer) {
        EXPECT_EQ(0, v);
    }

    MutableMemoryView::size_type r = 0;
    buffer.fill(1);
    for (const auto& v : buffer) {
        EXPECT_EQ(1, v);
        r += v;
    }
    EXPECT_EQ(r, buffer.size());

    buffer.fill(64);
    for (const auto& v : buffer) {
        EXPECT_EQ(64, v);
    }

    buffer.fill(36, 20, 40);
    for (MutableMemoryView::size_type i = 0; i < buffer.size(); ++i) {
        if (i >= 20 && i < 40) {
            EXPECT_EQ(36, buffer[i]);
        } else {
            EXPECT_EQ(64, buffer[i]);
        }
    }

    // 'To' index is out of range
//    EXPECT_THROW(buffer.fill(3, 3, 130), IndexOutOfRangeException);
//    // 'From' index is out of range
//    EXPECT_THROW(buffer.fill(3, 100, 30), IndexOutOfRangeException);
//    // 'To' index is out of range - less then from
//    EXPECT_THROW(buffer.fill(3, 30, 3), IndexOutOfRangeException);
//    // 'From' and 'To' index is out of range
//    EXPECT_THROW(buffer.fill(3, 100, 130), IndexOutOfRangeException);
}


TEST(TestMemoryView, testWrapping) {
    void* nullB = nullptr;
    EXPECT_TRUE(wrapMemory(nullB, 0).empty());
     // Can't wrap nullptr with non-zero size
    EXPECT_THROW(wrapMemory(nullB, 321).empty(), Exception);

    {   // Wrapping constructor
        byte example[] = {0, 1, 0, 3, 2, 1};  // size = 6
        auto test = wrapMemory(example);

        EXPECT_TRUE(!test.empty());
		EXPECT_EQ(6U, test.size());

		auto b = test.begin();
		for (MutableMemoryView::size_type i = 0; i < test.size(); ++i, ++b) {
			EXPECT_EQ(example[i], *b);
        }
    }

    {   // Wrapping constructor with specific size
        byte example[] = {0, 1, 0, 3, 2, 1};  // size = 6
        auto test = wrapMemory(example, 4);

        EXPECT_TRUE(!test.empty());
		EXPECT_EQ(4U, test.size());

		auto b = test.begin();
		for (MutableMemoryView::size_type i = 0; i < test.size(); ++i, ++b) {
			EXPECT_EQ(example[i], *b);
        }
    }
}


TEST(TestMemoryView, testConstruction) {
    {   // Fixed size constructor
        byte buff[3102];
        MutableMemoryView test = wrapMemory(buff);

        EXPECT_TRUE(!test.empty());
		EXPECT_EQ(3102U, test.size());
        test[0] = 19;
        test[2] = 17;
        test[1] = 4;
        test[test.size() - 1] = 255;
		EXPECT_EQ(19, test[0]);
		EXPECT_EQ(4, test[1]);
		EXPECT_EQ(17, test[2]);
		EXPECT_EQ(255, test[test.size() - 1]);
    }


    {   // Move-constructor
        byte example[] = {7, 10, 13, 16, 19, 22, 25};  // size = 7
        const MutableMemoryView::size_type exampleSize = sizeof(example);
        auto b1 = wrapMemory(example);
        {
			MutableMemoryView b2(mv(b1));

			EXPECT_EQ(0U, b1.size());
            EXPECT_EQ(exampleSize, b2.size());

            for (MutableMemoryView::size_type i = 0; i < b2.size(); ++i) {
                EXPECT_EQ(example[i], b2[i]);
            }
        }

        // Test that after b2 has been destroyed - the memory is still valid.
        for (MutableMemoryView::size_type i = 0; i < b1.size(); ++i) {
            EXPECT_EQ(7 + 3*i, b1[i]);
        }
    }
}


TEST(TestMemoryView, testRead) {
    byte b1[128], b2[24];
    MutableMemoryView buffer = wrapMemory(b1);
    MutableMemoryView dest = wrapMemory(b2);

    dest.fill(0);
    buffer.fill(64);
    {
        // Test simple read
		ASSERT_TRUE(buffer.read(dest));
        for (auto b : dest) {
            EXPECT_EQ(64, b);
        }

        // Test that source is independent of dest
        buffer.fill(76);
        for (auto b : dest) {
            EXPECT_EQ(64, b);
        }
    }

    dest.fill(0);
    {  // Safety checks
        // Reading more then bytes in the buffer
		EXPECT_TRUE(buffer.slice(0, 10).read(dest).isError());

        // Reading more then fits into the dest
		EXPECT_TRUE(dest.read(buffer).isError());
    }

	EXPECT_TRUE(buffer.slice(buffer.size(), 3).read(dest).isError());
}


TEST(TestMemoryView, testReadingPastTheSize) {
    byte src[15];

	auto buffer = wrapMemory(src);
	EXPECT_TRUE(buffer.offsetAddress(16).isNone());
    EXPECT_THROW(buffer[1042], IndexOutOfRangeException);
}


TEST(TestMemoryView, testDataAs) {
    byte src[sizeof(SimpleType) + 5];
	auto buffer = wrapMemory(src);

    int tx, ty, tz;

    tx = 1; ty = 3; tz = 2;

	size_t const offs[] = {sizeof(tx), sizeof(tx) + sizeof(ty), sizeof(tx) + sizeof(ty) + sizeof(tz)};
	ASSERT_TRUE(buffer.slice(0, offs[0]).write(wrapMemory(&tx, sizeof(tx))));
	ASSERT_TRUE(buffer.slice(offs[0], offs[1]).write(wrapMemory(&ty, sizeof(ty))));
	ASSERT_TRUE(buffer.slice(offs[1], offs[2]).write(wrapMemory(&tz, sizeof(tz))));

	SimpleType& uuid1 = buffer.dataAs<SimpleType>();
	EXPECT_EQ(SimpleType(1, 3, 2), uuid1);

    tx = 7; ty = 44; tz = -32;
	ASSERT_TRUE(buffer.slice(0, offs[0]).write(wrapMemory(&tx, sizeof(tx))));
	ASSERT_TRUE(buffer.slice(offs[0], offs[1]).write(wrapMemory(&ty, sizeof(ty))));
	ASSERT_TRUE(buffer.slice(offs[1], offs[2]).write(wrapMemory(&tz, sizeof(tz))));
	EXPECT_EQ(SimpleType(7, 44, -32), uuid1);


    tx = -91; ty = 12; tz = 0;
	ASSERT_TRUE(buffer.slice(4, 4 + offs[0]).write(wrapMemory(&tx, sizeof(tx))));
	ASSERT_TRUE(buffer.slice(4 + offs[0], 4 + offs[1]).write(wrapMemory(&ty, sizeof(ty))));
	ASSERT_TRUE(buffer.slice(4 + offs[1], 4 + offs[2]).write(wrapMemory(&tz, sizeof(tz))));

	SimpleType const& uuid2 = buffer.dataAs<SimpleType>(4);
	EXPECT_EQ(SimpleType(-91, 12, 0), uuid2);

	EXPECT_THROW(buffer.dataAs<SimpleType>(6), Exception);
	EXPECT_THROW(buffer.dataAs<LargePodType>(), Exception);
}


TEST(TestMemoryView, dataAsWhenNoRoom) {
	byte src2[sizeof(LargePodType) / 2];
	auto buffer = wrapMemory(src2);
	EXPECT_ANY_THROW(buffer.dataAs<LargePodType>());
}


TEST(TestMemoryView, testWrite) {
    byte b1[128], b2[24];
    MutableMemoryView buffer = wrapMemory(b1);
    MutableMemoryView src = wrapMemory(b2);

    src.fill(32);
    buffer.fill(0);

    {  // Identity writing:
		ASSERT_TRUE(buffer.write(buffer));
        for (MutableMemoryView::size_type i = 0; i < buffer.size(); ++i) {
            EXPECT_EQ(0, buffer[i]);
        }
    }
    {
        // Test simple read
		ASSERT_TRUE(buffer.write(src));
        for (MutableMemoryView::size_type i = 0; i < src.size(); ++i) {
            EXPECT_EQ(32, buffer[i]);
        }
        for (MutableMemoryView::size_type i = src.size(); i < buffer.size(); ++i) {
            EXPECT_EQ(0, buffer[i]);
        }

        // Test that source is independent of dest
        src.fill(76);
        for (MutableMemoryView::size_type i = 0; i < src.size(); ++i) {
            EXPECT_EQ(32, buffer[i]);
        }
        for (MutableMemoryView::size_type i = src.size(); i < buffer.size(); ++i) {
            EXPECT_EQ(0, buffer[i]);
        }
    }

    {  // Safety checks

        // Writing more then fits into the buffer
		EXPECT_TRUE(src.write(buffer).isError());

        // Reading from invalid offset
		EXPECT_TRUE(buffer.slice(buffer.size() + 1, 10).write(src).isError());
    }

}


TEST(TestMemoryView, testZeroSizedSlice) {
    byte src[24];
    auto buffer = wrapMemory(src);

	EXPECT_EQ(0U, buffer.slice(3, 3).size());
	EXPECT_EQ(0U, buffer.slice(512, 512).size());

	EXPECT_EQ(0U, MemoryView().slice(0, 0).size());
	EXPECT_EQ(0U, MemoryView().slice(312, 312).size());
	EXPECT_EQ(0U, MutableMemoryView().slice(0, 0).size());
	EXPECT_EQ(0U, MutableMemoryView().slice(10, 10).size());
}


TEST(TestMemoryView, testSlice) {
    byte src[64];

    byte i = 0;
    for (auto& a : src) {
        a = i++;
    }

    auto buffer = wrapMemory(src);
    auto slice = buffer.slice(32, buffer.size());
	EXPECT_EQ(32U, slice.size());
    EXPECT_EQ(32, slice[0]);
    EXPECT_EQ(63, slice[31]);


    EXPECT_EQ(buffer, buffer.slice(0, buffer.size()));
    EXPECT_TRUE(buffer.slice(3, 3).empty());
    EXPECT_TRUE(buffer.slice(128, 256).empty());
    EXPECT_TRUE(buffer.slice(128, 2).empty());
    EXPECT_TRUE(buffer.slice(32, 2).empty());
}


TEST(TestMemoryView, testGreadySlice) {
    byte src[64];
    byte i = 0;
    for (auto& a : src) { a = i++; }

    // Overslicing
    {
        MutableMemoryView buffer = wrapMemory(src);
        auto slice = buffer.slice(0, 256);
		EXPECT_EQ(64U, slice.size());
        EXPECT_EQ(0, slice[0]);
        EXPECT_EQ(31, slice[31]);
        EXPECT_EQ(63, slice[63]);
    }

    {
        MemoryView buffer = wrapMemory(src);
        auto slice = buffer.slice(0, 256);
		EXPECT_EQ(64U, slice.size());
        EXPECT_EQ(0, slice[0]);
        EXPECT_EQ(31, slice[31]);
        EXPECT_EQ(63, slice[63]);
    }
}


TEST(TestMemoryView, testPlacementConstruct) {
    {
        byte src[3];
        auto buffer = wrapMemory(src);
		EXPECT_THROW(buffer.construct<SimpleType>(4, -2, 12), Exception);
        EXPECT_EQ(0, SimpleType::InstanceCount);
    }

    {
        byte src[sizeof(SimpleType)];
        auto buffer = wrapMemory(src);
        auto simpleInstance = buffer.construct<SimpleType>(4, -2, 12);

        EXPECT_EQ(-2, simpleInstance->y);
        EXPECT_EQ(1, SimpleType::InstanceCount);

        buffer.destruct<SimpleType>();
        EXPECT_EQ(0, SimpleType::InstanceCount);
    }
}
