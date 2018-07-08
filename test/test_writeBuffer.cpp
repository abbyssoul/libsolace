/*
*  Copyright 2017 Ivan Ryabov
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
*******************************************************************************/
#include <solace/writeBuffer.hpp>  // Class being tested

#include <cppunit/extensions/HelperMacros.h>



using namespace Solace;


class TestWriteBuffer :
        public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestWriteBuffer);
        CPPUNIT_TEST(testConstruction);
        CPPUNIT_TEST(testPositioning);
        CPPUNIT_TEST(testWrite);

        CPPUNIT_TEST(writeBigEndian);
        CPPUNIT_TEST(writeLittleEndian);
    CPPUNIT_TEST_SUITE_END();

public:

    void testConstruction() {
    }

    void testPositioning() {
        byte mem[12];
        constexpr WriteBuffer::size_type testSize = sizeof(mem);
        auto buffer = WriteBuffer(wrapMemory(mem));

        CPPUNIT_ASSERT_EQUAL(testSize, buffer.capacity());
        CPPUNIT_ASSERT_EQUAL(testSize, buffer.limit());
        CPPUNIT_ASSERT_EQUAL(static_cast<WriteBuffer::size_type>(0), buffer.position());

        CPPUNIT_ASSERT(buffer.position(buffer.position() + 12).isOk());
        CPPUNIT_ASSERT(buffer.position(0).isOk());
        CPPUNIT_ASSERT(buffer.advance(12).isOk());
        CPPUNIT_ASSERT(buffer.position(0).isOk());

        for (WriteBuffer::size_type i = 0; i < testSize; ++i) {
            buffer.advance(1);
        }
        CPPUNIT_ASSERT_EQUAL(buffer.limit(), buffer.position());

        CPPUNIT_ASSERT(buffer.position(buffer.limit() + 1).isError());

        CPPUNIT_ASSERT(buffer.position(buffer.limit()).isOk());
        CPPUNIT_ASSERT(buffer.advance(1).isError());
    }

    void testWrite() {
        byte destMem[7];

        {  // Happy path
            byte bytes[] = {'a', 'b', 'c', 0, 'd', 'f', 'g'};

            WriteBuffer buffer(wrapMemory(destMem));
            CPPUNIT_ASSERT_NO_THROW(buffer.write(wrapMemory(bytes)));
            CPPUNIT_ASSERT_EQUAL(buffer.limit(), buffer.position());
        }

        {  // Error cases
            byte truckLoadOfData[] = {'a', 'b', 'c', 0, 'd', 'e', 'f', 'g'};
            auto viewBytes = wrapMemory(truckLoadOfData);

            WriteBuffer buffer(wrapMemory(destMem));
            // Attempt to write more bytes then fit into the dest buffer
            CPPUNIT_ASSERT(buffer.write(viewBytes).isError());

            // Attempt to write more bytes then availible in the source buffer
            CPPUNIT_ASSERT(buffer.write(viewBytes, 128).isError());
        }
    }


    void writeBigEndian() {
        byte bytes[8];

        {
            uint16 const value(1025);
            CPPUNIT_ASSERT(WriteBuffer(wrapMemory(bytes)).writeBE(value).isOk());
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x04), bytes[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x01), bytes[1]);
        }

        {
            uint32 const value(0x842da380);
            CPPUNIT_ASSERT(WriteBuffer(wrapMemory(bytes)).writeBE(value).isOk());
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x84), bytes[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x2d), bytes[1]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0xa3), bytes[2]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x80), bytes[3]);
        }

        {
            uint64 const value(0x842da380e3426dff);
            CPPUNIT_ASSERT(WriteBuffer(wrapMemory(bytes)).writeBE(value).isOk());
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
        {
            uint16 const value(1025);
            CPPUNIT_ASSERT(WriteBuffer(wrapMemory(bytes)).writeLE(value).isOk());
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x01), bytes[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x04), bytes[1]);
        }

        {
            uint32 const value(1025);
            CPPUNIT_ASSERT(WriteBuffer(wrapMemory(bytes)).writeLE(value).isOk());
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x01), bytes[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x04), bytes[1]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x00), bytes[2]);
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0x00), bytes[3]);
        }

        {
            uint64 const value(0x842da380e3426dff);
            CPPUNIT_ASSERT(WriteBuffer(wrapMemory(bytes)).writeLE(value).isOk());
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

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestWriteBuffer);
