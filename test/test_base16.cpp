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
 * @file: test/test_base16.cpp
 * @author: soultaker
 ********************************************************************************/
#include <solace/base16.hpp>  // Class being tested

#include <solace/exception.hpp>
#include <cppunit/extensions/HelperMacros.h>


using namespace Solace;


class TestBase16: public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestBase16);
        CPPUNIT_TEST(testEncodedSize);
        CPPUNIT_TEST(testDecodedSize);

        CPPUNIT_TEST(testBasicEncoding);
        CPPUNIT_TEST(testBasicDecoding);

        CPPUNIT_TEST(decodingInvalidInputThrows);
        CPPUNIT_TEST(decodingInputOfUnEvenSizeThrows);
        CPPUNIT_TEST(decodingIntoSmallerBufferThrowsOverflow);
    CPPUNIT_TEST_SUITE_END();

public:

    void testEncodedSize() {
        CPPUNIT_ASSERT_EQUAL(static_cast<Base16Encoder::size_type>(0), Base16Encoder::encodedSize(0));
        CPPUNIT_ASSERT_EQUAL(static_cast<Base16Encoder::size_type>(2), Base16Encoder::encodedSize(1));
        CPPUNIT_ASSERT_EQUAL(static_cast<Base16Encoder::size_type>(4), Base16Encoder::encodedSize(2));
        CPPUNIT_ASSERT_EQUAL(static_cast<Base16Encoder::size_type>(6), Base16Encoder::encodedSize(3));
        CPPUNIT_ASSERT_EQUAL(static_cast<Base16Encoder::size_type>(8), Base16Encoder::encodedSize(4));
        CPPUNIT_ASSERT_EQUAL(static_cast<Base16Encoder::size_type>(10), Base16Encoder::encodedSize(5));
        CPPUNIT_ASSERT_EQUAL(static_cast<Base16Encoder::size_type>(12), Base16Encoder::encodedSize(6));
    }

    void testDecodedSize() {
        CPPUNIT_ASSERT_EQUAL(static_cast<Base16Decoder::size_type>(0), Base16Decoder::encodedSize(0));
        CPPUNIT_ASSERT_EQUAL(static_cast<Base16Decoder::size_type>(0), Base16Decoder::encodedSize(1));
        CPPUNIT_ASSERT_EQUAL(static_cast<Base16Decoder::size_type>(1), Base16Decoder::encodedSize(2));
        CPPUNIT_ASSERT_EQUAL(static_cast<Base16Decoder::size_type>(0), Base16Decoder::encodedSize(3));
        CPPUNIT_ASSERT_EQUAL(static_cast<Base16Decoder::size_type>(2), Base16Decoder::encodedSize(4));
        CPPUNIT_ASSERT_EQUAL(static_cast<Base16Decoder::size_type>(0), Base16Decoder::encodedSize(5));
        CPPUNIT_ASSERT_EQUAL(static_cast<Base16Decoder::size_type>(3), Base16Decoder::encodedSize(6));
    }

    void testBasicEncoding() {
        byte buffer[80];
        ByteBuffer dest(wrapMemory(buffer));

        Base16Encoder encoder(dest);

        const char* srcMem = "foobar";

        // BASE16("") = ""
        encoder.encode(wrapMemory(srcMem, 0));
        CPPUNIT_ASSERT(dest.viewWritten().empty());

        // BASE16("f") = "66"
        dest.rewind();
        encoder.encode(wrapMemory(srcMem, 1));
        CPPUNIT_ASSERT_EQUAL(wrapMemory("66", 2), dest.viewWritten().viewImmutableShallow());

        // BASE16("fo") = "666F"
        dest.rewind();
        encoder.encode(wrapMemory(srcMem, 2));
        CPPUNIT_ASSERT_EQUAL(wrapMemory("666F", 4), dest.viewWritten().viewImmutableShallow());

        // BASE16("foo") = "666F6F"
        dest.rewind();
        encoder.encode(wrapMemory(srcMem, 3));
        CPPUNIT_ASSERT_EQUAL(wrapMemory("666F6F", 6), dest.viewWritten().viewImmutableShallow());

        // BASE16("foob") = "666F6F62"
        dest.rewind();
        encoder.encode(wrapMemory(srcMem, 4));
        CPPUNIT_ASSERT_EQUAL(wrapMemory("666F6F62", 8), dest.viewWritten().viewImmutableShallow());

        // BASE16("fooba") = "666F6F6261"
        dest.rewind();
        encoder.encode(wrapMemory(srcMem, 5));
        CPPUNIT_ASSERT_EQUAL(wrapMemory("666F6F6261", 10), dest.viewWritten().viewImmutableShallow());

        // BASE16("foobar") = "666F6F626172"
        dest.rewind();
        encoder.encode(wrapMemory(srcMem, 6));
        CPPUNIT_ASSERT_EQUAL(wrapMemory("666F6F626172", 12), dest.viewWritten().viewImmutableShallow());


        dest.rewind();
        encoder.encode(wrapMemory("This is test message we want to encode", 38));
        CPPUNIT_ASSERT_EQUAL(wrapMemory("546869732069732074657374206D6573736167652077652077616E7420746F20656E636F6465",
                                        76),
                             dest.viewWritten().viewImmutableShallow());
    }

    void testBasicEncodingIterator() {
        auto encodedText = "546869732069732074657374206D6573736167652077652077616E7420746F20656E636F6465";
        auto message = wrapMemory("This is test message we want to encode", 38);

        int j = 0;
        for (auto i = base16Encode_begin(message),
             end = base16Encode_end(message); i != end; ++i) {

            CPPUNIT_ASSERT_EQUAL(StringView(encodedText + j, 2), *i);
            j += 2;
        }
    }

    void testBasicDecoding() {
        byte buffer[80];
        ByteBuffer dest(wrapMemory(buffer));

        Base16Decoder decoder(dest);

        // BASE16("") = ""
        decoder.encode(wrapMemory("", 0));
        CPPUNIT_ASSERT(dest.viewWritten().empty());

        // BASE16("f") = "66"
        dest.rewind();
        decoder.encode(wrapMemory("66", 2));
        CPPUNIT_ASSERT_EQUAL(wrapMemory("f", 1), dest.viewWritten().viewImmutableShallow());

        // BASE16("fo") = "666F"
        dest.rewind();
        decoder.encode(wrapMemory("666F", 4));
        CPPUNIT_ASSERT_EQUAL(wrapMemory("fo", 2), dest.viewWritten().viewImmutableShallow());

        // BASE16("foo") = "666F6F"
        dest.rewind();
        decoder.encode(wrapMemory("666F6F", 6));
        CPPUNIT_ASSERT_EQUAL(wrapMemory("foo", 3), dest.viewWritten().viewImmutableShallow());

        // BASE16("foob") = "666F6F62"
        dest.rewind();
        decoder.encode(wrapMemory("666F6F62", 8));
        CPPUNIT_ASSERT_EQUAL(wrapMemory("foob", 4), dest.viewWritten().viewImmutableShallow());

        // BASE16("fooba") = "666F6F6261"
        dest.rewind();
        decoder.encode(wrapMemory("666F6F6261", 10));
        CPPUNIT_ASSERT_EQUAL(wrapMemory("fooba", 5), dest.viewWritten().viewImmutableShallow());

        // BASE16("foobar") = "666F6F626172"
        dest.rewind();
        decoder.encode(wrapMemory("666F6F626172", 12));
        CPPUNIT_ASSERT_EQUAL(wrapMemory("foobar", 6), dest.viewWritten().viewImmutableShallow());

        dest.rewind();
        decoder.encode(wrapMemory("546869732069732074657374206D6573736167652077652077616E7420746F20656E636F6465", 76));
        CPPUNIT_ASSERT_EQUAL(wrapMemory("This is test message we want to encode", 38),
                             dest.viewWritten().viewImmutableShallow());

    }


    void decodingInvalidInputThrows() {
        byte buffer[30];
        ByteBuffer dest(wrapMemory(buffer));
        Base16Decoder v(dest);

        CPPUNIT_ASSERT(v.encode(wrapMemory("some! Not base16 (c)", 18)).isError());
    }

    void decodingInputOfUnEvenSizeThrows() {
        byte buffer[30];
        ByteBuffer dest(wrapMemory(buffer));
        Base16Decoder v(dest);

        CPPUNIT_ASSERT(v.encode(wrapMemory("666F6F626172", 11)).isError());
        CPPUNIT_ASSERT(v.encode(wrapMemory("666F6F626172", 9)).isError());
    }

    void decodingIntoSmallerBufferThrowsOverflow() {
        byte buffer[3];
        ByteBuffer dest(wrapMemory(buffer));
        Base16Decoder v(dest);

        CPPUNIT_ASSERT_THROW(v.encode(wrapMemory("666F6F626172", 12)), Solace::OverflowException);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestBase16);
