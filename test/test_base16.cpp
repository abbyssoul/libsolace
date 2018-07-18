#include <solace/base16.hpp>  // Class being tested

#include <solace/exception.hpp>
#include <gtest/gtest.h>

using namespace Solace;

class TestBase16: public ::testing::Test  {

public:

    void setUp() {
    }

    void tearDown() {
    }
};

TEST_F(TestBase16, testEncodedSize) {
    EXPECT_EQ(static_cast<Base16Encoder::size_type>(0), Base16Encoder::encodedSize(0));
    EXPECT_EQ(static_cast<Base16Encoder::size_type>(2), Base16Encoder::encodedSize(1));
    EXPECT_EQ(static_cast<Base16Encoder::size_type>(4), Base16Encoder::encodedSize(2));
    EXPECT_EQ(static_cast<Base16Encoder::size_type>(6), Base16Encoder::encodedSize(3));
    EXPECT_EQ(static_cast<Base16Encoder::size_type>(8), Base16Encoder::encodedSize(4));
    EXPECT_EQ(static_cast<Base16Encoder::size_type>(10), Base16Encoder::encodedSize(5));
    EXPECT_EQ(static_cast<Base16Encoder::size_type>(12), Base16Encoder::encodedSize(6));
}

TEST_F(TestBase16, testDecodedSize) {
    EXPECT_EQ(static_cast<Base16Decoder::size_type>(0), Base16Decoder::encodedSize(0));
    EXPECT_EQ(static_cast<Base16Decoder::size_type>(0), Base16Decoder::encodedSize(1));
    EXPECT_EQ(static_cast<Base16Decoder::size_type>(1), Base16Decoder::encodedSize(2));
    EXPECT_EQ(static_cast<Base16Decoder::size_type>(1), Base16Decoder::encodedSize(3));
    EXPECT_EQ(static_cast<Base16Decoder::size_type>(2), Base16Decoder::encodedSize(4));
    EXPECT_EQ(static_cast<Base16Decoder::size_type>(2), Base16Decoder::encodedSize(5));
    EXPECT_EQ(static_cast<Base16Decoder::size_type>(3), Base16Decoder::encodedSize(6));
}

TEST_F(TestBase16, testBasicEncoding) {
    byte buffer[80];
    WriteBuffer dest(wrapMemory(buffer));
    Base16Encoder encoder(dest);

    const char* srcMem = "foobar";

    // BASE16("") = ""
    encoder.encode(wrapMemory(srcMem, 0));
    EXPECT_TRUE(dest.viewWritten().empty());

    // BASE16("f") = "66"
    dest.rewind();
    encoder.encode(wrapMemory(srcMem, 1));
    EXPECT_EQ(wrapMemory("66", 2), dest.viewWritten().viewImmutableShallow());

    // BASE16("fo") = "666F"
    dest.rewind();
    encoder.encode(wrapMemory(srcMem, 2));
    EXPECT_EQ(wrapMemory("666f", 4), dest.viewWritten().viewImmutableShallow());

    // BASE16("foo") = "666F6F"
    dest.rewind();
    encoder.encode(wrapMemory(srcMem, 3));
    EXPECT_EQ(wrapMemory("666f6f", 6), dest.viewWritten().viewImmutableShallow());

    // BASE16("foob") = "666F6F62"
    dest.rewind();
    encoder.encode(wrapMemory(srcMem, 4));
    EXPECT_EQ(wrapMemory("666f6f62", 8), dest.viewWritten().viewImmutableShallow());

    // BASE16("fooba") = "666F6F6261"
    dest.rewind();
    encoder.encode(wrapMemory(srcMem, 5));
    EXPECT_EQ(wrapMemory("666f6f6261", 10), dest.viewWritten().viewImmutableShallow());

    // BASE16("foobar") = "666F6F626172"
    dest.rewind();
    encoder.encode(wrapMemory(srcMem, 6));
    EXPECT_EQ(wrapMemory("666f6f626172", 12), dest.viewWritten().viewImmutableShallow());


    dest.rewind();
    encoder.encode(wrapMemory("This is test message we want to encode", 38));
    EXPECT_EQ(wrapMemory("546869732069732074657374206d6573736167652077652077616e7420746f20656e636f6465",
                                    76),
                            dest.viewWritten().viewImmutableShallow());
}

TEST_F(TestBase16, testBasicEncodingIterator) {
    auto encodedText = "546869732069732074657374206d6573736167652077652077616e7420746f20656e636f6465";
    auto message = wrapMemory("This is test message we want to encode", 38);

    int j = 0;
    for (auto i = base16Encode_begin(message),
            end = base16Encode_end(message); i != end; ++i) {

        EXPECT_EQ(StringView(encodedText + j, 2), *i);
        j += 2;
    }
}

TEST_F(TestBase16, testBasicDecoding) {
    byte buffer[80];
    WriteBuffer dest(wrapMemory(buffer));

    Base16Decoder decoder(dest);

    // BASE16("") = ""
    decoder.encode(wrapMemory("", 0));
    EXPECT_TRUE(dest.viewWritten().empty());

    // BASE16("f") = "66"
    dest.rewind();
    decoder.encode(wrapMemory("66", 2));
    EXPECT_EQ(wrapMemory("f", 1), dest.viewWritten().viewImmutableShallow());

    // BASE16("fo") = "666F"
    dest.rewind();
    decoder.encode(wrapMemory("666F", 4));
    EXPECT_EQ(wrapMemory("fo", 2), dest.viewWritten().viewImmutableShallow());

    // BASE16("foo") = "666F6F"
    dest.rewind();
    decoder.encode(wrapMemory("666F6F", 6));
    EXPECT_EQ(wrapMemory("foo", 3), dest.viewWritten().viewImmutableShallow());

    // BASE16("foob") = "666F6F62"
    dest.rewind();
    decoder.encode(wrapMemory("666F6F62", 8));
    EXPECT_EQ(wrapMemory("foob", 4), dest.viewWritten().viewImmutableShallow());

    // BASE16("fooba") = "666F6F6261"
    dest.rewind();
    decoder.encode(wrapMemory("666F6F6261", 10));
    EXPECT_EQ(wrapMemory("fooba", 5), dest.viewWritten().viewImmutableShallow());

    // BASE16("foobar") = "666F6F626172"
    dest.rewind();
    decoder.encode(wrapMemory("666F6F626172", 12));
    EXPECT_EQ(wrapMemory("foobar", 6), dest.viewWritten().viewImmutableShallow());

    dest.rewind();
    decoder.encode(wrapMemory("546869732069732074657374206D6573736167652077652077616E7420746F20656E636F6465", 76));
    EXPECT_EQ(wrapMemory("This is test message we want to encode", 38),
                            dest.viewWritten().viewImmutableShallow());

}

TEST_F(TestBase16, testDecodingIterator) {

    auto encodedText = wrapMemory("546869732069732074657374206d6573736167652077652077616e7420746f20656e636f6465");
    auto message = wrapMemory("This is test message we want to encode", 38);

    uint j = 0;
    for (auto i = base16Decode_begin(encodedText),
            end = base16Decode_end(encodedText); i != end; ++i, ++j) {

        EXPECT_EQ(message[j], *i);
    }
}

TEST_F(TestBase16, testDecodingIterator_InvalidInputLenght) {
    {
        auto const encodedText1 = wrapMemory("F");
        EXPECT_TRUE(base16Decode_end(encodedText1) == base16Decode_begin(encodedText1));
    }

    {
        auto const encodedText2 = wrapMemory("F65");
        auto i = base16Decode_begin(encodedText2);
        EXPECT_TRUE(i != base16Decode_end(encodedText2));
        EXPECT_TRUE(246 == *i);
        EXPECT_TRUE(base16Decode_end(encodedText2) == ++i);
    }
}

TEST_F(TestBase16, testDecodingIterator_InvalidData) {
    {
        auto const encodedText = wrapMemory("pX");
        EXPECT_TRUE(base16Decode_end(encodedText) == base16Decode_begin(encodedText));
    }

    {
        auto const encodedText = wrapMemory("F6k");
        auto i = base16Decode_begin(encodedText);
        EXPECT_TRUE(i != base16Decode_end(encodedText));
        EXPECT_TRUE(246 == *i);
        EXPECT_TRUE(base16Decode_end(encodedText) == ++i);
    }
}


TEST_F(TestBase16, decodingInvalidInputThrows) {
    byte buffer[30];
    WriteBuffer dest(wrapMemory(buffer));
    Base16Decoder v(dest);

    EXPECT_TRUE(v.encode(wrapMemory("some! Not base16 (c)", 18)).isError());
}

TEST_F(TestBase16, decodingInputOfUnEvenSizeThrows) {
    byte buffer[30];
    WriteBuffer dest(wrapMemory(buffer));
    Base16Decoder v(dest);

    EXPECT_TRUE(v.encode(wrapMemory("666F6F626172", 11)).isError());
    EXPECT_TRUE(v.encode(wrapMemory("666F6F626172", 9)).isError());
}

TEST_F(TestBase16, decodingIntoSmallerBufferErrors) {
    byte buffer[3];
    WriteBuffer dest(wrapMemory(buffer));
    Base16Decoder v(dest);

    EXPECT_TRUE(v.encode(wrapMemory("666F6F626172", 12)).isError());
}