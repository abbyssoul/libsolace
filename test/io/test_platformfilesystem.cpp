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
 * @file: test/io/test_platformFilesystem.cpp
 * @author: soultaker
 *
 * Created on: 20/06/2016
*******************************************************************************/
#include <solace/io/platformFilesystem.hpp>  // Class being tested

#include <solace/memoryManager.hpp>
#include <solace/exception.hpp>
#include <solace/uuid.hpp>

#include <gtest/gtest.h>

#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <memory.h>

using namespace Solace;
using namespace Solace::IO;

class TestPlatformFs : public ::testing::Test {

public:
    MemoryManager _memoryManager;

    class DirectoryGuard {
    public:
        ~DirectoryGuard() {
            chdir(_cwd);
            std::free(_cwd);
        }

        DirectoryGuard(char* cwd) : _cwd(cwd)
        {}

        DirectoryGuard() :
        #ifdef SOLACE_PLATFORM_LINUX
            DirectoryGuard(get_current_dir_name())
        #else
            DirectoryGuard(getcwd(nullptr, 0))
        #endif
        {}

    private:
        char* _cwd;

    };

public:

    TestPlatformFs(): _memoryManager(4096)
    {
    }

    void setUp() {
	}

    void tearDown() {
	}
};

TEST_F(TestPlatformFs, testCreation) {
    const auto fileUID = UUID::random();
    const auto filename = Path::parse(String::join("-", {"test", fileUID.toString()}).view()).unwrap();
    const auto& fileUIDBytes = fileUID.view();

    auto fs = PlatformFilesystem();
    if (fs.exists(filename)) {
        fs.unlink(filename);
    }

    EXPECT_TRUE(!fs.exists(filename));
    {
        auto f = fs.create(filename);
        EXPECT_TRUE(fs.exists(filename));

        const auto written = f->write(fileUIDBytes);
        EXPECT_TRUE(written.isOk());

        const MemoryView::size_type bytesWriten = written.unwrap();
        EXPECT_EQ(fileUIDBytes.size(), bytesWriten);

        f->seek(0, File::Seek::Set);

        WriteBuffer readBuffer(_memoryManager.create(fileUIDBytes.size()));
        const auto read = f->read(readBuffer);
        EXPECT_TRUE(read.isOk());
        const MemoryView::size_type bytesRead = read.unwrap();
        EXPECT_EQ(fileUIDBytes.size(), bytesRead);
        EXPECT_EQ(false, readBuffer.hasRemaining());
        readBuffer.flip();

        EXPECT_TRUE(fileUIDBytes == readBuffer.viewRemaining());
    }

    EXPECT_TRUE(fs.unlink(filename));
    EXPECT_TRUE(!fs.exists(filename));
}

TEST_F(TestPlatformFs, testCreationAndRemoval) {
    const auto fileUID = UUID::random();
    const auto filename = Path::parse(String::join("-", {"test", fileUID.toString()}).view()).unwrap();
    const auto& fileUIDBytes = fileUID.view();

    auto fs = PlatformFilesystem();
    if (fs.exists(filename)) {
        fs.unlink(filename);
    }

    EXPECT_TRUE(!fs.exists(filename));

    {
        auto f = fs.create(filename);
        EXPECT_TRUE(fs.exists(filename));
        f->write(fileUIDBytes);
        f->close();

        WriteBuffer readBuffer(_memoryManager.create(fileUIDBytes.size()));
        EXPECT_THROW(f->seek(0, File::Seek::Set), NotOpen);
        EXPECT_THROW(f->read(readBuffer), NotOpen);
    }

    // Attempt to 'create' already existing file
    EXPECT_THROW(auto f = fs.create(filename), IOException);
    EXPECT_EQ(fileUIDBytes.size(), fs.getFileSize(filename));
    EXPECT_TRUE(fs.isFile(filename));
    EXPECT_TRUE(!fs.isDirectory(filename));

    {
        auto f = fs.open(filename);

        WriteBuffer readBuffer(_memoryManager.create(fileUIDBytes.size()));
        const auto read = f->read(readBuffer);
        EXPECT_TRUE(read.isOk());
        const MemoryView::size_type bytesRead = read.unwrap();

        EXPECT_EQ(fileUIDBytes.size(), bytesRead);
        EXPECT_EQ(false, readBuffer.hasRemaining());
        readBuffer.flip();

        EXPECT_TRUE(fileUIDBytes == readBuffer.viewRemaining());
    }

    timeval timeOfDay;
    gettimeofday(&timeOfDay, nullptr);
    auto t = fs.getTimestamp(filename);
    EXPECT_LT(timeOfDay.tv_sec - t, 2);

    EXPECT_TRUE(fs.unlink(filename));
    EXPECT_TRUE(!fs.exists(filename));
}

TEST_F(TestPlatformFs, testGetExecPath) {
    auto fs = PlatformFilesystem();

    auto pathToThisTest = fs.getExecPath();
    EXPECT_EQ(String("test_solace"), pathToThisTest.last());
}

TEST_F(TestPlatformFs, testThereIsADirectory) {
    auto fs = PlatformFilesystem();

    auto pathToThisTest = fs.getExecPath();
    auto realPathToThisTest = fs.realPath(pathToThisTest);

    EXPECT_TRUE(fs.isFile(realPathToThisTest));
    EXPECT_TRUE(fs.isDirectory(realPathToThisTest.getParent()));
}

TEST_F(TestPlatformFs, testWorkingDirectory) {
    auto fs = PlatformFilesystem();

    auto cwd = fs.getWorkingDirectory();
    EXPECT_TRUE(fs.isDirectory(cwd));
    EXPECT_TRUE(!fs.isFile(cwd));

    DirectoryGuard guardCwd;

    // Commented out at it changes run-time environment.
    fs.setWorkingDirectory(cwd.getParent());
    EXPECT_EQ(cwd.getParent(), fs.getWorkingDirectory());
}

TEST_F(TestPlatformFs, testTemp) {
    const auto fileUID = UUID::random();
    const auto& fileUIDBytes = fileUID.view();

    auto fs = PlatformFilesystem();
    {
        auto f = fs.createTemp();

        const auto written = f->write(fileUIDBytes);
        EXPECT_TRUE(written.isOk());
        const MemoryView::size_type bytesWriten = written.unwrap();
        EXPECT_EQ(fileUIDBytes.size(), bytesWriten);
        EXPECT_EQ(bytesWriten, static_cast<decltype(bytesWriten)>(f->tell()));

        f->seek(0, File::Seek::Set);

        WriteBuffer readBuffer(_memoryManager.create(fileUIDBytes.size()));
        const auto read = f->read(readBuffer);
        EXPECT_TRUE(read.isOk());
        const MemoryView::size_type bytesRead = read.unwrap();
        EXPECT_EQ(fileUIDBytes.size(), bytesRead);
        EXPECT_EQ(false, readBuffer.hasRemaining());
        readBuffer.flip();

        EXPECT_TRUE(fileUIDBytes == readBuffer.viewRemaining());
    }
}
