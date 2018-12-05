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
 * @file: test/test_base64.cpp
 * @author: soultaker
 ********************************************************************************/
#include <solace/base64.hpp>  // Class being tested

#include <solace/exception.hpp>
#include <gtest/gtest.h>

#include <cstring>

using namespace Solace;


TEST(TestBase64, testEncodedSize) {
    EXPECT_EQ(0, Base64Encoder::encodedSize(0));
    EXPECT_EQ(4, Base64Encoder::encodedSize(1));
    EXPECT_EQ(4, Base64Encoder::encodedSize(2));
    EXPECT_EQ(4, Base64Encoder::encodedSize(3));
    EXPECT_EQ(8, Base64Encoder::encodedSize(4));
    EXPECT_EQ(8, Base64Encoder::encodedSize(5));
    EXPECT_EQ(8, Base64Encoder::encodedSize(6));
}

TEST(TestBase64, testDecodedSize) {
    const char* nullStr = nullptr;
    EXPECT_EQ(0, Base64Decoder::decodedSize(wrapMemory(nullStr, 0)));
    EXPECT_EQ(1, Base64Decoder::decodedSize(wrapMemory("Zg==", 4)));
    EXPECT_EQ(2, Base64Decoder::decodedSize(wrapMemory("Zm8=", 4)));
    EXPECT_EQ(3, Base64Decoder::decodedSize(wrapMemory("Zm9v", 4)));
    EXPECT_EQ(4, Base64Decoder::decodedSize(wrapMemory("Zm9vYg==", 8)));
    EXPECT_EQ(5, Base64Decoder::decodedSize(wrapMemory("Zm9vYmE=", 8)));
    EXPECT_EQ(6, Base64Decoder::decodedSize(wrapMemory("Zm9vYmFy", 8)));
}

TEST(TestBase64, testBasicEncoding) {
    byte buffer[64];
    ByteWriter dest(wrapMemory(buffer));
    Base64Encoder encoder(dest);

    const char* srcMem = "foobar";

    //  BASE64("") = ""
    encoder.encode(wrapMemory(srcMem, 0));
    EXPECT_TRUE(dest.viewWritten().empty());

    //  BASE64("f") = "Zg=="
    dest.rewind();
    encoder.encode(wrapMemory(srcMem, 1));
    EXPECT_EQ(wrapMemory("Zg==", 4), dest.viewWritten());

    //  BASE64("fo") = "Zm8="
    dest.rewind();
    encoder.encode(wrapMemory(srcMem, 2));
    EXPECT_EQ(wrapMemory("Zm8=", 4), dest.viewWritten());

    //  BASE64("foo") = "Zm9v"
    dest.rewind();
    encoder.encode(wrapMemory(srcMem, 3));
    EXPECT_EQ(wrapMemory("Zm9v", 4), dest.viewWritten());

    //  BASE64("foob") = "Zm9vYg=="
    dest.rewind();
    encoder.encode(wrapMemory(srcMem, 4));
    EXPECT_EQ(wrapMemory("Zm9vYg==", 8), dest.viewWritten());

    //  BASE64("fooba") = "Zm9vYmE="
    dest.rewind();
    encoder.encode(wrapMemory(srcMem, 5));
    EXPECT_EQ(wrapMemory("Zm9vYmE=", 8), dest.viewWritten());

    //  BASE64("foobar") = "Zm9vYmFy"
    dest.rewind();
    encoder.encode(wrapMemory(srcMem, 6));
    EXPECT_EQ(wrapMemory("Zm9vYmFy", 8), dest.viewWritten());


    dest.rewind();
    encoder.encode(wrapMemory("This is test message we want to encode", 38));
    EXPECT_EQ(wrapMemory("VGhpcyBpcyB0ZXN0IG1lc3NhZ2Ugd2Ugd2FudCB0byBlbmNvZGU=", 52),
                            dest.viewWritten());
}

TEST(TestBase64, testBasicDecoding) {
    byte buffer[64];
    ByteWriter dest(wrapMemory(buffer));

    Base64Decoder decoder(dest);

    //  BASE64("") = ""
    decoder.encode(wrapMemory("", 0));
    EXPECT_TRUE(dest.viewWritten().empty());

    //  BASE64("f") = "Zg=="
    dest.rewind();
    decoder.encode(wrapMemory("Zg==", 4));
    EXPECT_EQ(wrapMemory("f", 1), dest.viewWritten());

    //  BASE64("fo") = "Zm8="
    dest.rewind();
    decoder.encode(wrapMemory("Zm8=", 4));
    EXPECT_EQ(wrapMemory("fo", 2), dest.viewWritten());

    //  BASE64("foo") = "Zm9v"
    dest.rewind();
    decoder.encode(wrapMemory("Zm9v", 4));
    EXPECT_EQ(wrapMemory("foo", 3), dest.viewWritten());

    //  BASE64("foob") = "Zm9vYg=="
    dest.rewind();
    decoder.encode(wrapMemory("Zm9vYg==", 8));
    EXPECT_EQ(wrapMemory("foob", 4), dest.viewWritten());

    //  BASE64("fooba") = "Zm9vYmE="
    dest.rewind();
    decoder.encode(wrapMemory("Zm9vYmE=", 8));
    EXPECT_EQ(wrapMemory("fooba", 5), dest.viewWritten());

    //  BASE64("foobar") = "Zm9vYmFy"
    dest.rewind();
    decoder.encode(wrapMemory("Zm9vYmFy", 8));
    EXPECT_EQ(wrapMemory("foobar", 6), dest.viewWritten());

    dest.rewind();
    decoder.encode(wrapMemory("VGhpcyBpcyB0ZXN0IG1lc3NhZ2Ugd2Ugd2FudCB0byBlbmNvZGU=", 53));
    EXPECT_EQ(wrapMemory("This is test message we want to encode", 38),
                            dest.viewWritten());

}

TEST(TestBase64, testBasicUrlEncoding) {
    byte buffer[70];
    ByteWriter dest(wrapMemory(buffer));

    Base64UrlEncoder encoder(dest);

    encoder.encode(wrapMemory("This is test message encoded as a URL safe base64", 49));
    EXPECT_EQ(wrapMemory("VGhpcyBpcyB0ZXN0IG1lc3NhZ2UgZW5jb2RlZCBhcyBhIFVSTCBzYWZlIGJhc2U2NA==", 68),
                            dest.viewWritten());
}

TEST(TestBase64, testBasicUrlDecoding) {
    byte buffer[70];
    ByteWriter dest(wrapMemory(buffer));

    Base64UrlDecoder encoder(dest);

    encoder.encode(wrapMemory("VGhpcyBpcyB0ZXN0IG1lc3NhZ2UgZW5jb2RlZCBhcyBhIFVSTCBzYWZlIGJhc2U2NA==", 68));
    EXPECT_EQ(wrapMemory("This is test message encoded as a URL safe base64", 49),
                            dest.viewWritten());
}


TEST(TestBase64, testMultilineMessageEncoding) {
    byte buffer[90];
    ByteWriter dest(wrapMemory(buffer));

    const char* srcMem = "This is line one\nThis is line two\nThis is line three\nAnd so on...\n";
    const char* expectedMsg =
            "VGhpcyBpcyBsaW5lIG9uZQpUaGlzIGlzIGxpbmUgdHdvClRoaXMgaXMgbGluZSB0aHJlZQpBbmQgc28gb24uLi4K";

    Base64Encoder(dest)
            .encode(wrapMemory(srcMem, strlen(srcMem)));

    EXPECT_EQ(wrapMemory(expectedMsg, strlen(expectedMsg)), dest.viewWritten());
}

TEST(TestBase64, testMultilineMessageDecoding) {
    byte buffer[90];
    ByteWriter dest(wrapMemory(buffer));

    const char* expectedMsg = "This is line one\nThis is line two\nThis is line three\nAnd so on...\n";
    const char* srcMem = "VGhpcyBpcyBsaW5lIG9uZQpUaGlzIGlzIGxpbmUgdHdvClRoaXMgaXMgbGluZSB0aHJlZQpBbmQgc28gb24uLi4K";

    Base64Decoder(dest)
            .encode(wrapMemory(srcMem, strlen(srcMem)));

    EXPECT_EQ(wrapMemory(expectedMsg, strlen(expectedMsg)), dest.viewWritten());
}

TEST(TestBase64, testUnicodeCharactersEncoding) {
    byte buffer[30];
    ByteWriter dest(wrapMemory(buffer));

    const char* srcMem = "foo © bar 𝌆 baz";
    const char* expectedMsg = "Zm9vIMKpIGJhciDwnYyGIGJheg==";

    Base64Encoder(dest)
            .encode(wrapMemory(srcMem, strlen(srcMem)));

    EXPECT_EQ(wrapMemory(expectedMsg, strlen(expectedMsg)), dest.viewWritten());
}

TEST(TestBase64, testUnicodeCharactersDecoding) {
    byte buffer[30];
    ByteWriter dest(wrapMemory(buffer));

    const char* srcMem = "Zm9vIMKpIGJhciDwnYyGIGJheg==";
    const char* expectedMsg = "foo © bar 𝌆 baz";

    Base64Decoder(dest)
            .encode(wrapMemory(srcMem, strlen(srcMem)));

    EXPECT_EQ(wrapMemory(expectedMsg, strlen(expectedMsg)), dest.viewWritten());
}
