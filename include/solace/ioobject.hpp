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
 * libSolace: MemoryView
 *	@file		solace/memoryView.hpp
 *	@author		$LastChangedBy: $
 *	@date		$LastChangedDate: $
 *	@brief		MemoryView object
 *	ID:			$Id: $
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IOOBJECT_HPP
#define SOLACE_IOOBJECT_HPP

#include "solace/types.hpp"
#include "solace/assert.hpp"
#include "solace/result.hpp"
#include "solace/error.hpp"


namespace Solace {


class MemoryView;
class ByteBuffer;


/**
 * Basic interface for classes doing I/O
 * All IO objects implementing this interface will be able to accept MemoryView and ByteBuffer as data sources / dest.
 */
class IOObject {
public:
    typedef size_t size_type;
    using IOResult = Result<size_type, Error>;

public:
    virtual ~IOObject() = default;


    /** Test if this io object is ready for IO operations.
     * @return True if object is ready for IO.
     */
    explicit operator bool() { return isOpened(); }


    /** Test if this io object is opened and ready for IO requests.
     *
     * @return True if the IO object is opened.
     */
    virtual bool isOpened() const = 0;


    /** Test if this io object has been closed.
     *
     * @return True if the IO object has been closed
     */
    virtual bool isClosed() const {
        return !isOpened();
    }


    /** Close this io object making further IO operations impossible.
     * A closed IO object should not be read or written to.
     * Any operation which requires that the io object be opened will raise an error
     * after the io object has been closed.
     *
     * @throws IOException if close system call failed
     * @see man(2) close for more details
     */
    virtual void close() = 0;


    /** Read data from the io object into the given memory location/buffer.
     * The read operation will attepmt to read as much data as fits into the buffer (using buffer.size()),
     * however @note that it is not an error if less data has been read.
     * Depending on the undelaying implementaiton it is possible to read 0 byte, for example if EOF was reached or
     * socket connection has been closed.
     *
     * @param destBuffer Destanation buffer to read data into
     * @return Result of number of bytes actually read if successful or an error.
     */
    virtual IOResult read(MemoryView& destBuffer) = 0;


    /** Read data from the io object into the given byte stream.
     * The read operation will attepmt to read as much data as fits into the buffer (using buffer.size()),
     * however @note that it is not an error if less data has been read.
     * @see IOObject::read(MemoryView& destBuffer) for more details.
     * @note destBuffer will be advanced on the number of bytes read if the read was successful.
     *
     * @param buffer Byte buffer to read data into.
     * @return Number of bytes actually read.
     */
    IOResult read(ByteBuffer& destBuffer);


    /** Read data from this io object into the given byte stream.
     * The read operation attempts to read as much data as availiable space in the buffer.
     * @see IOObject::read(MemoryView& destBuffer) for more details.
     *
     * @param destBuffer Byte buffer to read data into
     * @param bytesToRead Number of bytes to be read into the buffer
     *
     * @return Number of bytes actually read.
     */
    IOResult read(ByteBuffer& destBuffer, size_type bytesToRead);


    /** Write data from the given byte buffer into this io object.
     * The write operation attempts to write as much data as availiable the given buffer.
     * however @note that it is not an error if less data has been actually written.
     *
     * @param data Bytes to write into this file object.

     * @return Number of bytes actually writen
     */
    virtual IOResult write(const MemoryView& srcBuffer) = 0;


    /** Write data from the given byte buffer into this io object.
     * The write operation attempts to write as much data as availiable the given buffer.
     * however @note that it is not an error if less data has been actually written.
     * @see IOObject::write(const MemoryView& srcBuffer) for more details.
     * @note srcBuffer will be advanced on the number of bytes actually written if the write operation was successful.
     *
     * @param srcBuffer Bytes to write into this io object.

     * @return Number of bytes actually writen
     */
    IOResult write(ByteBuffer& srcBuffer);


    /** Write data from the given byte buffer into this io object.
     * The write operation attempts to write as much data as availiable the given buffer.
     * however @note that it is not an error if less data has been actually written.
     * @see IOObject::write(const MemoryView& srcBuffer) for more details.
     * @note srcBuffer will be advanced on the number of bytes actually written if the write operation was successful.
     *
     * @param srcBuffer Bytes to write into this io object.
     * @param bytesToWrite Number of bytes to write
     *
     * @return Number of bytes actually writen
     */
    IOResult write(ByteBuffer& srcBuffer, size_type bytesToWrite);

};

}  // End of namespace Solace
#endif  // SOLACE_IOOBJECT_HPP
