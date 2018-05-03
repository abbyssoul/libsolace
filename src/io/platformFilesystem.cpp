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
#include <cstring>  // memcpy (should review)
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <glob.h>


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
PlatformFilesystem::BufferedFile::read(MemoryView& buffer) {
    if (!_fp) {
        raise<NotOpen>();
    }

    const auto bytesRead = ::fread(buffer.dataAddress(), 1, buffer.size(), _fp);
    // NOTE: Number of bytes read can be less then 'bytesToRead' if there are no more data in the file or end of file
    // has been reached.

    return Ok(bytesRead);
}


IOObject::IOResult
PlatformFilesystem::BufferedFile::write(const ImmutableMemoryView& buffer) {
    if (!_fp) {
        raise<NotOpen>();
    }

    const auto bytesWritten = ::fwrite(buffer.dataAddress(), 1, buffer.size(), _fp);
    if (bytesWritten != buffer.size()) {
        raise<IOException>(errno);
    }

    return Ok(bytesWritten);
}


File::size_type PlatformFilesystem::BufferedFile::seek(size_type offset, File::Seek type) {
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


PlatformFilesystem::BufferedFile::size_type PlatformFilesystem::BufferedFile::tell() {
    if (!_fp) {
        raise<NotOpen>();
    }

    return ftell(_fp);
}





std::shared_ptr<File> PlatformFilesystem::create(const Path& path) {
    const auto& pathString = path.toString();
    auto fp = fopen(pathString.c_str(), "w+x");

    if (!fp) {
        raise<IOException>(errno);
    }

    return std::make_shared<PlatformFilesystem::BufferedFile>(fp);
}


std::shared_ptr<File> PlatformFilesystem::open(const Path& path) {
    const auto& pathString = path.toString();
    auto fp = fopen(pathString.c_str(), "r+");

    if (!fp) {
        raise<IOException>(errno);
    }

    return std::make_shared<PlatformFilesystem::BufferedFile>(fp);
}


bool PlatformFilesystem::unlink(const Path& path) {
    const auto& pathString = path.toString();
    auto err = ::remove(pathString.c_str());

    if (err) {
        raise<IOException>(errno);
    }

    return true;
}


bool PlatformFilesystem::exists(const Path& path) const {
    struct stat sb;

    const auto& pathString = path.toString();
    return (stat(pathString.c_str(), &sb ) == 0);
}


bool PlatformFilesystem::isFile(const Path& path) const {
    const auto& pathString = path.toString();

    struct stat sb;
    if (stat(pathString.c_str(), &sb )) {
        raise<IOException>(errno);
    }

    return S_ISREG(sb.st_mode);
}


bool PlatformFilesystem::isDirectory(const Path& path) const {
    const auto& pathString = path.toString();

    struct stat sb;
    if (stat(pathString.c_str(), &sb )) {
        raise<IOException>(errno);
    }

    return S_ISDIR(sb.st_mode);
}


time_t PlatformFilesystem::getTimestamp(const Path& path) const {
    const auto& pathString = path.toString();

    struct stat sb;
    if (stat(pathString.c_str(), &sb )) {
        raise<IOException>(errno);
    }

    return sb.st_mtime;
}


PlatformFilesystem::size_type PlatformFilesystem::getFileSize(const Path& path) const {
    struct stat sb;

    const auto& pathString = path.toString();
    if (stat(pathString.c_str(), &sb )) {
        raise<IOException>(errno);
    }

    return sb.st_size;
}


Path PlatformFilesystem::realPath(const Path& path) const {
    const auto& pathString = path.toString();
    std::unique_ptr<char, decltype(std::free)*> real_path{realpath(pathString.c_str(), nullptr), std::free};

    return (real_path)
            ? Path::parse(real_path.get()).unwrap()
            : Path::Root;
}


std::shared_ptr<PlatformFilesystem::BufferedFile> PlatformFilesystem::createTemp() {
    auto fp = tmpfile();

    if (!fp) {
        raise<IOException>(errno);
    }

    return std::make_shared<PlatformFilesystem::BufferedFile>(fp);
}


Array<Path> PlatformFilesystem::glob(const String& pattern) const {
    std::vector<Path> pathsFound;

    glob_t globResults;
    auto ret = ::glob(pattern.c_str(), 0, nullptr, &globResults);

    if (ret == 0) {  // All good - transfer matches into an array
        pathsFound.reserve(globResults.gl_pathc);

        for (size_t i = 0; i < globResults.gl_pathc; ++i) {
            pathsFound.push_back(Path::parse(globResults.gl_pathv[i]).unwrap());
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

    globfree(&globResults);

    return pathsFound;
}


Array<Path>
PlatformFilesystem::glob(std::initializer_list<const char*> patterns) const {
    std::vector<Path> pathsFound;

    if (patterns.size() == 0)
        return pathsFound;

    glob_t globResults;
    auto iter = patterns.begin();

    ::glob(*iter, 0, nullptr, &globResults);
    while (++iter != patterns.end()) {
        ::glob(*iter, GLOB_APPEND, nullptr, &globResults);
    }

    pathsFound.reserve(globResults.gl_pathc);
    for (size_t i = 0; i < globResults.gl_pathc; ++i) {
        pathsFound.push_back(Path::parse(globResults.gl_pathv[i]).unwrap());
    }

    globfree(&globResults);

    return pathsFound;
}


Path PlatformFilesystem::getExecPath() const {
    char execPath[1024];
    const size_t buffSize = sizeof(execPath);

    ssize_t bytesRead = readlink("/proc/self/exe", execPath, buffSize - 1);
    if (bytesRead == -1) {
        raise<IOException>(errno);
    } else {
        execPath[bytesRead] = 0;
    }

    return Path::parse(execPath).unwrap();
}


Path PlatformFilesystem::getWorkingDirectory() const {
    char buf[1024];  // FIXME(abbyssoul): Shouldn't it be max_path or something?

    char* buffer = getcwd(buf, sizeof(buf));
    if (!buffer) {
        raise<IOException>(errno);
    }

    return Path::parse(buffer).unwrap();
}


void PlatformFilesystem::setWorkingDirectory(const Path& path) {
    const auto& pathString = path.toString();
    if (0 != chdir(pathString.c_str())){
        raise<IOException>(errno);
    }
}
