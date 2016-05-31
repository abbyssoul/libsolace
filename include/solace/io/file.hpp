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
class File: public ISelectable {
public:

	enum class Seek {
		Set,
		Current,
		End
	};

	typedef ssize_t size_type;
	using ISelectable::poll_id;

public:

	/**
	 * Create a file object using file id
	 * This doesn't call 'open' on the file id provided 
	 * as it is assumed that file already has been opened
	 *
	 * @param fd - Id of the opened file.
	 */
	File(const poll_id fd) noexcept: _fd(fd) {
	}


	/**
	 * Open a file in the file system
	 * open sys called is called on to open the file identified by the give path
	 *
	 * @param path - Path to the file to open
	 */
	File(const Path& path, int flags = 0);


    File(File&& other);


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


    /** Read data from this file object into the given buffer.
     * The read attempts to read as much data as availiable space in the buffer.
     * @param buffer Byte buffer to read data into
     * @return Number of bytes actually read.
     * @see File::read()
     */
    size_type read(ByteBuffer& buffer);

    /** Read data from this file object into the given buffer.
     * The read attempts to read as much data as availiable space in the buffer.
     * @param buffer Byte buffer to read data into
     * @param bytesToRead Number of bytes to be read into the buffer
     *
     * @return Number of bytes actually read.
     * @see File::read()
     *
     * @throws IOException if close system call failed
     */
    virtual size_type read(ByteBuffer& buffer, ByteBuffer::size_type bytesToRead);


    /** Write data from the given byte buffer into this file object.
     *
     * @param data Bytes to write into this file object.

     * @return Number of bytes actually writen
     * @see File::write()
     *
     * @throws IOException if underlaying system call failed
     */
    size_type write(ByteBuffer& buffer);

    /** Write data from the given byte buffer into this file object.
     *
     * @param data Bytes to write into this file object.
     * @param bytesToWrite Number of bytes to write
     *
     * @return Number of bytes actually writtem
     * @see File::write()
     *
     * @throws IOException if underlaying system call failed
     */
    virtual size_type write(ByteBuffer& buffer, ByteBuffer::size_type bytesToWrite);


	/*
	 * Returns current 'position' in the file is applicable
     */
	// size_type tell() const;

	/*
	 * Attempt to move current position in the file stream
	*/
    virtual size_type seek(size_type offset, Seek type = Seek::Set);


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

protected:

    /**
     * Validate that file descriptor was opened and return it if it valid
     * Throws NotOpened IOException if the file id was not opened pior to usage
     *
     * @return Valid file id
     * @throws NotOpened exception
     */
    poll_id validateFd() const;

    /** Default constructor is to be called by derived classes only */
    File() = default;

private:

	poll_id _fd;
};


}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_FILE_HPP
