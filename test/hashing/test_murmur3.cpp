#include <solace/hashing/murmur3.hpp>  // Class being tested

#include <gtest/gtest.h>

using namespace Solace;
using namespace Solace::hashing;

class TestHashingMurmur3: public ::testing::Test {

public:

    void setUp() {
	}

    void tearDown() {
	}
};

TEST_F(TestHashingMurmur3, testAlgorithmName) {
    EXPECT_EQ(String("MURMUR3-32"), Murmur3_32(0).getAlgorithm());
    EXPECT_EQ(String("MURMUR3-128"), Murmur3_128(0).getAlgorithm());
}

TEST_F(TestHashingMurmur3, hashEmptyMessage32) {
    char message[] = "";
    EXPECT_EQ(MessageDigest({0x0, 0x0, 0x0, 0x0}),
                            Murmur3_32(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
}

TEST_F(TestHashingMurmur3, hashEmptyMessage128) {
    char message[] = "";
    EXPECT_EQ(MessageDigest({0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}),
                            Murmur3_128(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
}

TEST_F(TestHashingMurmur3, hashSingleLetter32) {
    char message[] = "a";
    EXPECT_EQ(MessageDigest({0x3c, 0x25, 0x69, 0xb2}),
                            Murmur3_32(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
}

TEST_F(TestHashingMurmur3, hashSingleLetter128) {
    char message[] = "a";
    EXPECT_EQ(MessageDigest({0x89, 0x78, 0x59, 0xf6, 0x65, 0x55, 0x55, 0x85,
                                        0x5a, 0x89, 0x0e, 0x51, 0x48, 0x3a, 0xb5, 0xe6}),
                            Murmur3_128(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
}

TEST_F(TestHashingMurmur3, hashABC32) {
    char message[] = "abc";
    EXPECT_EQ(MessageDigest({0xB3, 0xDD, 0x93, 0xFA}),
                            Murmur3_32(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
}

TEST_F(TestHashingMurmur3, hashABC128) {
    char message[] = "abc";
    EXPECT_EQ(MessageDigest({0x67, 0x78, 0xad, 0x3f, 0x3f, 0x3f, 0x96, 0xb4,
                                        0x52, 0x2d, 0xca, 0x26, 0x41, 0x74, 0xa2, 0x3b}),
                            Murmur3_128(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
}

TEST_F(TestHashingMurmur3, hashMessageDigest32) {
    EXPECT_EQ(static_cast<Murmur3_32::size_type>(4), Murmur3_32(0).getDigestLength());

    char message[] = "message digest";
    EXPECT_EQ(MessageDigest({0x63, 0x8F, 0x41, 0x69}),
                            Murmur3_32(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
}

TEST_F(TestHashingMurmur3, hashMessageDigest128) {
    EXPECT_EQ(static_cast<Murmur3_128::size_type>(16), Murmur3_128(0).getDigestLength());

    char message[] = "message digest";
    EXPECT_EQ(MessageDigest({0xfc, 0x7d, 0x14, 0x76, 0x2d, 0x2c, 0x5d, 0x87,
                                        0x39, 0x6f, 0xbc, 0x12, 0x2a, 0xb0, 0x22, 0xf6}),
                            Murmur3_128(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
}

TEST_F(TestHashingMurmur3, hashAlphabet32) {
    char message[] = "abcdefghijklmnopqrstuvwxyz";
    EXPECT_EQ(MessageDigest({0xa3, 0x4e, 0x03, 0x6d}),
                            Murmur3_32(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
}

TEST_F(TestHashingMurmur3, hashAlphabet128) {
    char message[] = "abcdefghijklmnopqrstuvwxyz";
    EXPECT_EQ(MessageDigest({0xa9, 0x4a, 0x6f, 0x51, 0x7e, 0x9d, 0x9c, 0x74,
                                        0x29, 0xd5, 0xa7, 0xb6, 0x89, 0x9c, 0xad, 0xe9}),
                            Murmur3_128(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
}

TEST_F(TestHashingMurmur3, hashAlphanum32) {
    char message[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    EXPECT_EQ(MessageDigest({0xa2, 0x7a, 0xf3, 0x9b}),
                            Murmur3_32(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
}

TEST_F(TestHashingMurmur3, hashAlphanum128) {
    char message[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    EXPECT_EQ(MessageDigest({0x3b, 0x3e, 0xd7, 0x5f, 0x32, 0x1f, 0x99, 0x49,
                                        0xbc, 0xb9, 0xce, 0xa9, 0x3c, 0xd2, 0xad, 0xcb}),
                            Murmur3_128(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
}

TEST_F(TestHashingMurmur3, hashNumbers32) {
    char message[] = "12345678901234567890123456789012345678901234567890123456789012345678901234567890";
    EXPECT_EQ(MessageDigest({0xf8, 0xe0, 0x52, 0x87}),
                            Murmur3_32(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
}

TEST_F(TestHashingMurmur3, hashNumbers128) {
    char message[] = "12345678901234567890123456789012345678901234567890123456789012345678901234567890";
    EXPECT_EQ(MessageDigest({0xee, 0x6a, 0x87, 0xa4, 0x7f, 0x06, 0x63, 0x91,
                                        0xab, 0xf5, 0xd5, 0xa2, 0x27, 0xca, 0x4f, 0x77}),
                            Murmur3_128(0).update(wrapMemory(message, sizeof(message) - 1)).digest());
}