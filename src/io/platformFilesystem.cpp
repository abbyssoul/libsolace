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
 * @file: io/platformFilesystem.cpp
 *
 *  Created by soultaker on 16/06/16.
*******************************************************************************/
#include "solace/io/platformFilesystem.hpp"
#include "solace/exception.hpp"


#include <memory>   // std::unique_ptr<>
#include <ctime>
#include <unistd.h>
#include <sys/stat.h>
#include <glob.h>

#ifdef SOLACE_PLATFORM_BSD
#include <sys/sysctl.h>
#elif defined(SOLACE_PLATFORM_APPLE)
#include <mach-o/dyld.h>       /* _NSGetExecutablePath */
#endif


using namespace Solace;
using namespace Solace::IO;



PlatformFilesystem::BufferedFile::BufferedFile(FILE* fp) : File(fileno(fp)),
    _fp(fp)
{
}


PlatformFilesystem::BufferedFile::~BufferedFile() {
    close();
}

IOObject::IOResult
PlatformFilesystem::BufferedFile::read(MutableMemoryView& buffer) {
    if (!_fp) {
        raise<NotOpen>();
    }

    const auto bytesRead = ::fread(buffer.dataAddress(), 1, buffer.size(), _fp);
    // NOTE: Number of bytes read can be less then 'bytesToRead' if there are no more data in the file or end of file
    // has been reached.

    return Ok(bytesRead);
}


IOObject::IOResult
PlatformFilesystem::BufferedFile::write(const MemoryView& buffer) {
    if (!_fp) {
        raise<NotOpen>();
    }

    const auto bytesWritten = ::fwrite(buffer.dataAddress(), 1, buffer.size(), _fp);
    if (bytesWritten != buffer.size()) {
        raise<IOException>(errno);
    }

    return Ok(bytesWritten);
}


File::size_type
PlatformFilesystem::BufferedFile::seek(size_type offset, File::Seek type) {
    if (!_fp) {
        raise<NotOpen>();
    }

    int result = 0;
    switch (type) {
        case Seek::Set:     result = fseeko(_fp, offset, SEEK_SET); break;
        case Seek::Current: result = fseeko(_fp, offset, SEEK_CUR); break;
        case Seek::End:     result = fseeko(_fp, offset, SEEK_END); break;
    }

    if (0 != result) {
        raise<IOException>(errno);
    }

    return result;
}


void PlatformFilesystem::BufferedFile::close() {
    if (_fp) {
        fclose(_fp);
        _fp = nullptr;
    }

    invalidateFd();
}


PlatformFilesystem::BufferedFile::size_type
PlatformFilesystem::BufferedFile::tell() {
    if (!_fp) {
        raise<NotOpen>();
    }

    return ftell(_fp);
}





std::unique_ptr<File>
PlatformFilesystem::create(const Path& path) {
    const auto& pathString = path.toString();
    auto fp = fopen(pathString.c_str(), "w+x");

    if (!fp) {
        raise<IOException>(errno);
    }

    return std::make_unique<PlatformFilesystem::BufferedFile>(fp);
}


std::unique_ptr<File>
PlatformFilesystem::open(const Path& path) {
    const auto& pathString = path.toString();
    auto fp = fopen(pathString.c_str(), "r+");

    if (!fp) {
        raise<IOException>(errno);
    }

    return std::make_unique<PlatformFilesystem::BufferedFile>(fp);
}


bool
PlatformFilesystem::unlink(const Path& path) {
    const auto& pathString = path.toString();
    auto err = ::remove(pathString.c_str());

    if (err) {
        raise<IOException>(errno);
    }

    return true;
}


bool
PlatformFilesystem::exists(const Path& path) const {
    struct stat sb;

    const auto& pathString = path.toString();
    return (stat(pathString.c_str(), &sb ) == 0);
}


bool
PlatformFilesystem::isFile(const Path& path) const {
    const auto& pathString = path.toString();

    struct stat sb;
    if (stat(pathString.c_str(), &sb )) {
        raise<IOException>(errno);
    }

    return S_ISREG(sb.st_mode);
}


bool
PlatformFilesystem::isDirectory(const Path& path) const {
    const auto& pathString = path.toString();

    struct stat sb;
    if (stat(pathString.c_str(), &sb )) {
        raise<IOException>(errno);
    }

    return S_ISDIR(sb.st_mode);
}


time_t
PlatformFilesystem::getTimestamp(const Path& path) const {
    const auto& pathString = path.toString();

    struct stat sb;
    if (stat(pathString.c_str(), &sb )) {
        raise<IOException>(errno);
    }

    return sb.st_mtime;
}


PlatformFilesystem::size_type
PlatformFilesystem::getFileSize(const Path& path) const {
    struct stat sb;

    const auto& pathString = path.toString();
    if (stat(pathString.c_str(), &sb )) {
        raise<IOException>(errno);
    }

    return sb.st_size;
}


Path
PlatformFilesystem::realPath(const Path& path) const {
    auto const pathString = path.toString();
    std::unique_ptr<char, decltype(std::free)*> real_path{realpath(pathString.c_str(), nullptr), std::free};

    return (real_path)
            ? Path::parse(real_path.get()).unwrap()
            : allocPath(String::Empty);
}


std::unique_ptr<PlatformFilesystem::BufferedFile>
PlatformFilesystem::createTemp() {
    auto fp = tmpfile();

    if (!fp) {
        raise<IOException>(errno);
    }

    return std::make_unique<PlatformFilesystem::BufferedFile>(fp);
}


std::vector<Path>
PlatformFilesystem::glob(StringView pattern) const {
    std::vector<Path> pathsFound;

    glob_t globResults;
    auto const ret = ::glob(pattern.data(), 0, nullptr, &globResults);  // FIXME(abbyssoul): Unsafe usage of .data()
    auto const globResultsGuard = std::unique_ptr<glob_t, decltype(globfree)*>{&globResults, globfree};

    if (ret == 0) {  // All good - transfer matches into an array
        pathsFound.reserve(globResults.gl_pathc);

        for (size_t i = 0; i < globResults.gl_pathc; ++i) {
            pathsFound.emplace_back(Path::parse(globResults.gl_pathv[i]).unwrap());
        }
    } else {
        if (ret == GLOB_NOSPACE) {
            raise<Exception>("glob is out of space!");
        } else if (ret == GLOB_ABORTED) {
            raise<Exception>("glob aborted!");
        } else if (ret == GLOB_NOMATCH) {
            // Meh
        } else {
            raise<Exception>("Unexpected glob error code!");
        }
    }


    return pathsFound;
}


std::vector<Path>
PlatformFilesystem::glob(std::initializer_list<const char*> patterns) const {
    std::vector<Path> pathsFound;

    if (patterns.size() == 0) {
        return pathsFound;
    }

    glob_t globResults;
    auto iter = patterns.begin();

    ::glob(*iter, 0, nullptr, &globResults);         // FIXME(abbyssoul): Handle glob return values
    auto const globResultsGuard = std::unique_ptr<glob_t, decltype(globfree)*>{&globResults, globfree};

    for (++iter; iter != patterns.end(); ++iter) {
        // FIXME(abbyssoul): Handle glob return values
        ::glob(*iter, GLOB_APPEND, nullptr, &globResults);
    }

    pathsFound.reserve(globResults.gl_pathc);
    for (size_t i = 0; i < globResults.gl_pathc; ++i) {
        pathsFound.emplace_back(Path::parse(globResults.gl_pathv[i]).unwrap());
    }


    return pathsFound;
}


Path
PlatformFilesystem::getExecPath() const {
#ifdef SOLACE_PLATFORM_LINUX
    char execPath[1024];
    size_t buffSize = sizeof(execPath);
    ssize_t const bytesRead = readlink("/proc/self/exe", execPath, buffSize - 1);
    if (bytesRead < 0) {
        raise<IOException>(errno);
    } else {
        execPath[bytesRead] = 0;
    }

    return Path::parse(execPath).unwrap();

#elif defined(SOLACE_PLATFORM_APPLE)
    char execPath[1024];
    uint buffSize = sizeof(execPath);
    auto const success = (_NSGetExecutablePath(execPath, &buffSize) == 0);
    execPath[buffSize - 1] = '\0';
    if (!success) {
        raise<Exception>("Failed to read executable path");
    }

    return Path::parse(execPath).unwrap();
#elif defined(SOLACE_PLATFORM_BSD)
    char exePath[2048];
    size_t len = sizeof(exePath);
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
    if (sysctl(mib, 4, exePath, &len, NULL, 0) != 0) {
        exePath[0] = '\0';
    }
    return Path::parse(exePath).unwrap();

//    char temp[1024];

//    ::snprintf(temp, sizeof(temp),"/proc/%d/file", ::getpid());
//    std::unique_ptr<char, decltype(std::free)*> real_path{realpath(temp, nullptr), std::free};

//    if (!real_path) {
//        raise<Exception>("Failed to read executable path");
//    }

//    return Path::parse(real_path.get()).unwrap();
#else
    #warning Implementation of PlatformFilesystem::getExecPath not avaliable
    return Path::Root;
#endif
}


Path
PlatformFilesystem::getWorkingDirectory() const {
    char buf[1024];  // FIXME(abbyssoul): Shouldn't it be max_path or something?

    char* buffer = getcwd(buf, sizeof(buf));
    if (buffer == nullptr) {
        raise<IOException>(errno);
    }

    return Path::parse(buffer).unwrap();
}


void
PlatformFilesystem::setWorkingDirectory(const Path& path) {
    const auto& pathString = path.toString();
    if (0 != chdir(pathString.c_str())){
        raise<IOException>(errno);
    }
}
