/*
*  Copyright 2018 Ivan Ryabov
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

#include <solace/exception.hpp>
#include <gtest/gtest.h>

#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "../interruptexception.hpp"

using namespace Solace;
using namespace Solace::IO;


TEST(TestAnonSharedMemory, testCreate_InvalidSize) {
    EXPECT_THROW(auto mem = SharedMemory::createAnon(0), IllegalArgumentException);
}


TEST(TestAnonSharedMemory, testOpen_Exclusive) {
}


TEST(TestAnonSharedMemory, testFill) {
    auto buffer = SharedMemory::createAnon(47);

    buffer.view().fill(0);
    for (const auto& v : buffer.view()) {
        EXPECT_EQ(static_cast<byte>(0), v);
    }

    MutableMemoryView::size_type r = 0;
    buffer.view().fill(1);
    for (const auto& v : buffer.view()) {
        EXPECT_EQ(static_cast<byte>(1), v);
        r += v;
    }
    EXPECT_EQ(r, buffer.size());

    buffer.view().fill(211);
    for (const auto& v : buffer.view()) {
        EXPECT_EQ(static_cast<byte>(211), v);
    }
}

[[noreturn]]
void writeTextAndExit(uint64 memSize, MemoryResource& memBuffer) {
    EXPECT_EQ(memSize, memBuffer.size());

    {
        ByteWriter wb(std::move(memBuffer));
        wb.write(memSize);
        wb.write(StringView("child").view());
    }

    exit(0);
}


TEST(TestAnonSharedMemory, testShareAndMap) {
    const SharedMemory::size_type memSize = 24;
    auto memBuffer = SharedMemory::createAnon(memSize, SharedMemory::Access::Shared);

    EXPECT_TRUE(memBuffer);
    EXPECT_EQ(memSize, memBuffer.size());

    EXPECT_EXIT(writeTextAndExit(memSize, memBuffer), ::testing::ExitedWithCode(0), ".*");

    uint64 viewedPid;
    char message[10];
    auto messageDest = wrapMemory(message);

    ByteReader sb(memBuffer);
    EXPECT_TRUE(sb.read(&viewedPid).isOk());

    EXPECT_TRUE(sb.read(messageDest, 6).isOk());
    message[5] = 0;
    EXPECT_EQ(StringView("child"), StringView(message));
}
