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
 * libSolace: Unix POSIX shared memory object
 *	@file		solace/io/sharedMemory.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IO_SHAREDMEMORY_HPP
#define SOLACE_IO_SHAREDMEMORY_HPP


#include "solace/byteBuffer.hpp"
#include "solace/path.hpp"
#include "solace/io/selectable.hpp"
#include "solace/io/file.hpp"  // File::Mode


namespace Solace { namespace IO {

/** Shared memory region
 *
 */
class SharedMemory: public ISelectable
{
public:

    using ISelectable::poll_id;

    typedef MemoryView::size_type size_type;

    /** Desired protection of the mapping
     * It is either None for no protection
     * or the bitwise OR of one or more of the flags
     */
    struct Protection {
        static const int None;   //!< Pages may not be accessed.
        static const int Exec;   //!< Pages may be executed.
        static const int Read;   //!< Pages may be read.
        static const int Write;  //!< Pages may be written.
    };


    /** Sharing access mode
     * Determines whether updates to the mapping are visible to other processes mapping the same region,
     * and whether updates are carried through to the underlying file (if any)
     */
    enum class MappingAccess {

        /** Share this mapping.
         * Updates to the mapping are visible to other processes that map this file,
         * and are carried through to the underlying file.
         */
        Shared,

        /** Create a private copy-on-write mapping.
         * Updates to the mapping are not visible to other processes mapping the same file,
         * and are not carried through to the underlying file.
         */
        Private
    };

    class MappedMemoryView: public MemoryView {
    public:

        using MemoryView::size_type;

        MappedMemoryView(size_type newSize, void* dataAddress);
        MappedMemoryView(MappedMemoryView&& rhs): MemoryView(std::move(rhs))
        {
        }

        ~MappedMemoryView();

        MappedMemoryView& swap(MappedMemoryView& rhs) noexcept {
            MemoryView::swap(rhs);

            return (*this);
        }

        MappedMemoryView& operator= (MappedMemoryView&& rhs) noexcept {
            return swap(rhs);
        }
    };

public:

    /**
     * Wrap existing file descriptor into a file object
     *
     * @note The file descriptor is not dup'd so the passed fid will be closed when file object is destroyd.
     * @param fid File descriptor to wrap into a file object
     * @return File object wrapping given file descriptor
     */
    static SharedMemory fromFd(poll_id fid);

    /**
     * Create named shared memory segment of the given size
     *
     * @param pathname Name of the shared memory segment
     * @param size Size in byted of the shared memory sergment
     * @param mode Access mode to the shared memory segment. @see File::AccessMode
     * @param permissionsMode File access permissions. @see File::Mode

     * @return Shared memory segment.
     */
    static SharedMemory create(const Path& pathname, size_type size,
                               File::AccessMode mode = File::AccessMode::ReadWrite,
                               int permissionsMode = (File::Mode::IRUSR | File::Mode::IWUSR));

    /**
     * Open already existing shared memory segment that was created by another process.
     * @param pathname The name of the shared memory segment to attach.
     * @param mode Access mode to open shared memory in.
     *
     * @return Shared memory segment.
     */
    static SharedMemory open(const Path& pathname, File::AccessMode mode = File::AccessMode::ReadWrite);

    /**
     * Remove named shared memory segment.
     * The segment will be delited only after all processes that hold reference to it will exit.
     *
     * @param pathname Nam of the shared memory segment.
     */
    static void unlink(const Path& pathname);

public:

    /**
     * Move construct this file object
     * @param other A file object to move from
     */
    SharedMemory(SharedMemory&& other);

    /**
     * Default distructor.
     */
    virtual ~SharedMemory();


    /**
     * Swap content of this file with an other
     * @param rhs A file handle to swap with
     * @return Reference to this
     */
    SharedMemory& swap(SharedMemory& rhs) noexcept {
        std::swap(_fd, rhs._fd);

        return *this;
    }

    /**
     * Move assignment operator
     * @param rhs Other file to move data from
     * @return reference to this
     */
    SharedMemory& operator= (SharedMemory&& rhs) noexcept {
        return swap(rhs);
    }


    /**
     * Map opened/created memory segment
     *
     * @param mapping Sharing mode. @see SharedMemory::Access.
     * @param access Mapped page protection. @see SharedMemory::Protection.
     * @param mapSize The size of allocatied memory segment to map.
     *  Note this should be less then the size of allocated memory.
     *
     * @return Memory view of the mapped shared memory region.
     */
    MappedMemoryView map(MappingAccess mapping = MappingAccess::Private,
                   int access = Protection::Read | Protection::Write,
                   size_type mapSize = 0);


    /**
     * Get the size of the shared memory segment
     *
     * @return The size of the shared memory segment
     */
    size_type size() const;


    /** Test to see if file is still opened.
     */
    inline bool operator! () { return !isOpen(); }

    /** Test to see if file is still opened.
     *
     * @return True if file is open
     */
    virtual bool isOpen() const;

    /** Test to see if file is still opened.
     *
     * @return True if file has been closed
     */
    virtual bool isClosed() const;

    /** Get underlying OS file descriptor of the file object
     *
     * @note: Do not copy this FD as this file object close is called from descriptor
     **/
    poll_id getSelectId() const override {
        return _fd;
    }

    /** Close this file if it was opened.
     * A closed file cannot be read or written to any more.
     * Any operation which requires that the file be open will raise an error
     * after the file has been closed.
     * Calling close() more than once is NOT allowed.
     *
     * @see man(2) close for more details
     *
     * @throws IOException if close system call failed
     */
    virtual void close();

protected:

    /**
     * Create a file object using file id
     * This doesn't call 'open' on the file id provided
     * as it is assumed that file already has been opened
     *
     * @param fd - Id of the opened file.
     */
    SharedMemory(const poll_id fd) noexcept;


    /**
     * Validate that file descriptor was opened and return it if it valid
     * Throws NotOpened IOException if the file id was not opened pior to usage
     *
     * @return Valid file id
     * @throws NotOpened exception
     */
    virtual poll_id validateFd() const;

    /**
     * Set poll fd to an invalid value.
     * @return Old value of fileno.
     */
    poll_id invalidateFd();

private:

    poll_id     _fd;
};


}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_SHAREDMEMORY_HPP
