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
 * @file: test/test_readBuffer.cpp
 * @author: soultaker
 ********************************************************************************/
#include <solace/readBuffer.hpp>  // Class being tested

#include <cppunit/extensions/HelperMacros.h>



using namespace Solace;


class TestReadBuffer: public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestReadBuffer);
        CPPUNIT_TEST(defaultConstructedBufferIsEmpty);
        CPPUNIT_TEST(constructFromImmutableMemory);
        CPPUNIT_TEST(constructFromMutableMemory);

        CPPUNIT_TEST(testPositioning);
        CPPUNIT_TEST(testGetByte);
        CPPUNIT_TEST(testByteRead);

        CPPUNIT_TEST(testReadIntoByteBuffer);
        CPPUNIT_TEST(testReadFromOffset);

        CPPUNIT_TEST(readBigEndian);
        CPPUNIT_TEST(readLittleEndian);
    CPPUNIT_TEST_SUITE_END();

protected:
    using size_type = ReadBuffer::size_type;
    static const size_type ZERO;

public:

    void defaultConstructedBufferIsEmpty() {
        ReadBuffer buffer;

        CPPUNIT_ASSERT_EQUAL(ZERO, buffer.capacity());
        CPPUNIT_ASSERT_EQUAL(ZERO, buffer.limit());
        CPPUNIT_ASSERT_EQUAL(ZERO, buffer.position());
    }


    void constructFromImmutableMemory() {
        const byte bytes[] = {'a', 'b', 'c', 0, 'd', 'f', 'g'};
        constexpr size_type testSize = sizeof (bytes);

        ReadBuffer buffer(wrapMemory(bytes));
        CPPUNIT_ASSERT_EQUAL(testSize, buffer.capacity());
        CPPUNIT_ASSERT_EQUAL(testSize, buffer.limit());
        CPPUNIT_ASSERT_EQUAL(ZERO, buffer.position());
    }

    void constructFromMutableMemory() {
        byte bytes[] = {'a', 'b', 'c', 0, 'd', 'f', 'g'};
        constexpr size_type testSize = sizeof (bytes);

        MemoryView memView = wrapMemory(bytes);

        ReadBuffer buffer(std::move(memView));
        CPPUNIT_ASSERT_EQUAL(testSize, buffer.capacity());
        CPPUNIT_ASSERT_EQUAL(testSize, buffer.limit());
        CPPUNIT_ASSERT_EQUAL(ZERO, buffer.position());
    }


    void testPositioning() {
        const byte bytes[] = {'a', 'b', 'c', 0, 'd', 'f', 'g'};

        ReadBuffer buffer(wrapMemory(bytes));

        // We can re-position safely
        CPPUNIT_ASSERT(buffer.position(4).isOk());
        CPPUNIT_ASSERT_EQUAL(size_type(4), buffer.position());
        CPPUNIT_ASSERT(buffer.position(0).isOk());
        CPPUNIT_ASSERT_EQUAL(ZERO, buffer.position());
        // Can't go beyond the limit
        CPPUNIT_ASSERT(buffer.position(buffer.limit() + 3).isError());

        CPPUNIT_ASSERT_EQUAL(ZERO, buffer.position());
        CPPUNIT_ASSERT(buffer.advance(5).isOk());
        CPPUNIT_ASSERT_EQUAL(size_type(5), buffer.position());

        // It's ok to got to the end of the world... err the end of the buffer.
        CPPUNIT_ASSERT_NO_THROW(buffer.position(buffer.limit()));
        // ..but not beyond
        CPPUNIT_ASSERT(buffer.advance(1).isError());
    }

    void testGetByte() {
        const byte srcBytes[] = {'a', 'b', 'c', 0, 'd', 'f', 'g'};
        ReadBuffer buffer(wrapMemory(srcBytes));

        size_type i = 0;
        for (auto b : srcBytes) {
            const auto value = buffer.get(i++);
            CPPUNIT_ASSERT(value.isOk());
            CPPUNIT_ASSERT_EQUAL(b, value.unwrap());
        }

        for (auto b : srcBytes) {
            const auto value = buffer.get();
            CPPUNIT_ASSERT(value.isOk());
            CPPUNIT_ASSERT_EQUAL(b, value.unwrap());
        }

        CPPUNIT_ASSERT_EQUAL(buffer.limit(), buffer.position());
        CPPUNIT_ASSERT(buffer.get().isError());
    }


    void testByteRead() {
        const byte bytes[] = {'a', 'b', 'c', 0, 'd', 'f', 'g'};
        constexpr size_type kTestSize = sizeof(bytes);
        byte readBytes[kTestSize];

        ReadBuffer buffer(wrapMemory(bytes));

        for (size_type i = 0; i < kTestSize; ++i) {
            buffer.read(&(readBytes[i]));
        }

        // Check that we read all
        CPPUNIT_ASSERT_EQUAL(buffer.limit(), buffer.position());

        for (size_type i = 0; i < sizeof(readBytes); ++i) {
            CPPUNIT_ASSERT_EQUAL(bytes[i], readBytes[i]);
        }
    }

    void testReadIntoByteBuffer() {
        byte const srcBytes[] = {'a', 'b', 'c', 0, 'd', 'f', 'g'};
        constexpr size_type testSize = sizeof(srcBytes);

        byte destBuffer[128];
        constexpr size_type readBufferChunk = 3;

        ReadBuffer buffer(wrapMemory(srcBytes));
        MemoryView destView = wrapMemory(destBuffer);

        CPPUNIT_ASSERT(buffer.read(destView, readBufferChunk).isOk());
        for (size_type i = 0; i < readBufferChunk; ++i) {
            CPPUNIT_ASSERT_EQUAL(srcBytes[i], destBuffer[i]);
        }

        // Make sure read updated the position
        CPPUNIT_ASSERT_EQUAL(readBufferChunk, buffer.position());

        // Attempting to read more data then there is the buffer
        CPPUNIT_ASSERT(buffer.read(destView, testSize).isError());
    }

    void testReadFromOffset() {
        byte const srcBytes[] = {'a', 'b', 'c', 0, 'd', 'f', 'g'};
        constexpr size_type testSize = sizeof(srcBytes);

        byte readBuffer[128];
        constexpr size_type readBufferChunk = 3;

        ReadBuffer buffer(wrapMemory(srcBytes));
        MemoryView destView = wrapMemory(readBuffer);

        // Read data from an offset
        CPPUNIT_ASSERT(buffer.read(4, destView, readBufferChunk).isOk());
        for (size_type i = 0; i < readBufferChunk; ++i) {
            CPPUNIT_ASSERT_EQUAL(srcBytes[4 + i], readBuffer[i]);
        }

        // Make sure we have not moved position
        CPPUNIT_ASSERT_EQUAL(ZERO, buffer.position());

        // We can't read more data than there is in the buffer
        CPPUNIT_ASSERT(buffer.read(testSize - 3, destView, 12).isError());

        // We can't read from the offset beyond the buffer size either
        CPPUNIT_ASSERT(buffer.read(testSize + 3, destView, 2).isError());
    }


    void readBigEndian() {
        byte const bytes[] =   {0x84, 0x2d, 0xa3, 0x80,
                                0xe3, 0x42, 0x6d, 0xff};

        uint8 const expected8(0x84);
        uint16 const expected16(0x842d);
        uint32 const expected32(0x842da380);
        uint64 const expected64(0x842da380e3426dff);

        {
            uint8 result;
            CPPUNIT_ASSERT(ReadBuffer(wrapMemory(bytes)).readBE(result).isOk());
            CPPUNIT_ASSERT_EQUAL(expected8, result);
        }
        {
            uint16 result;
            CPPUNIT_ASSERT(ReadBuffer(wrapMemory(bytes)).readBE(result).isOk());
            CPPUNIT_ASSERT_EQUAL(expected16, result);
        }

        {
            uint32 result;
            CPPUNIT_ASSERT(ReadBuffer(wrapMemory(bytes)).readBE(result).isOk());
            CPPUNIT_ASSERT_EQUAL(expected32, result);
        }

        {
            uint64 result;
            CPPUNIT_ASSERT(ReadBuffer(wrapMemory(bytes)).readBE(result).isOk());
            CPPUNIT_ASSERT_EQUAL(expected64, result);
        }
    }


    void readLittleEndian() {
        byte const bytes[] =   {0x01, 0x04, 0x00, 0x00,
                                0xe3, 0x42, 0x6d, 0xff};

        uint8 const expected8(0x01);
        uint16 const expected16(1025);
        uint32 const expected32(1025);
        uint64 const expected64(0xff6d42e300000401);

        {
            uint8 result;
            CPPUNIT_ASSERT(ReadBuffer(wrapMemory(bytes)).readLE(result).isOk());
            CPPUNIT_ASSERT_EQUAL(expected8, result);
        }
        {
            uint16 result;
            CPPUNIT_ASSERT(ReadBuffer(wrapMemory(bytes)).readLE(result).isOk());
            CPPUNIT_ASSERT_EQUAL(expected16, result);
        }

        {
            uint32 result;
            CPPUNIT_ASSERT(ReadBuffer(wrapMemory(bytes)).readLE(result).isOk());
            CPPUNIT_ASSERT_EQUAL(expected32, result);
        }

        {
            uint64 result;
            CPPUNIT_ASSERT(ReadBuffer(wrapMemory(bytes)).readLE(result).isOk());
            CPPUNIT_ASSERT_EQUAL(expected64, result);
        }
    }

};

const TestReadBuffer::size_type TestReadBuffer::ZERO = 0;

CPPUNIT_TEST_SUITE_REGISTRATION(TestReadBuffer);
