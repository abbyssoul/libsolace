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
 * @file: test/io/test_sharedMemory.cpp
 * @author: soultaker
 *
 * Created on: 03/07/2016
*******************************************************************************/
#include <solace/io/mappedMemory.hpp>  // Class being tested

#include <solace/exception.hpp>
#include <solace/string.hpp>
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


class TestAnonSharedMemory: public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestAnonSharedMemory);
        CPPUNIT_TEST(testCreate_InvalidSize);

        CPPUNIT_TEST(testOpen_Exclusive);

        CPPUNIT_TEST(testShareAndMap);
    CPPUNIT_TEST_SUITE_END();

public:


    void testCreate_InvalidSize() {
        CPPUNIT_ASSERT_THROW(auto mem = MappedMemoryView::create(0), IllegalArgumentException);
    }


    void testOpen_Exclusive() {
    }

    void testFill() {
        auto buffer = MappedMemoryView::create(47);

        buffer.fill(0);
        for (const auto& v : buffer) {
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0), v);
        }

        MemoryView::size_type r = 0;
        buffer.fill(1);
        for (const auto& v : buffer) {
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(1), v);
            r += v;
        }
        CPPUNIT_ASSERT_EQUAL(r, buffer.size());

        buffer.fill(211);
        for (const auto& v : buffer) {
            CPPUNIT_ASSERT_EQUAL(static_cast<byte>(211), v);
        }
    }


    void testShareAndMap() {
        const MappedMemoryView::size_type memSize = 24;
        bool isChild = false;
        {
            auto view = MappedMemoryView::create(memSize, MappedMemoryView::Access::Shared);
            CPPUNIT_ASSERT_EQUAL(memSize, view.size());

            const auto childPid = fork();
            switch (childPid) {           /* Parent and child share mapping */
            case -1:
                CPPUNIT_FAIL("fork");
                return;

            case 0: {                     /* Child: increment shared integer and exit */
                isChild = true;
                ByteBuffer sb(view.viewShallow());
                sb << getpid();
                sb.write("child", 5);

            } break;

            default: {  /* Parent: wait for child to terminate */
                if (waitpid(childPid, nullptr, 0) == -1) {
                    const auto msg = String::join(": ", {"waitpid", strerror(errno)});
                    CPPUNIT_FAIL(msg.c_str());
                }


                int viewedPid;
                char message[10];

                ByteBuffer sb(view.viewShallow());
                sb >> viewedPid;
                CPPUNIT_ASSERT_EQUAL(childPid, viewedPid);

                sb.read(message, 5);
                message[5] = 0;
                CPPUNIT_ASSERT_EQUAL(String("child"), String(message));
            }

            }
        }
        if (isChild) {
            throw InterruptTest();
        }
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestAnonSharedMemory);
