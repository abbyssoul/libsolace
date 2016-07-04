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
#include <solace/io/sharedMemory.hpp>  // Class being tested

#include <solace/uuid.hpp>
#include <solace/io/file.hpp>
#include <solace/io/ioexception.hpp>
#include <cppunit/extensions/HelperMacros.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>


using namespace Solace;
using namespace Solace::IO;


class TestSharedMemory: public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestSharedMemory);
        CPPUNIT_TEST(testCreate_InvalidFilename);
        CPPUNIT_TEST(testCreate_InvalidSize);

        CPPUNIT_TEST(testOpen_NonExisting);
        CPPUNIT_TEST(testOpen_Exclusive);

        CPPUNIT_TEST(testCreateAndMap);
    CPPUNIT_TEST_SUITE_END();

public:

    void testCreate_InvalidFilename() {
        CPPUNIT_ASSERT_THROW(auto mem = SharedMemory::create(Path("/somewhere/XXX"), 128), IOException);
    }

    void testCreate_InvalidSize() {
        CPPUNIT_ASSERT_THROW(auto mem = SharedMemory::create(Path("/validname"), 0), IOException);
    }

    void testOpen_NonExisting() {
        const auto uuid = UUID::random();
        const auto name = Path::Root.join(uuid.toString());
        CPPUNIT_ASSERT_THROW(auto mem = SharedMemory::open(name), IOException);
    }

    void testOpen_Exclusive() {
        const auto uuid = UUID::random();
        const auto name = Path::Root.join(uuid.toString());
        auto mem1 = SharedMemory::create(name, 128, File::AccessMode::ReadWrite,
                                         File::Flags::Exlusive);

        CPPUNIT_ASSERT_THROW(auto mem = SharedMemory::open(name), IOException);
    }

    void testCreateAndMap() {
        const SharedMemory::size_type memSize = 24;
        auto mem = SharedMemory::create(Path("/somename"), memSize);

        auto view = mem.map(SharedMemory::MappingAccess::Shared);
        CPPUNIT_ASSERT_EQUAL(memSize, mem.size());
        CPPUNIT_ASSERT_EQUAL(memSize, view.size());

        const auto childPid = fork();
        switch (childPid) {           /* Parent and child share mapping */
        case -1:
            CPPUNIT_FAIL("fork");
            return;

        case 0: {                     /* Child: increment shared integer and exit */
            ByteBuffer sb(view);
            sb << getpid();
            sb.write("child", 5);

            exit(EXIT_SUCCESS);
        }

        default: {  /* Parent: wait for child to terminate */
            if (wait(NULL) == -1)
                CPPUNIT_FAIL("wait");

            ByteBuffer sb(view);

            int viewedPid;
            char message[10];

            sb >> viewedPid;
            CPPUNIT_ASSERT_EQUAL(childPid, viewedPid);

            sb.read(message, 5);
            message[5] = 0;
            CPPUNIT_ASSERT_EQUAL(String("child"), String(message));
        }

        }
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestSharedMemory);
