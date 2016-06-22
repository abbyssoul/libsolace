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
    CPPUNIT_TEST_SUITE_END();

public:

	void testConstruction() {
	}

    void testPositioning() {
        constexpr ByteBuffer::size_type testSize = 12;
        ByteBuffer buffer(testSize);

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
        byte bytes[] = {'a', 'b', 'c', 0, 'd', 'f', 'g'};

        constexpr ByteBuffer::size_type testSize = sizeof(bytes);
        ByteBuffer buffer(testSize);

        CPPUNIT_ASSERT_NO_THROW(buffer.write(bytes, sizeof(bytes)));
        CPPUNIT_ASSERT_EQUAL(buffer.limit(), buffer.position());
    }

    void testGetByte() {
        byte bytes[] = {'a', 'b', 'c', 0, 'd', 'f', 'g'};

        constexpr ByteBuffer::size_type testSize = sizeof(bytes);
        ByteBuffer buffer(testSize);

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
        ByteBuffer buffer(testSize);

        CPPUNIT_ASSERT_NO_THROW(buffer.write(bytes, sizeof(bytes)));

        CPPUNIT_ASSERT_THROW(buffer.read(readBytes, sizeof(readBytes)), OverflowException);
        CPPUNIT_ASSERT_NO_THROW(buffer.read(3, readBytes, sizeof(readBytes)));

        for (ByteBuffer::size_type i = 0; i < sizeof(readBytes); ++i) {
            CPPUNIT_ASSERT_EQUAL(bytes[i + 3], readBytes[i]);
        }
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestByteBuffer);
