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

class TestBlockingPipe: public ::testing::Test {

public:

    void setUp() {
	}

    void tearDown() {
	}
};

TEST_F(TestBlockingPipe, testWrite) {

    byte message[] = "Hello there";
    Pipe pipe;

    auto msgBuffer = wrapMemory(message);

    auto const written = pipe.write(msgBuffer);
    EXPECT_TRUE(written.isOk());
    EXPECT_EQ(msgBuffer.size(), static_cast<MutableMemoryView::size_type>(written.unwrap()));
}

TEST_F(TestBlockingPipe, testWriteRead) {
    byte message[] = "Hello there";
    Pipe pipe;

    auto msgBuffer = wrapMemory(message);
    auto const written = pipe.write(msgBuffer);
    EXPECT_TRUE(written.isOk());
    EXPECT_EQ(msgBuffer.size(), static_cast<MutableMemoryView::size_type>(written.unwrap()));

    byte rcv[48];
    auto rcvBuffer = wrapMemory(rcv);
    auto const read = pipe.read(rcvBuffer);
    EXPECT_TRUE(read.isOk());
    EXPECT_EQ(msgBuffer.size(), static_cast<MutableMemoryView::size_type>(read.unwrap()));
}
