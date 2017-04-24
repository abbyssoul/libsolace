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
        CPPUNIT_TEST(testRead);
        CPPUNIT_TEST(testWrite);
        CPPUNIT_TEST(testWrapping);
        CPPUNIT_TEST(testDataAs);
        CPPUNIT_TEST(testReadingPastTheSize);
        CPPUNIT_TEST(testSlice);
    CPPUNIT_TEST_SUITE_END();

protected:

    MemoryManager manager;

public:

    struct SomePodType {
        int x, y, z;

        SomePodType(int a, int b, int c) : x(a), y(b), z(c)
        {}

        friend bool operator == (const SomePodType& a, const SomePodType& b) {
            return ((a.x == b.x) && (a.y == b.y) && (a.z == b.z));
        }
    };

    struct LargePodType {
        SomePodType i1, i2, i3;
    };

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
        void* nullB = nullptr;
        CPPUNIT_ASSERT_NO_THROW(auto buffer = wrapMemory(nullB, 0));

        {   // Can't wrap nullptr with non-zero size
            CPPUNIT_ASSERT_THROW(auto b = wrapMemory(nullB, 321), IllegalArgumentException);
        }

        {   // Wrapping constructor
            byte example[] = {0, 1, 0, 3, 2, 1};  // size = 6
            auto test = wrapMemory(example);

            CPPUNIT_ASSERT(!test.empty());
            CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(6), test.size());

            for (MemoryView::size_type i = 0; i < test.size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(example[i], test.dataAddress()[i]);
            }
        }

        {   // Wrapping constructor with specific size
            byte example[] = {0, 1, 0, 3, 2, 1};  // size = 6
            auto test = wrapMemory(example, 4);

            CPPUNIT_ASSERT(!test.empty());
            CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(4), test.size());

            for (MemoryView::size_type i = 0; i < test.size(); ++i) {
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


        {   // Move-constructor
            byte example[] = {7, 10, 13, 16, 19, 22, 25};  // size = 7
            const MemoryView::size_type exampleSize = sizeof(example);
            auto b1 = wrapMemory(example, exampleSize);
            {
                MemoryView b2(b1.viewShallow());

                CPPUNIT_ASSERT_EQUAL(exampleSize, b1.size());
                CPPUNIT_ASSERT_EQUAL(exampleSize, b2.size());

                for (MemoryView::size_type i = 0; i < b1.size(); ++i) {
                    CPPUNIT_ASSERT_EQUAL(example[i], b1.dataAddress()[i]);
                    CPPUNIT_ASSERT_EQUAL(example[i], b2.dataAddress()[i]);
                }
            }

            // Test that after b2 has been destroyed - the memory is still valid.
            for (MemoryView::size_type i = 0; i < b1.size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(7 + 3*i, static_cast<MemoryView::size_type>(b1[i]));
            }
        }
    }

    void testRead() {
        MemoryView buffer = manager.create(128);
        MemoryView dest = manager.create(24);

        dest.fill(0);
        buffer.fill(64);

        {
            // Test simple read
            buffer.read(dest);
            for (auto b : dest) {
                CPPUNIT_ASSERT_EQUAL(static_cast<byte>(64), b);
            }

            // Test that source is independent of dest
            buffer.fill(76);
            for (auto b : dest) {
                CPPUNIT_ASSERT_EQUAL(static_cast<byte>(64), b);
            }
        }
        dest.fill(0);

        {  // Safety checks
            // Reading more then bytes in the buffer
            CPPUNIT_ASSERT_THROW(buffer.read(dest, 2*buffer.size()), Exception);

            // Reading more then fits into the dest
            CPPUNIT_ASSERT_THROW(dest.read(buffer), Exception);
            CPPUNIT_ASSERT_THROW(buffer.read(dest, 2*dest.size()), Exception);

            // Reading from invalid offset
            CPPUNIT_ASSERT_THROW(buffer.read(dest, 1, buffer.size() + 10), Exception);

            // Reading from invalid offset and too much
            CPPUNIT_ASSERT_THROW(buffer.read(dest, 2*dest.size(), buffer.size() + 10), Exception);
        }

        {  // Test reading from an offset
            buffer.fill(67, 0, 24);
            buffer.fill(76, 24, buffer.size());

            buffer.read(dest, 24);
            for (auto b : dest) {
                CPPUNIT_ASSERT_EQUAL(static_cast<byte>(67), b);
            }

            // Test that source is independent of dest
            buffer.read(dest, 24, 24);
            for (auto b : dest) {
                CPPUNIT_ASSERT_EQUAL(static_cast<byte>(76), b);
            }
        }
    }

    void testReadingPastTheSize() {
        byte src[15];

        auto&& buffer = wrapMemory(src);
        CPPUNIT_ASSERT_THROW(buffer[1042], IndexOutOfRangeException);
        CPPUNIT_ASSERT_THROW(buffer.dataAddress(15), IndexOutOfRangeException);
    }


    void testDataAs() {
        byte src[sizeof(SomePodType) + 5];

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

        SomePodType* uuid1 = buffer.dataAs<SomePodType>();
        CPPUNIT_ASSERT_EQUAL(SomePodType(1, 3, 2), *uuid1);

        tx = 7; ty = 44; tz = -32;
        buffer.write(wrapMemory(&tx, sizeof(tx)));
        buffer.write(wrapMemory(&ty, sizeof(tx)), sizeof(tx));
        buffer.write(wrapMemory(&tz, sizeof(tx)), 2*sizeof(ty));
        CPPUNIT_ASSERT_EQUAL(SomePodType(7, 44, -32), *uuid1);

        SomePodType* uuid2 = buffer.dataAs<SomePodType>(4);
        tx = -91; ty = 12; tz = 0;
        buffer.write(wrapMemory(&tx, sizeof(tx)), 4);
        buffer.write(wrapMemory(&ty, sizeof(tx)), 4 + sizeof(tx));
        buffer.write(wrapMemory(&tz, sizeof(tx)), 4 + 2*sizeof(ty));
        CPPUNIT_ASSERT_EQUAL(SomePodType(-91, 12, 0), *uuid2);

        CPPUNIT_ASSERT_THROW(buffer.dataAs<SomePodType>(6), IndexOutOfRangeException);
        CPPUNIT_ASSERT_THROW(buffer.dataAs<LargePodType>(), IndexOutOfRangeException);


        byte src2[sizeof(LargePodType)];
        auto buffer2 = wrapMemory(src2);
        CPPUNIT_ASSERT_NO_THROW(buffer2.dataAs<LargePodType>());
    }

    void testWrite() {
        MemoryView buffer = manager.create(128);
        MemoryView src = manager.create(24);

        src.fill(32);
        buffer.fill(0);

        {
            // Test simple read
            buffer.write(src);
            for (MemoryView::size_type i = 0; i < src.size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(static_cast<byte>(32), buffer[i]);
            }
            for (MemoryView::size_type i = src.size(); i < buffer.size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0), buffer[i]);
            }

            // Test that source is independent of dest
            src.fill(76);
            for (MemoryView::size_type i = 0; i < src.size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(static_cast<byte>(32), buffer[i]);
            }
            for (MemoryView::size_type i = src.size(); i < buffer.size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0), buffer[i]);
            }
        }

        {  // Safety checks

            // Writing more then fits into the buffer
            CPPUNIT_ASSERT_THROW(src.write(buffer), Exception);

            // Reading from invalid offset
            CPPUNIT_ASSERT_THROW(buffer.write(src, buffer.size() + 10), Exception);

            // Reading from invalid offset and too much
            CPPUNIT_ASSERT_THROW(buffer.write(src, buffer.size() - src.size() + 2), Exception);
        }

        {  // Test reading from an offset
            src.fill(41);
            buffer.fill(67, 0, 24);
            buffer.fill(76, 24, buffer.size());

            buffer.write(src, 24);
            for (MemoryView::size_type i = 0; i < 24; ++i) {
                CPPUNIT_ASSERT_EQUAL(static_cast<byte>(67), buffer[i]);
            }
            for (MemoryView::size_type i = 24; i < 24 + src.size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(static_cast<byte>(41), buffer[i]);
            }
            for (MemoryView::size_type i = 24 + src.size(); i < buffer.size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(static_cast<byte>(76), buffer[i]);
            }

            src.fill(71);
            buffer.write(src, 14);
            for (MemoryView::size_type i = 0; i < 14; ++i) {
                CPPUNIT_ASSERT_EQUAL(static_cast<byte>(67), buffer[i]);
            }
            for (MemoryView::size_type i = 14; i < 14 + src.size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(static_cast<byte>(71), buffer[i]);
            }
            for (MemoryView::size_type i = 24 + src.size(); i < buffer.size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(static_cast<byte>(76), buffer[i]);
            }
        }
    }

    void testSlice() {
        byte src[64];

        byte i = 0;
        for (auto& a : src) {
            a = i++;
        }

        auto&& buffer = wrapMemory(src);

        auto slice = buffer.slice(32, buffer.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<MemoryView::size_type>(32), slice.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<byte>(32), slice[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<byte>(63), slice[31]);

        CPPUNIT_ASSERT_THROW(buffer.slice(120, 152), IndexOutOfRangeException);
        CPPUNIT_ASSERT_THROW(buffer.slice(31, 18), IndexOutOfRangeException);
        CPPUNIT_ASSERT_THROW(buffer.slice(31, 939), IndexOutOfRangeException);
    }


};


std::ostream& operator<< (std::ostream& ostr, const TestMemoryView::SomePodType& a) {
    return ostr << "SomePodType(" <<
            "'x': " << a.x << ", " <<
            "'y': " << a.y << ", " <<
            "'z': " << a.z << ")";
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestMemoryView);
