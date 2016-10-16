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
 * libSolace: Unix File handle wrapper
 *	@file		solace/io/file.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IO_FILE_HPP
#define SOLACE_IO_FILE_HPP

#include "solace/ioobject.hpp"
#include "solace/byteBuffer.hpp"
#include "solace/path.hpp"


#include "solace/io/selectable.hpp"
#include "solace/io/ioexception.hpp"


namespace Solace { namespace IO {

/**
 * Abstact data source aka File.
 * "Everything is a file" @ Unix
 *
 * File can be identified by it's file descriptor.
 * This means that the file object is actually opened
 *
 * @note: not all files are real files stored on a physical disk,
 * (Think of a socket or a pipe)
 * 	not all files can be seek or rewinded.
 */
class File :
        public IOObject,
        public ISelectable {
public:

    /**
     * File access modes
     * These request opening the file read-only, write-only, or read/write, respectively.
     */
    enum class AccessMode {
        ReadOnly,   //!< Open a file in read-only mode.
        WriteOnly,  //!< Open a file in write-only mode.
        ReadWrite   //!< Open a file in read/write mode.
    };


    /** File creation mode
     *
     * Specifies the file mode bits for a newly created file
     */
    struct Mode {
        static const int IRWXU;  //!< 00700 user (file owner) has read, write, and execute permission
        static const int IRUSR;  //!< 00400 user has read permission
        static const int IWUSR;  //!< 00200 user has write permission
        static const int IXUSR;  //!< 00100 user has execute permission

        static const int IRWXG;  //!< 00070 group has read, write, and execute permission
        static const int IRGRP;  //!< 00040 group has read permission
        static const int IWGRP;  //!< 00020 group has write permission
        static const int IXGRP;  //!< 00010 group has execute permission

        static const int IRWXO;  //!< 00007 others have read, write, and execute permission
        static const int IROTH;  //!< 00004 others have read permission
        static const int IWOTH;  //!< 00002 others have write permission
        static const int IXOTH;  //!< 00001 others have execute permission
    };

    struct Flags {
        static const int Append;
        static const int Async;
        static const int CloseExec;
        static const int Direct;
        static const int Directory;
        static const int DSync;
        static const int Exlusive;
        static const int NoCTTY;
        static const int NonBlock;
        static const int Sync;
        static const int Trunc;
    };

	enum class Seek {
		Set,
		Current,
		End
	};


	typedef ssize_t size_type;
	using ISelectable::poll_id;

public:

//    using IOObject::operator !;
    using IOObject::read;
    using IOObject::write;


public:

    /**
     * Wrap existing file descriptor into a file object
     * @note The file descriptor is not dup'd so the passed fid will be closed when file object is destroyd.
     * @param fid File descriptor to wrap into a file object
     * @return File object wrapping given file descriptor
     */
    static File fromFd(poll_id fid);

public:

    /**
     * Create a file handle object.
     */
    File();

    /**
     * Move construct this file object
     * @param other A file object to move from
     */
    File(File&& other);

    /**
     * Default virtual distructor of file types.
     */
	virtual ~File();


    /**
     * Swap content of this file with an other
     * @param rhs A file handle to swap with
     * @return Reference to this
     */
    File& swap(File& rhs) noexcept {
        std::swap(_fd, rhs._fd);

        return *this;
    }

    /**
     * Move assignment operator
     * @param rhs Other file to move data from
     * @return reference to this
     */
    File& operator= (File&& rhs) noexcept {
        return swap(rhs);
    }


	/** Test to see if file is still opened.
	 * 
	 * @return True if file is open
	 */
    bool isOpened() const override;


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
    void close() override;


    /** Read data from this file object into the given buffer.
     * The read attempts to read as much data as availiable space in the buffer.
     *
     * @param buffer Byte buffer to read data into
     * @return Number of bytes actually read.
     * @see File::read()
     */
    IOResult read(MemoryView& buffer) override;


    /** Write data from the given byte buffer into this file object.
     *
     * @param data Bytes to write into this file object.

     * @return Number of bytes actually writen
     * @see File::write()
     *
     * @throws IOException if underlaying system call failed
     */
    IOResult write(const MemoryView& buffer) override;


	/*
	 * Attempt to move current position in the file stream
	*/
    virtual size_type seek(size_type offset, Seek type = Seek::Set);


    /** Flush file buffer.
	 * If the file was open for writing and
	 * the last i/o operation was an output operation,
	 * any unwritten data in the output buffer is written to the file.
	 * If it was open for reading and the last operation was an input operation,
	 * the behaviour depends on the specific library implementation.
	 * (In some implementations this causes the input buffer to be cleared,
	 * but this is not standard behaviour).
     *
     * @see man(2) fsync for more details
     *
     * @throws IOException if close system call failed
     */
    virtual void flush();

    /** Get underlying OS file descriptor of the file object
     *
     * @note: Do not copy this FD as this file object close is called from descriptor
     **/
    poll_id getSelectId() const override {
        return _fd;
    }

protected:

    /**
     * Create a file object using file id
     * This doesn't call 'open' on the file id provided
     * as it is assumed that file already has been opened
     *
     * @param fd - Id of the opened file.
     */
    File(const poll_id fd) noexcept;


    /**
     * Open a file in the file system
     * open sys called is called on to open the file identified by the give path
     *
     * @param path - Path to the file to open
     */
    File(const Path& path, int flags = 0);


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

	poll_id _fd;
};


}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_FILE_HPP
