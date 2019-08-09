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
 ********************************************************************************/
#include <solace/byteReader.hpp>  // Class being tested

#include <gtest/gtest.h>

using namespace Solace;


using size_type = ByteReader::size_type;


TEST(TestReadBuffer, defaultConstructedBufferIsEmpty) {
    ByteReader buffer;

    EXPECT_EQ(0, buffer.capacity());
    EXPECT_EQ(0, buffer.limit());
    EXPECT_EQ(0, buffer.position());
}


TEST(TestReadBuffer, constructFromImmutableMemory) {
    const byte bytes[] = {'a', 'b', 'c', 0, 'd', 'f', 'g'};
    constexpr size_type testSize = sizeof (bytes);

    ByteReader buffer(wrapMemory(bytes));
    EXPECT_EQ(testSize, buffer.capacity());
    EXPECT_EQ(testSize, buffer.limit());
    EXPECT_EQ(0, buffer.position());
}

TEST(TestReadBuffer, constructFromMutableMemory) {
    byte bytes[] = {'a', 'b', 'c', 0, 'd', 'f', 'g'};
    constexpr size_type testSize = sizeof (bytes);

    MutableMemoryView memView = wrapMemory(bytes);

    ByteReader buffer(std::move(memView));
    EXPECT_EQ(testSize, buffer.capacity());
    EXPECT_EQ(testSize, buffer.limit());
    EXPECT_EQ(0, buffer.position());
}


TEST(TestReadBuffer, testPositioning) {
    const byte bytes[] = {'a', 'b', 'c', 0, 'd', 'f', 'g'};

    ByteReader buffer(wrapMemory(bytes));

    // We can re-position safely
    EXPECT_TRUE(buffer.position(4).isOk());
    EXPECT_EQ(size_type(4), buffer.position());
    EXPECT_TRUE(buffer.position(0).isOk());
    EXPECT_EQ(0, buffer.position());
    // Can't go beyond the limit
    EXPECT_TRUE(buffer.position(buffer.limit() + 3).isError());

    EXPECT_EQ(0, buffer.position());
    EXPECT_TRUE(buffer.advance(5).isOk());
    EXPECT_EQ(size_type(5), buffer.position());

    // It's ok to got to the end of the world... err the end of the buffer.
	EXPECT_TRUE(buffer.position(buffer.limit()).isOk());
    // ..but not beyond
    EXPECT_TRUE(buffer.advance(1).isError());
}

TEST(TestReadBuffer, testGetByte) {
    const byte srcBytes[] = {'a', 'b', 'c', 0, 'd', 'f', 'g'};
    ByteReader buffer(wrapMemory(srcBytes));

    size_type i = 0;
    for (auto b : srcBytes) {
        auto const value = buffer.get(i++);
        EXPECT_TRUE(value.isOk());
        EXPECT_EQ(b, value.unwrap());
    }

    for (auto b : srcBytes) {
        auto const value = buffer.get();
        EXPECT_TRUE(value.isOk());
        EXPECT_EQ(b, value.unwrap());
    }

    EXPECT_EQ(buffer.limit(), buffer.position());
    EXPECT_TRUE(buffer.get().isError());
}


TEST(TestReadBuffer, testByteRead) {
    const byte bytes[] = {'a', 'b', 'c', 0, 'd', 'f', 'g'};
    constexpr size_type kTestSize = sizeof(bytes);
    byte readBytes[kTestSize];

    ByteReader buffer(wrapMemory(bytes));

    for (size_type i = 0; i < kTestSize; ++i) {
        buffer.read(&(readBytes[i]));
    }

    // Check that we read all
    EXPECT_EQ(buffer.limit(), buffer.position());

    for (size_type i = 0; i < sizeof(readBytes); ++i) {
        EXPECT_EQ(bytes[i], readBytes[i]);
    }
}

TEST(TestReadBuffer, testReadIntoBuffer) {
    byte const srcBytes[] = {'a', 'b', 'c', 0, 'd', 'f', 'g'};
    constexpr size_type testSize = sizeof(srcBytes);

    byte destBuffer[128];
    constexpr size_type readBufferChunk = 3;

    ByteReader buffer(wrapMemory(srcBytes));
    MutableMemoryView destView = wrapMemory(destBuffer);

    EXPECT_TRUE(buffer.read(destView, readBufferChunk).isOk());
    for (size_type i = 0; i < readBufferChunk; ++i) {
        EXPECT_EQ(srcBytes[i], destBuffer[i]);
    }

    // Make sure read updated the position
    EXPECT_EQ(readBufferChunk, buffer.position());

    // Attempting to read more data then there is the buffer
    EXPECT_TRUE(buffer.read(destView, testSize).isError());
}

TEST(TestReadBuffer, testReadFromOffset) {
    byte const srcBytes[] = {'a', 'b', 'c', 0, 'd', 'f', 'g'};
    constexpr size_type testSize = sizeof(srcBytes);

    byte readBuffer[128];
    constexpr size_type readBufferChunk = 3;

    ByteReader buffer(wrapMemory(srcBytes));
    MutableMemoryView destView = wrapMemory(readBuffer);

    // Read data from an offset
    EXPECT_TRUE(buffer.read(4, destView, readBufferChunk).isOk());
    for (size_type i = 0; i < readBufferChunk; ++i) {
        EXPECT_EQ(srcBytes[4 + i], readBuffer[i]);
    }

    // Make sure we have not moved position
    EXPECT_EQ(0, buffer.position());

    // We can't read more data than there is in the buffer
    EXPECT_TRUE(buffer.read(testSize - 3, destView, 12).isError());

    // We can't read from the offset beyond the buffer size either
    EXPECT_TRUE(buffer.read(testSize + 3, destView, 2).isError());
}


TEST(TestReadBuffer, readBigEndian) {
    byte const bytes[] =   {0x84, 0x2d, 0xa3, 0x80,
                            0xe3, 0x42, 0x6d, 0xff};

    uint8 const expected8(0x84);
    uint16 const expected16(0x842d);
    uint32 const expected32(0x842da380);
    uint64 const expected64(0x842da380e3426dff);

    {
        uint8 result;
        EXPECT_TRUE(ByteReader(wrapMemory(bytes)).readBE(result).isOk());
        EXPECT_EQ(expected8, result);
    }
    {
        uint16 result;
        EXPECT_TRUE(ByteReader(wrapMemory(bytes)).readBE(result).isOk());
        EXPECT_EQ(expected16, result);
    }

    {
        uint32 result;
        EXPECT_TRUE(ByteReader(wrapMemory(bytes)).readBE(result).isOk());
        EXPECT_EQ(expected32, result);
    }

    {
        uint64 result;
        EXPECT_TRUE(ByteReader(wrapMemory(bytes)).readBE(result).isOk());
        EXPECT_EQ(expected64, result);
    }
}


TEST(TestReadBuffer, readLittleEndian) {
    byte const bytes[] =   {0x01, 0x04, 0x00, 0x00,
                            0xe3, 0x42, 0x6d, 0xff};

    uint8 const expected8(0x01);
    uint16 const expected16(1025);
    uint32 const expected32(1025);
    uint64 const expected64(0xff6d42e300000401);

    {
        uint8 result;
        EXPECT_TRUE(ByteReader(wrapMemory(bytes)).readLE(result).isOk());
        EXPECT_EQ(expected8, result);
    }
    {
        uint16 result;
        EXPECT_TRUE(ByteReader(wrapMemory(bytes)).readLE(result).isOk());
        EXPECT_EQ(expected16, result);
    }

    {
        uint32 result;
        EXPECT_TRUE(ByteReader(wrapMemory(bytes)).readLE(result).isOk());
        EXPECT_EQ(expected32, result);
    }

    {
        uint64 result;
        EXPECT_TRUE(ByteReader(wrapMemory(bytes)).readLE(result).isOk());
        EXPECT_EQ(expected64, result);
    }
}
