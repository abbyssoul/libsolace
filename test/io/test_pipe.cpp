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

    const auto written = pipe.write(msgBuffer);
    EXPECT_TRUE(written.isOk());
    EXPECT_EQ(msgBuffer.size(), static_cast<MemoryView::size_type>(written.unwrap()));
}

TEST_F(TestBlockingPipe, testWriteRead) {
    byte message[] = "Hello there";
    Pipe pipe;

    auto msgBuffer = wrapMemory(message);
    const auto written = pipe.write(msgBuffer);
    EXPECT_TRUE(written.isOk());
    EXPECT_EQ(msgBuffer.size(), static_cast<MemoryView::size_type>(written.unwrap()));

    byte rcv[48];
    auto rcvBuffer = wrapMemory(rcv);
    const auto read = pipe.read(rcvBuffer);
    EXPECT_TRUE(read.isOk());
    EXPECT_EQ(msgBuffer.size(), static_cast<MemoryView::size_type>(read.unwrap()));
}