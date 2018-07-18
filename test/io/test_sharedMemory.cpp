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

#include "../interruptexception.hpp"

using namespace Solace;
using namespace Solace::IO;

class TestSharedMemory: public ::testing::Test {

public:

    void setUp() {
	}

    void tearDown() {
	}
};

TEST_F(TestSharedMemory, testCreate_InvalidFilename) {
    EXPECT_THROW(auto mem = SharedMemory::create(Path("/somewhere/XXX"), 128), IOException);
}

TEST_F(TestSharedMemory, testCreate_InvalidSize) {
    EXPECT_THROW(auto mem = SharedMemory::create(Path("/validname"), 0), IOException);
}

TEST_F(TestSharedMemory, testOpen_NonExisting) {
    const auto uuid = UUID::random();
    const auto name = Path::Root.join(uuid.toString());
    EXPECT_THROW(auto mem = SharedMemory::open(name), IOException);
}

TEST_F(TestSharedMemory, testOpen_Exclusive) {
    const auto uuid = UUID::random();
    const auto name = Path::Root.join(uuid.toString());
    auto mem1 = SharedMemory::create(name, 128, File::AccessMode::ReadWrite,
                                        File::Flags::Exlusive);

    EXPECT_THROW(auto mem = SharedMemory::open(name), IOException);
}

//to do : RETEST
// TEST_F(TestSharedMemory, testCreateAndMap) {
//     const SharedMemory::size_type memSize = 24;
//     bool isChild = false;
//     {
//         auto mem = SharedMemory::create(Path("/somename"), memSize);
//         EXPECT_TRUE(mem);

//         auto view = mem.map(SharedMemory::Access::Shared);
//         EXPECT_EQ(memSize, mem.size());
//         EXPECT_EQ(memSize, view.size());

//         const auto childPid = fork();
//         switch (childPid) {           /* Parent and child share mapping */
//         case -1:
//             FAIL() << ("fork");
//             return;

//         case 0: {                     /* Child: increment shared integer and exit */
//             EXPECT_EQ(memSize, mem.size());
//             EXPECT_EQ(memSize, view.size());

//             isChild = true;
//             WriteBuffer sb(view);
//             sb.write(getpid());
//             sb.write(StringView("child").view());

//             // Child will quit after that and this will signal the parent to read data from the shared memory.
//         } break;

//         default: {  /* Parent: wait for child to terminate */
//             if (waitpid(childPid, nullptr, 0) == -1) {
//                 const auto msg = String::join(": ", {"waitpid", strerror(errno)});
//                 FAIL() << (msg.c_str());
//             }

//             int viewedPid;
//             char message[10];
//             auto messageDest = wrapMemory(message);

//             ReadBuffer sb(view);
//             EXPECT_TRUE(sb.read(&viewedPid).isOk());
//             EXPECT_EQ(childPid, viewedPid);

//             EXPECT_TRUE(sb.read(messageDest, 5).isOk());
//             message[5] = 0;
//             EXPECT_EQ(StringView("child"), StringView(message));
//         } break;

//         }
//     }

//     if (isChild) {
//         throw InterruptTest();
//     }
// }