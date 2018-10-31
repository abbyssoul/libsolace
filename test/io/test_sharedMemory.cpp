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
#include <solace/exception.hpp>

#include <gtest/gtest.h>

#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>


using namespace Solace;
using namespace Solace::IO;


TEST(TestSharedMemory, testCreate_InvalidFilename) {
    EXPECT_THROW(auto mem = SharedMemory::create(makePath(Path::Root, "somewhere", "XXX"), 128), IOException);
}

TEST(TestSharedMemory, testCreate_InvalidSize) {
    EXPECT_THROW(auto mem = SharedMemory::create(makePath(Path::Root, "validname"), 0), IOException);
}

TEST(TestSharedMemory, testOpen_NonExisting) {
    auto const uuid = makeRandomUUID();
    auto const name = makePath(Path::Root, uuid.toString());
    EXPECT_THROW(auto mem = SharedMemory::open(name), IOException);
}

TEST(TestSharedMemory, testOpen_Exclusive) {
    auto const uuid = makeRandomUUID();

    // Create exclusive memory
    auto const mem1 = SharedMemory::create(makePath(Path::Root, uuid.toString()),
                                     128, File::AccessMode::ReadWrite,
                                        File::Flags::Exlusive);

    // Pretend someone else wants to open it too.
    auto const name = makePath(Path::Root, uuid.toString());
    EXPECT_THROW(auto mem = SharedMemory::open(name), IOException);
}


[[noreturn]]
void writeMessageAndExit(uint64 memSize, MemoryResource& view) {
    EXPECT_EQ(memSize, view.size());

    {
        ByteWriter sb(view);
        sb.write(memSize);
        sb.write(StringView("child").view());
    }

    exit(0);
}

TEST(TestSharedMemory, testCreateAndMap) {
    const SharedMemory::size_type memSize = 24;

    auto mem = SharedMemory::create(makePath(Path::Root, "somename"), memSize);
    EXPECT_TRUE(mem);

    auto view = mem.map(SharedMemory::Access::Shared);
    EXPECT_EQ(memSize, mem.size());
    EXPECT_EQ(memSize, view.size());

    EXPECT_EXIT(writeMessageAndExit(memSize, view), ::testing::ExitedWithCode(0), ".*");

    uint64 viewedMemsize;
    char message[10];
    auto messageDest = wrapMemory(message);

    ByteReader sb(view);
    EXPECT_TRUE(sb.read(&viewedMemsize).isOk());
    EXPECT_EQ(memSize, viewedMemsize);

    EXPECT_TRUE(sb.read(messageDest, 6).isOk()); message[5] = 0;
    EXPECT_EQ(StringView("child"), StringView(message));
}
