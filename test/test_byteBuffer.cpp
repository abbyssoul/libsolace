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
        {   // NullPointer smoke test
            // CPPUNIT_ASSERT_THROW(Buffer nullbuffer(321, NULL), IllegalArgumentException);
            Buffer nullbuffer(321, NULL);
            CPPUNIT_ASSERT_EQUAL(static_cast<Solace::Buffer::size_type>(0), nullbuffer.size());
        }

        {   // Fixed size constructor
			Buffer test(3102);

            CPPUNIT_ASSERT(!test.empty());
			CPPUNIT_ASSERT_EQUAL(static_cast<Solace::Buffer::size_type>(3102), test.size());
			test[0] = 19;
			test[2] = 17;
			test[1] = 4;
			test[test.size() - 1] = 255;
			CPPUNIT_ASSERT_EQUAL(static_cast<byte>(19), test.data()[0]);
			CPPUNIT_ASSERT_EQUAL(static_cast<byte>(4), test.data()[1]);
			CPPUNIT_ASSERT_EQUAL(static_cast<byte>(17), test.data()[2]);
			CPPUNIT_ASSERT_EQUAL(static_cast<byte>(255), test.data()[test.size() - 1]);
		}

        {   // Wrapping constructor
            byte example[] = {0, 1, 0, 3, 2, 1};  // size = 6
			Buffer test(sizeof(example), example);

            CPPUNIT_ASSERT(!test.empty());
			CPPUNIT_ASSERT_EQUAL(static_cast<Solace::Buffer::size_type>(6), test.size());

			for (size_t i = 0; i < test.size(); ++i)
				CPPUNIT_ASSERT_EQUAL(example[i], test.data()[i]);
		}

        {   // Copy-constructor
            byte example[] = {7, 5, 0, 2, 21, 15, 178};  // size = 7
            Buffer::size_type exampleSize = sizeof(example);
            Buffer b1(exampleSize, example);
			Buffer b2(b1);

            CPPUNIT_ASSERT_EQUAL(exampleSize, b1.size());
            CPPUNIT_ASSERT_EQUAL(exampleSize, b2.size());

			for (size_t i = 0; i < b1.size(); ++i) {
				CPPUNIT_ASSERT_EQUAL(example[i], b1.data()[i]);
				CPPUNIT_ASSERT_EQUAL(example[i], b2.data()[i]);
			}

			CPPUNIT_ASSERT_EQUAL(false, b2.isOwner());
		}
	}

    void testPositioning() {
        constexpr ByteBuffer::size_type testSize = 12;
        ByteBuffer buffer(testSize);

        CPPUNIT_ASSERT_EQUAL(testSize, buffer.capacity());
        CPPUNIT_ASSERT_EQUAL(testSize, buffer.limit());
        CPPUNIT_ASSERT_EQUAL(static_cast<ByteBuffer::size_type>(0), buffer.position());

        CPPUNIT_ASSERT_NO_THROW(buffer.position(buffer.position() + 12));
        CPPUNIT_ASSERT_NO_THROW(buffer.position(0));

        for (ByteBuffer::size_type i = 0; i < testSize; ++i) {
            buffer << 'a';
        }

        CPPUNIT_ASSERT_EQUAL(buffer.limit(), buffer.position());

        CPPUNIT_ASSERT_THROW(buffer.position(buffer.limit() + 1), IllegalArgumentException);
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
