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
 * @file: test/io/async/test_pipe.cpp
 * @author: soultaker
 *
 * Created on: 10/10/2016
*******************************************************************************/
#include <solace/io/async/pipe.hpp>  // Class being tested

#include <cppunit/extensions/HelperMacros.h>

#include <unistd.h>
#include <fcntl.h>


using namespace Solace;
using namespace Solace::IO;


class TestAsyncPipe : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestAsyncPipe);
        CPPUNIT_TEST(testAsyncWrite);
        CPPUNIT_TEST(testAsyncRead_epoll);
        CPPUNIT_TEST(testAsyncRead_poll);
        CPPUNIT_TEST(testAsyncReadWrite);
    CPPUNIT_TEST_SUITE_END();

protected:


    void asyncWrite(async::EventLoop& iocontext) {
        async::Pipe iopipe(iocontext);

        char message[] = "Hello there!";
        auto buffer = ByteBuffer(wrapMemory(message));

        bool writeComplete = false;
        iopipe.asyncWrite(buffer).then([&writeComplete, &iocontext](){
            writeComplete = true;
        });

        CPPUNIT_ASSERT(!writeComplete);

        // Should block untill event is triggered
        iocontext.runFor(300);

        CPPUNIT_ASSERT(writeComplete);
    }


    void asyncRead(async::EventLoop& iocontext) {
        async::Pipe iopipe(iocontext);

        char message[] = "Hello there!";
        auto messageBuffer = ByteBuffer(wrapMemory(message));

        char rcv_buffer[128];
        auto readBuffer = ByteBuffer(wrapMemory(rcv_buffer));

        bool readComplete = false;
        bool writeComplete = false;

        iopipe.asyncRead(readBuffer).then([&readComplete, &iocontext]() {
            readComplete = true;
        });

        iopipe.asyncWrite(messageBuffer).then([&writeComplete]() {
            writeComplete = true;
        });

        CPPUNIT_ASSERT(!readComplete);
        CPPUNIT_ASSERT(!writeComplete);

        iocontext.runFor(300);

        // Check that we have read something
        CPPUNIT_ASSERT_EQUAL(true, writeComplete);
        CPPUNIT_ASSERT_EQUAL(true, readComplete);
        // Check that we read as much as was written
        CPPUNIT_ASSERT_EQUAL(messageBuffer.position(), readBuffer.position());
    }


    void asyncReadWrite(async::EventLoop& iocontext) {

    }

public:

    void testAsyncWrite() {
        {
            auto iocontext = async::EventLoop(2, Selector::createEPoll(3));
            asyncWrite(iocontext);
        }

        {
            auto iocontext = async::EventLoop(2, Selector::createPoll(3));
            asyncWrite(iocontext);
        }
    }


    void testAsyncRead_epoll() {
        auto iocontext = async::EventLoop(2, Selector::createEPoll(3));
        asyncRead(iocontext);
    }

    void testAsyncRead_poll() {
        auto iocontext = async::EventLoop(2, Selector::createPoll(3));
        asyncRead(iocontext);
    }


    void testAsyncReadWrite() {
        {
            auto iocontext = async::EventLoop(2, Selector::createEPoll(3));
            asyncReadWrite(iocontext);
        }

        {
            auto iocontext = async::EventLoop(2, Selector::createPoll(3));
            asyncReadWrite(iocontext);
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestAsyncPipe);
