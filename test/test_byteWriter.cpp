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
#include <solace/byteWriter.hpp>  // Class being tested

#include <gtest/gtest.h>

using namespace Solace;


TEST(TestByteWriter, testConstruction) {
}

TEST(TestByteWriter, testPositioning) {
    byte mem[12];
    constexpr ByteWriter::size_type testSize = sizeof(mem);
    auto buffer = ByteWriter(wrapMemory(mem));

    EXPECT_EQ(testSize, buffer.capacity());
    EXPECT_EQ(testSize, buffer.limit());
    EXPECT_EQ(0, buffer.position());

    EXPECT_TRUE(buffer.position(buffer.position() + 12).isOk());
    EXPECT_TRUE(buffer.position(0).isOk());
    EXPECT_TRUE(buffer.advance(12).isOk());
    EXPECT_TRUE(buffer.position(0).isOk());

    for (ByteWriter::size_type i = 0; i < testSize; ++i) {
        buffer.advance(1);
    }
    EXPECT_EQ(buffer.limit(), buffer.position());

    EXPECT_TRUE(buffer.position(buffer.limit() + 1).isError());

    EXPECT_TRUE(buffer.position(buffer.limit()).isOk());
    EXPECT_TRUE(buffer.advance(1).isError());
}

TEST(TestByteWriter, testWrite) {
    byte destMem[7];

    {  // Happy path
        byte bytes[] = {'a', 'b', 'c', 0, 'd', 'f', 'g'};

        auto writer = ByteWriter{wrapMemory(destMem)};
        EXPECT_NO_THROW(writer.write(wrapMemory(bytes)));
        EXPECT_EQ(writer.limit(), writer.position());
    }

    {  // Error cases
        byte const truckLoadOfData[] = {'a', 'b', 'c', 0, 'd', 'e', 'f', 'g'};
        auto viewBytes = wrapMemory(truckLoadOfData);

        auto writer = ByteWriter{wrapMemory(destMem)};
        // Attempt to write more bytes then fit into the dest buffer
        EXPECT_TRUE(writer.write(viewBytes).isError());

        // Attempt to write more bytes then availible in the source buffer
        EXPECT_TRUE(writer.write(viewBytes, 128).isError());
    }
}


TEST(TestByteWriter, writeBigEndian) {
    byte bytes[8];

    {
        uint16 const value(1025);
        EXPECT_TRUE(ByteWriter(wrapMemory(bytes)).writeBE(value).isOk());
        EXPECT_EQ(static_cast<byte>(0x04), bytes[0]);
        EXPECT_EQ(static_cast<byte>(0x01), bytes[1]);
    }

    {
        uint32 const value(0x842da380);
        EXPECT_TRUE(ByteWriter(wrapMemory(bytes)).writeBE(value).isOk());
        EXPECT_EQ(static_cast<byte>(0x84), bytes[0]);
        EXPECT_EQ(static_cast<byte>(0x2d), bytes[1]);
        EXPECT_EQ(static_cast<byte>(0xa3), bytes[2]);
        EXPECT_EQ(static_cast<byte>(0x80), bytes[3]);
    }

    {
        uint64 const value(0x842da380e3426dff);
        EXPECT_TRUE(ByteWriter(wrapMemory(bytes)).writeBE(value).isOk());
        EXPECT_EQ(static_cast<byte>(0x84), bytes[0]);
        EXPECT_EQ(static_cast<byte>(0x2d), bytes[1]);
        EXPECT_EQ(static_cast<byte>(0xa3), bytes[2]);
        EXPECT_EQ(static_cast<byte>(0x80), bytes[3]);
        EXPECT_EQ(static_cast<byte>(0xe3), bytes[4]);
        EXPECT_EQ(static_cast<byte>(0x42), bytes[5]);
        EXPECT_EQ(static_cast<byte>(0x6d), bytes[6]);
        EXPECT_EQ(static_cast<byte>(0xff), bytes[7]);
    }
}


TEST(TestByteWriter, writeLittleEndian) {
    byte bytes[8];
    {
        uint16 const value(1025);
        EXPECT_TRUE(ByteWriter(wrapMemory(bytes)).writeLE(value).isOk());
        EXPECT_EQ(static_cast<byte>(0x01), bytes[0]);
        EXPECT_EQ(static_cast<byte>(0x04), bytes[1]);
    }

    {
        uint32 const value(1025);
        EXPECT_TRUE(ByteWriter(wrapMemory(bytes)).writeLE(value).isOk());
        EXPECT_EQ(static_cast<byte>(0x01), bytes[0]);
        EXPECT_EQ(static_cast<byte>(0x04), bytes[1]);
        EXPECT_EQ(static_cast<byte>(0x00), bytes[2]);
        EXPECT_EQ(static_cast<byte>(0x00), bytes[3]);
    }

    {
        uint64 const value(0x842da380e3426dff);
        EXPECT_TRUE(ByteWriter(wrapMemory(bytes)).writeLE(value).isOk());
        EXPECT_EQ(static_cast<byte>(0xff), bytes[0]);
        EXPECT_EQ(static_cast<byte>(0x6d), bytes[1]);
        EXPECT_EQ(static_cast<byte>(0x42), bytes[2]);
        EXPECT_EQ(static_cast<byte>(0xe3), bytes[3]);
        EXPECT_EQ(static_cast<byte>(0x80), bytes[4]);
        EXPECT_EQ(static_cast<byte>(0xa3), bytes[5]);
        EXPECT_EQ(static_cast<byte>(0x2d), bytes[6]);
        EXPECT_EQ(static_cast<byte>(0x84), bytes[7]);
    }
}
