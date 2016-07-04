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
 * libSolace: Plarform filesystem inteface
 *	@file		solace/io/platformFilesystem.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IO_PLATFORMFILESYSTEM_HPP
#define SOLACE_IO_PLATFORMFILESYSTEM_HPP

#include "solace/io/filesystem.hpp"


namespace Solace { namespace IO {

/** Filesystem interface.
 * Filesystem is a hierarchical dictionary of objects.
 * One can think of it as a dictionary with string keys. Objects returned by filesyste are called 'file'
 * Note: virtual filesystem does not represents real platform filesystem but only defines a notion of
 * files and hierarchy on names. For plarform specific filesystem implementation @see PlatformFilesystem
 *
 * TODO(abbyssoul): Most of the methods use real fs and thus should return Result<>
 */
class PlatformFilesystem: public Filesystem {
public:

    //! Type used to represent file sizes
    typedef ByteBuffer::size_type size_type;


    class BufferedFile : public File {
    public:

        using File::read;
        using File::write;

        size_type read(MemoryView& buffer, MemoryView::size_type bytesToRead) override;
        size_type write(const MemoryView& buffer, MemoryView::size_type bytesToWrite) override;
        size_type seek(size_type offset, File::Seek type) override;
        void close() override;

        size_type tell();

    public:
        BufferedFile(FILE* fp);
        ~BufferedFile() = default;

        BufferedFile(BufferedFile&& that);

        BufferedFile& operator= (BufferedFile&& rhs) noexcept {
            return swap(rhs);
        }

        BufferedFile& swap(BufferedFile& rhs) noexcept {
            std::swap(_fp, rhs._fp);
            File::swap(rhs);

            return *this;
        }

    private:
        FILE* _fp;
    };

public:

    /**
     * PlatformFilesystem default constructor. Nothing to see here.
     */
    PlatformFilesystem();

    ~PlatformFilesystem() = default;

    /**
     * Create a new file
     * @param path a path to the file to be created
     * @return A newly created file
     *
     * TODO(abbyssoul): should return Result<>
     */
    std::shared_ptr<File> create(const Path& path) override;

    /**
     * Open already existing file
     * @param path A path to the file to be opened
     * @return Opened file
     *
     * TODO(abbyssoul): should return Result<>
     */
    std::shared_ptr<File> open(const Path& path) override;

    /**
     * Remove file by path
     * @param path A path to the file to be removed
     * @return True if file was removed
     *
     * TODO(abbyssoul): should return Result<>
     */
    bool unlink(const Path& path) override;

    /**
     * Check if a file identified by the path actually exists
     * @param path A path to the file to be checked
     * @return True if file exists
     *
     */
    bool exists(const Path& path) const override;

public:

    /**
     * Check if the path points to a regular file
     *
     * @param path A path to check
     * @return True if the path points to a regular file
     */
    bool isFile(const Path& path) const;

    /**
     * Check if the path points to a directory
     * @param path A path to check
     * @return True if the path points to a directory
     */
    bool isDirectory(const Path& path) const;

    /**
     * Get timestamp of the file pointed by the path
     *
     * @param path A path to the file to get timestamp of
     * @return Timestamp of the file
     */
    timespec getTimestamp(const Path& path) const;

    /**
     * Get the size of the file pointed to by the path
     * @param path A path to the file to get the size of
     * @return Size of the file
     */
    size_type getFileSize(const Path& path) const;

    /**
     * Expand all symbolic links and resolves references to /./, /../ and extra '/' characters in the path
     * to produce a canonicalized absolute pathname.
     *
     * @param path A path to be resolved
     * @return A canonicalized absolute pathname pointed to by the path object passed in.
     */
    Path realPath(const Path& path) const;

    /** Create a temporary file
     * Creates a temporary binary file for update mode.
     * The file is deleted automatically when it is closed or when the program terminates.
     *
     * @return An opened temporary file
     */
    std::shared_ptr<BufferedFile> createTemp();

    /** Find pathnames matching a pattern
     *
     * @param patter A string pattern for filenames to match
     * @return An array of filenames matching pattern
     */
    Array<Path> glob(const String& pattern) const;

    /** Find pathnames matching a pattern
     *
     * @param patter A string pattern for filenames to match
     * @return An array of filenames matching pattern
     */
    Array<Path> glob(std::initializer_list<const char*> patterns) const;

    /**
     * Get the path to the current executable
     * @return The path to the executable
     */
    Path getExecPath() const;

    /**
     * Get current working directory of the calling procces.
     * @return Current working directory
     */
    Path getWorkingDirectory() const;

    /**
     * Set current working directory of the calling procces.
     */
    void setWorkingDirectory(const Path& value);

};

}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_PLATFORMFILESYSTEM_HPP
