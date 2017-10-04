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
 * @file: test/test_writeBuffer.cpp
 * @author: soultaker
 *
 * Created on: 28 Apr 2016
*******************************************************************************/
#include <solace/byteBuffer.hpp>  // Class being tested
#include <solace/memoryManager.hpp>

#include <solace/exception.hpp>
#include <cppunit/extensions/HelperMacros.h>



using namespace Solace;


class TestByteBuffer: public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestByteBuffer);
        CPPUNIT_TEST(testConstruction);
        CPPUNIT_TEST(testPositioning);
        CPPUNIT_TEST(testWrite);
        CPPUNIT_TEST(testRead);
        CPPUNIT_TEST(testGetByte);

        CPPUNIT_TEST(endianConsisten);
        CPPUNIT_TEST(readBigEndian);
        CPPUNIT_TEST(readLittleEndian);
        CPPUNIT_TEST(writeBigEndian);
        CPPUNIT_TEST(writeLittleEndian);
    CPPUNIT_TEST_SUITE_END();

protected:
    MemoryManager _memoryManager;

public:

    TestByteBuffer(): _memoryManager(4096)
    {
    }

	void testConstruction() {
	}

    void testPositioning() {
        constexpr ByteBuffer::size_type testSize = 12;
        ByteBuffer buffer(_memoryManager.create(testSize));

        CPPUNIT_ASSERT_EQUAL(testSize, buffer.capacity());
        CPPUNIT_ASSERT_EQUAL(testSize, buffer.limit());
        CPPUNIT_ASSERT_EQUAL(static_cast<ByteBuffer::size_type>(0), buffer.position());

        CPPUNIT_ASSERT_NO_THROW(buffer.position(buffer.position() + 12));
        CPPUNIT_ASSERT_NO_THROW(buffer.position(0));
        CPPUNIT_ASSERT_NO_THROW(buffer.advance(12));
        CPPUNIT_ASSERT_NO_THROW(buffer.position(0));

        for (ByteBuffer::size_type i = 0; i < testSize; ++i) {
            buffer << 'a';
        }

        CPPUNIT_ASSERT_EQUAL(buffer.limit(), buffer.position());

        CPPUNIT_ASSERT_THROW(buffer.position(buffer.limit() + 1), IllegalArgumentException);

        CPPUNIT_ASSERT_NO_THROW(buffer.position(buffer.limit()));
        CPPUNIT_ASSERT_THROW(buffer.advance(1), IllegalArgumentException);
    }

    void testWrite() {
        constexpr ByteBuffer::size_type testSize = 7;
        ByteBuffer buffer(_memoryManager.create(testSize));

        {  // Happy path
            byte bytes[] = {'a', 'b', 'c', 0, 'd', 'f', 'g'};

            CPPUNIT_ASSERT_NO_THROW(buffer.write(bytes, sizeof(bytes)));
            CPPUNIT_ASSERT_EQUAL(buffer.limit(), buffer.position());

            buffer.rewind();
        }

        {  // Error cases
            byte tracLoadOfData[] = {'a', 'b', 'c', 0, 'd', 'e', 'f', 'g'};
            auto viewBytes = wrapMemory(tracLoadOfData);

            // Attempt to write more bytes then fit into the dest buffer
            CPPUNIT_ASSERT_THROW(buffer.write(viewBytes), OverflowException);

            // Attempt to write more bytes then availible in the source buffer
            CPPUNIT_ASSERT_THROW(buffer.write(viewBytes, 128), OverflowException);
        }
    }


    void testGetByte() {
        byte bytes[] = {'a', 'b', 'c', 0, 'd', 'f', 'g'};

        constexpr ByteBuffer::size_type testSize = sizeof(bytes);
        ByteBuffer buffer(_memoryManager.create(testSize));

        CPPUNIT_ASSERT_NO_THROW(buffer.write(bytes, sizeof(bytes)));

        CPPUNIT_ASSERT_THROW(buffer.get(), OverflowException);
        CPPUNIT_ASSERT_THROW(buffer.get(testSize), IllegalArgumentException);

        for (ByteBuffer::size_type i = 0; i < testSize; ++i) {
            CPPUNIT_ASSERT_EQUAL(bytes[i], buffer.get(i));
        }

        buffer.flip();
        for (ByteBuffer::size_type i = 0; i < testSize; ++i) {
            CPPUNIT_ASSERT_EQUAL(bytes[i], buffer.get());
        }
        CPPUNIT_ASSERT_THROW(buffer.get(), OverflowException);
    }


    void testRead() {
        byte bytes[] = {'a', 'b', 'c', 0, 'd', 'f', 'g'};
        byte readBytes[3];

        constexpr ByteBuffer::size_type testSize = sizeof(bytes);
        ByteBuffer buffer(_memoryManager.create(testSize));

        CPPUNIT_ASSERT_NO_THROW(buffer.write(bytes, sizeof(bytes)));

        CPPUNIT_ASSERT_THROW(buffer.read(readBytes, sizeof(readBytes)), OverflowException);
        CPPUNIT_ASSERT_NO_THROW(buffer.read(3, readBytes, sizeof(readBytes)));

        for (ByteBuffer::size_type i = 0; i < sizeof(readBytes); ++i) {
            CPPUNIT_ASSERT_EQUAL(bytes[i + 3], readBytes[i]);
        }
    }


    void readBigEndian() {
        byte bytes[] = {0x84, 0x2d, 0xa3, 0x80,
                        0xe3, 0x42, 0x6d, 0xff};

        uint8 expected8(0x84);
        uint16 expected16(0x842d);
        uint32 expected32(0x842da380);
        uint64 expected64(0x842da380e3426dff);

        ByteBuffer buffer(wrapMemory(bytes));
        {
            uint8 result;
            buffer.readBE(result).rewind();
            CPPUNIT_ASSERT_EQUAL(expected8, result);
        }
        {
            uint16 result;
            buffer.readBE(result).rewind();
            CPPUNIT_ASSERT_EQUAL(expected16, result);
        }

        {
            uint32 result;
            buffer.readBE(result).rewind();
            CPPUNIT_ASSERT_EQUAL(expected32, result);
        }

        {
            uint64 result;
            buffer.readBE(result).rewind();
            CPPUNIT_ASSERT_EQUAL(expected64, result);
        }
    }


    void readLittleEndian() {
        byte bytes[] = {0x01, 0x04, 0x00, 0x00,
                        0xe3, 0x42, 0x6d, 0xff};

        uint8 expected8(0x01);
        uint16 expected16(1025);
        uint32 expected32(1025);
        uint64 expected64(0xff6d42e300000401);

        ByteBuffer buffer(wrapMemory(bytes));
        {
            uint8 result;
            buffer.readLE(result).rewind();
            CPPUNIT_ASSERT_EQUAL(expected8, result);
        }
        {
            uint16 result;
            buffer.readLE(result).rewind();
            CPPUNIT_ASSERT_EQUAL(expected16, result);
        }

        {
            uint32 result;
            buffer.readLE(result).rewind();
            CPPUNIT_ASSERT_EQUAL(expected32, result);
        }

        {
            uint64 result;
            buffer.readLE(result).rewind();
            CPPUNIT_ASSERT_EQUAL(expected64, result);
        }
    }



    void writeBigEndian() {
        byte bytes[8];
        ByteBuffer buffer(wrapMemory(bytes));

        {
            const uint16 value(1025);
            buffer.writeBE(value).rewind();
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x04), bytes[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x01), bytes[1]);
        }

        {
            const uint32 value(0x842da380);
            buffer.writeBE(value).rewind();
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x84), bytes[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x2d), bytes[1]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0xa3), bytes[2]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x80), bytes[3]);
        }

        {
            const uint64 value(0x842da380e3426dff);
            buffer.writeBE(value).rewind();
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x84), bytes[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x2d), bytes[1]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0xa3), bytes[2]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x80), bytes[3]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0xe3), bytes[4]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x42), bytes[5]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x6d), bytes[6]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0xff), bytes[7]);
        }
    }


    void writeLittleEndian() {
        byte bytes[8];
        ByteBuffer buffer(wrapMemory(bytes));

        {
            const uint16 value(1025);
            buffer.writeLE(value).rewind();
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x01), bytes[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x04), bytes[1]);
        }

        {
            const uint32 value(1025);
            buffer.writeLE(value).rewind();
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x01), bytes[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x04), bytes[1]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x00), bytes[2]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x00), bytes[3]);
        }

        {
            const uint64 value(0x842da380e3426dff);
            buffer.writeLE(value).rewind();
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0xff), bytes[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x6d), bytes[1]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x42), bytes[2]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0xe3), bytes[3]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x80), bytes[4]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0xa3), bytes[5]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x2d), bytes[6]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x84), bytes[7]);
        }
    }

    void endianConsisten() {
        byte bytes[8];
        ByteBuffer buffer(wrapMemory(bytes));

        {
            const uint16 value(0x842d);
            buffer.writeLE(value).rewind();

            uint16 res;
            buffer.readLE(res).rewind();
            CPPUNIT_ASSERT_EQUAL(value, res);
        }

        {
            const uint16 value(0x842d);
            buffer.writeBE(value).rewind();

            uint16 res;
            buffer.readBE(res).rewind();
            CPPUNIT_ASSERT_EQUAL(value, res);
        }

    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestByteBuffer);
