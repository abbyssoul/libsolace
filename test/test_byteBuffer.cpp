#include <solace/byteBuffer.hpp>  // Class being tested
#include <solace/exception.hpp>

#include <gtest/gtest.h>

using namespace Solace;

class TestByteBuffer : public ::testing::Test  {

public:

    void setUp() {
    }

    void tearDown() {
    }
};

TEST_F(TestByteBuffer, testConstruction) {
    //
}

TEST_F(TestByteBuffer, testPositioning) {
    byte mem[12];
    constexpr ByteBuffer::size_type testSize = sizeof(mem);
    ByteBuffer buffer(wrapMemory(mem));

    EXPECT_EQ(testSize, buffer.capacity());
    EXPECT_EQ(testSize, buffer.limit());
    EXPECT_EQ(static_cast<ByteBuffer::size_type>(0), buffer.position());

    EXPECT_TRUE(buffer.position(buffer.position() + 12).isOk());
    EXPECT_TRUE(buffer.position(0).isOk());
    EXPECT_TRUE(buffer.advance(12).isOk());
    EXPECT_TRUE(buffer.position(0).isOk());

    for (ByteBuffer::size_type i = 0; i < testSize; ++i) {
        buffer << 'a';
    }

    EXPECT_EQ(buffer.limit(), buffer.position());

    EXPECT_TRUE(buffer.position(buffer.limit() + 1).isError());

    EXPECT_TRUE(buffer.position(buffer.limit()).isOk());
    EXPECT_TRUE(buffer.advance(1).isError());
}

TEST_F(TestByteBuffer, testWrite) {
    byte destMem[7];

    {  // Happy path
        byte bytes[] = {'a', 'b', 'c', 0, 'd', 'f', 'g'};

        ByteBuffer buffer(wrapMemory(destMem));
        EXPECT_NO_THROW(buffer.write(wrapMemory(bytes)));
        EXPECT_EQ(buffer.limit(), buffer.position());
    }

    {  // Error cases
        byte truckLoadOfData[] = {'a', 'b', 'c', 0, 'd', 'e', 'f', 'g'};
        auto viewBytes = wrapMemory(truckLoadOfData);

        ByteBuffer buffer(wrapMemory(destMem));
        // Attempt to write more bytes then fit into the dest buffer
        EXPECT_THROW(buffer.write(viewBytes), OverflowException);

        // Attempt to write more bytes then availible in the source buffer
        EXPECT_THROW(buffer.write(viewBytes, 128), OverflowException);
    }
}

TEST_F(TestByteBuffer, readBigEndian) {
    byte bytes[] = {0x84, 0x2d, 0xa3, 0x80,
                    0xe3, 0x42, 0x6d, 0xff};

    uint8 expected8(0x84);
    uint16 expected16(0x842d);
    uint32 expected32(0x842da380);
    uint64 expected64(0x842da380e3426dff);

    {
        uint8 result;
        EXPECT_TRUE(ByteBuffer(wrapMemory(bytes)).readBE(result).isOk());
        EXPECT_EQ(expected8, result);
    }
    {
        uint16 result;
        EXPECT_TRUE(ByteBuffer(wrapMemory(bytes)).readBE(result).isOk());
        EXPECT_EQ(expected16, result);
    }

    {
        uint32 result;
        EXPECT_TRUE(ByteBuffer(wrapMemory(bytes)).readBE(result).isOk());
        EXPECT_EQ(expected32, result);
    }

    {
        uint64 result;
        EXPECT_TRUE(ByteBuffer(wrapMemory(bytes)).readBE(result).isOk());
        EXPECT_EQ(expected64, result);
    }
}


TEST_F(TestByteBuffer, readLittleEndian) {
    byte bytes[] = {0x01, 0x04, 0x00, 0x00,
                    0xe3, 0x42, 0x6d, 0xff};

    uint8 expected8(0x01);
    uint16 expected16(1025);
    uint32 expected32(1025);
    uint64 expected64(0xff6d42e300000401);

    {
        uint8 result;
        EXPECT_TRUE(ByteBuffer(wrapMemory(bytes)).readLE(result).isOk());
        EXPECT_EQ(expected8, result);
    }
    {
        uint16 result;
        EXPECT_TRUE(ByteBuffer(wrapMemory(bytes)).readLE(result).isOk());
        EXPECT_EQ(expected16, result);
    }

    {
        uint32 result;
        EXPECT_TRUE(ByteBuffer(wrapMemory(bytes)).readLE(result).isOk());
        EXPECT_EQ(expected32, result);
    }

    {
        uint64 result;
        EXPECT_TRUE(ByteBuffer(wrapMemory(bytes)).readLE(result).isOk());
        EXPECT_EQ(expected64, result);
    }
}



TEST_F(TestByteBuffer, writeBigEndian) {
    byte bytes[8];
    ByteBuffer buffer(wrapMemory(bytes));

    {
        const uint16 value(1025);
        buffer.writeBE(value).rewind();
        EXPECT_EQ(static_cast<byte>(0x04), bytes[0]);
        EXPECT_EQ(static_cast<byte>(0x01), bytes[1]);
    }

    {
        const uint32 value(0x842da380);
        buffer.writeBE(value).rewind();
        EXPECT_EQ(static_cast<byte>(0x84), bytes[0]);
        EXPECT_EQ(static_cast<byte>(0x2d), bytes[1]);
        EXPECT_EQ(static_cast<byte>(0xa3), bytes[2]);
        EXPECT_EQ(static_cast<byte>(0x80), bytes[3]);
    }

    {
        const uint64 value(0x842da380e3426dff);
        buffer.writeBE(value).rewind();
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


TEST_F(TestByteBuffer, writeLittleEndian) {
    byte bytes[8];
    ByteBuffer buffer(wrapMemory(bytes));

    {
        const uint16 value(1025);
        buffer.writeLE(value).rewind();
        EXPECT_EQ(static_cast<byte>(0x01), bytes[0]);
        EXPECT_EQ(static_cast<byte>(0x04), bytes[1]);
    }

    {
        const uint32 value(1025);
        buffer.writeLE(value).rewind();
        EXPECT_EQ(static_cast<byte>(0x01), bytes[0]);
        EXPECT_EQ(static_cast<byte>(0x04), bytes[1]);
        EXPECT_EQ(static_cast<byte>(0x00), bytes[2]);
        EXPECT_EQ(static_cast<byte>(0x00), bytes[3]);
    }

    {
        const uint64 value(0x842da380e3426dff);
        buffer.writeLE(value).rewind();
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

TEST_F(TestByteBuffer, endianConsisten) {
    byte bytes[8];

    {
        const uint16 value(0x842d);
        ByteBuffer(wrapMemory(bytes)).writeLE(value);

        uint16 res;
        EXPECT_TRUE(ByteBuffer(wrapMemory(bytes)).readLE(res).isOk());
        EXPECT_EQ(value, res);
    }

    {
        const uint16 value(0x842d);
        ByteBuffer(wrapMemory(bytes)).writeBE(value);

        uint16 res;
        EXPECT_TRUE(ByteBuffer(wrapMemory(bytes)).readBE(res).isOk());
        EXPECT_EQ(value, res);
    }

}