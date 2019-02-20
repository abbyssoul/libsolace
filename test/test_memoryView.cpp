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
    EXPECT_THROW(buffer.fill(3, 3, 130), IndexOutOfRangeException);
    // 'From' index is out of range
    EXPECT_THROW(buffer.fill(3, 100, 30), IndexOutOfRangeException);
    // 'To' index is out of range - less then from
    EXPECT_THROW(buffer.fill(3, 30, 3), IndexOutOfRangeException);
    // 'From' and 'To' index is out of range
    EXPECT_THROW(buffer.fill(3, 100, 130), IndexOutOfRangeException);
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
        EXPECT_EQ(6, test.size());

        for (MutableMemoryView::size_type i = 0; i < test.size(); ++i) {
            EXPECT_EQ(example[i], test.dataAddress()[i]);
        }
    }

    {   // Wrapping constructor with specific size
        byte example[] = {0, 1, 0, 3, 2, 1};  // size = 6
        auto test = wrapMemory(example, 4);

        EXPECT_TRUE(!test.empty());
        EXPECT_EQ(4, test.size());

        for (MutableMemoryView::size_type i = 0; i < test.size(); ++i) {
            EXPECT_EQ(example[i], test.dataAddress()[i]);
        }
    }
}


TEST(TestMemoryView, testConstruction) {
    {   // Fixed size constructor
        byte buff[3102];
        MutableMemoryView test = wrapMemory(buff);

        EXPECT_TRUE(!test.empty());
        EXPECT_EQ(3102, test.size());
        test[0] = 19;
        test[2] = 17;
        test[1] = 4;
        test[test.size() - 1] = 255;
        EXPECT_EQ(19, test.dataAddress()[0]);
        EXPECT_EQ(4, test.dataAddress()[1]);
        EXPECT_EQ(17, test.dataAddress()[2]);
        EXPECT_EQ(255, test.dataAddress()[test.size() - 1]);
    }


    {   // Move-constructor
        byte example[] = {7, 10, 13, 16, 19, 22, 25};  // size = 7
        const MutableMemoryView::size_type exampleSize = sizeof(example);
        auto b1 = wrapMemory(example);
        {
            MutableMemoryView b2(std::move(b1));

            EXPECT_EQ(0, b1.size());
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
        buffer.read(dest);
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
        EXPECT_THROW(buffer.read(dest, 2*buffer.size()), Exception);

        // Reading more then fits into the dest
        EXPECT_THROW(dest.read(buffer), Exception);
        EXPECT_THROW(buffer.read(dest, 2*dest.size()), Exception);

        // Reading from invalid offset
        EXPECT_THROW(buffer.read(dest, 1, buffer.size() + 10), Exception);

        // Reading from invalid offset and too much
        EXPECT_THROW(buffer.read(dest, 2*dest.size(), buffer.size() + 10), Exception);
    }

    {  // Test reading from an offset
        buffer.fill(67, 0, 24);
        buffer.fill(76, 24, buffer.size());

        buffer.read(dest, 24);
        for (auto b : dest) {
            EXPECT_EQ(67, b);
        }

        // Test that source is independent of dest
        buffer.read(dest, 24, 24);
        for (auto b : dest) {
            EXPECT_EQ(76, b);
        }
    }
}


TEST(TestMemoryView, testReadingPastTheSize) {
    byte src[15];

    auto&& buffer = wrapMemory(src);
    EXPECT_THROW(buffer[1042], IndexOutOfRangeException);
    EXPECT_THROW(buffer.dataAddress(16), IndexOutOfRangeException);
}


TEST(TestMemoryView, testDataAs) {
    byte src[sizeof(SimpleType) + 5];

    byte n = 0;
    for (auto& b : src) {
        b = ++n;
    }

    auto&& buffer = wrapMemory(src);

    int tx, ty, tz;

    tx = 1; ty = 3; tz = 2;
    buffer.write(wrapMemory(&tx, sizeof(tx)));
    buffer.write(wrapMemory(&ty, sizeof(tx)), sizeof(tx));
    buffer.write(wrapMemory(&tz, sizeof(tx)), 2*sizeof(ty));

    SimpleType* uuid1 = buffer.dataAs<SimpleType>();
    EXPECT_EQ(SimpleType(1, 3, 2), *uuid1);

    tx = 7; ty = 44; tz = -32;
    buffer.write(wrapMemory(&tx, sizeof(tx)));
    buffer.write(wrapMemory(&ty, sizeof(tx)), sizeof(tx));
    buffer.write(wrapMemory(&tz, sizeof(tx)), 2*sizeof(ty));
    EXPECT_EQ(SimpleType(7, 44, -32), *uuid1);

    SimpleType* uuid2 = buffer.dataAs<SimpleType>(4);
    tx = -91; ty = 12; tz = 0;
    buffer.write(wrapMemory(&tx, sizeof(tx)), 4);
    buffer.write(wrapMemory(&ty, sizeof(tx)), 4 + sizeof(tx));
    buffer.write(wrapMemory(&tz, sizeof(tx)), 4 + 2*sizeof(ty));
    EXPECT_EQ(SimpleType(-91, 12, 0), *uuid2);

    EXPECT_THROW(buffer.dataAs<SimpleType>(6), IndexOutOfRangeException);
    EXPECT_THROW(buffer.dataAs<LargePodType>(), IndexOutOfRangeException);


    byte src2[sizeof(LargePodType)];
    auto buffer2 = wrapMemory(src2);
    EXPECT_NO_THROW(buffer2.dataAs<LargePodType>());
}


TEST(TestMemoryView, testWrite) {
    byte b1[128], b2[24];
    MutableMemoryView buffer = wrapMemory(b1);
    MutableMemoryView src = wrapMemory(b2);

    src.fill(32);
    buffer.fill(0);

    {  // Identity writing:
        buffer.write(buffer);
        for (MutableMemoryView::size_type i = 0; i < buffer.size(); ++i) {
            EXPECT_EQ(0, buffer[i]);
        }
    }
    {
        // Test simple read
        buffer.write(src);
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
        EXPECT_THROW(src.write(buffer), Exception);

        // Reading from invalid offset
        EXPECT_THROW(buffer.write(src, buffer.size() + 10), Exception);

        // Reading from invalid offset and too much
        EXPECT_THROW(buffer.write(src, buffer.size() - src.size() + 2), Exception);
    }

    {  // Test reading from an offset
        src.fill(41);
        buffer.fill(67, 0, 24);
        buffer.fill(76, 24, buffer.size());

        buffer.write(src, 24);
        for (MutableMemoryView::size_type i = 0; i < 24; ++i) {
            EXPECT_EQ(67, buffer[i]);
        }
        for (MutableMemoryView::size_type i = 24; i < 24 + src.size(); ++i) {
            EXPECT_EQ(41, buffer[i]);
        }
        for (MutableMemoryView::size_type i = 24 + src.size(); i < buffer.size(); ++i) {
            EXPECT_EQ(76, buffer[i]);
        }

        src.fill(71);
        buffer.write(src, 14);
        for (MutableMemoryView::size_type i = 0; i < 14; ++i) {
            EXPECT_EQ(67, buffer[i]);
        }
        for (MutableMemoryView::size_type i = 14; i < 14 + src.size(); ++i) {
            EXPECT_EQ(71, buffer[i]);
        }
        for (MutableMemoryView::size_type i = 24 + src.size(); i < buffer.size(); ++i) {
            EXPECT_EQ(76, buffer[i]);
        }
    }
}


TEST(TestMemoryView, testZeroSizedSlice) {
    byte src[24];
    auto buffer = wrapMemory(src);

    EXPECT_EQ(0, buffer.slice(3, 3).size());
    EXPECT_EQ(0, buffer.slice(512, 512).size());

    EXPECT_EQ(0, MemoryView().slice(0, 0).size());
    EXPECT_EQ(0, MemoryView().slice(312, 312).size());
    EXPECT_EQ(0, MutableMemoryView().slice(0, 0).size());
    EXPECT_EQ(0, MutableMemoryView().slice(10, 10).size());
}


TEST(TestMemoryView, testSlice) {
    byte src[64];

    byte i = 0;
    for (auto& a : src) {
        a = i++;
    }

    auto buffer = wrapMemory(src);
    auto slice = buffer.slice(32, buffer.size());
    EXPECT_EQ(32, slice.size());
    EXPECT_EQ(32, slice[0]);
    EXPECT_EQ(63, slice[31]);

    EXPECT_EQ(buffer, buffer.slice(0, buffer.size()));
    EXPECT_TRUE(buffer.slice(3, 3).empty());
    EXPECT_TRUE(buffer.slice(128, 256).empty());
    EXPECT_TRUE(buffer.slice(128, 2).empty());
    EXPECT_TRUE(buffer.slice(32, 2).empty());
}


TEST(TestMemoryView, testPlacementConstruct) {
    {
        byte src[3];
        auto buffer = wrapMemory(src);
        EXPECT_THROW(buffer.construct<SimpleType>(4, -2, 12), IndexOutOfRangeException);
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
