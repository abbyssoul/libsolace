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
 * @file: test/io/test_pipe.cpp
 * @author: soultaker
 *
 * Created on: 10/10/2016
*******************************************************************************/
#include <solace/io/pipe.hpp>  // Class being tested

#include <solace/exception.hpp>
#include <cppunit/extensions/HelperMacros.h>

#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "../interruptexception.hpp"


using namespace Solace;
using namespace Solace::IO;


class TestBlockingPipe: public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestBlockingPipe);
        CPPUNIT_TEST(testWrite);
        CPPUNIT_TEST(testWriteRead);
    CPPUNIT_TEST_SUITE_END();

public:

    void testWrite() {

        byte message[] = "Hello there";
        Pipe pipe;

        auto msgBuffer = wrapMemory(message);

        const auto written = pipe.write(msgBuffer);
        CPPUNIT_ASSERT(written.isOk());
        CPPUNIT_ASSERT_EQUAL(msgBuffer.size(), static_cast<MemoryView::size_type>(written.unwrap()));
    }

    void testWriteRead() {
        byte message[] = "Hello there";
        Pipe pipe;

        auto msgBuffer = wrapMemory(message);
        const auto written = pipe.write(msgBuffer);
        CPPUNIT_ASSERT(written.isOk());
        CPPUNIT_ASSERT_EQUAL(msgBuffer.size(), static_cast<MemoryView::size_type>(written.unwrap()));

        byte rcv[48];
        auto rcvBuffer = wrapMemory(rcv);
        const auto read = pipe.read(rcvBuffer);
        CPPUNIT_ASSERT(read.isOk());
        CPPUNIT_ASSERT_EQUAL(msgBuffer.size(), static_cast<MemoryView::size_type>(read.unwrap()));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestBlockingPipe);
