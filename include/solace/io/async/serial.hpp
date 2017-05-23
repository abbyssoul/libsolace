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
 * libSolace: Channel for event loops
 *	@file		solace/io/async/channel.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IO_ASYNC_SERIAL_HPP
#define SOLACE_IO_ASYNC_SERIAL_HPP

#include "solace/byteBuffer.hpp"
#include "solace/io/serial.hpp"
#include "solace/io/selectable.hpp"
#include "solace/io/async/future.hpp"
#include "solace/io/async/channel.hpp"


namespace Solace { namespace IO { namespace async {

class SerialChannel :
        public Channel {
public:
    using Channel::size_type;

public:

    ~SerialChannel();

    SerialChannel(const SerialChannel& rhs) = delete;
    SerialChannel& operator= (const SerialChannel& rhs) = delete;

    SerialChannel(EventLoop& ioContext,
           const Path& file,
           uint32 baudrate = 9600,
           Serial::Bytesize bytesize = Serial::Bytesize::eightbits,
           Serial::Parity parity = Serial::Parity::none,
           Serial::Stopbits stopbits = Serial::Stopbits::one,
           Serial::Flowcontrol flowcontrol = Serial::Flowcontrol::none);

    SerialChannel(SerialChannel&& rhs):
        Channel(std::move(rhs)),
        _serial(std::move(rhs._serial))
    {}

    SerialChannel& operator= (SerialChannel&& rhs) noexcept {
        return swap(rhs);
    }

    SerialChannel& swap(SerialChannel& rhs) noexcept {
        using std::swap;

        Channel::swap(rhs);
        swap(_serial, rhs._serial);

        return *this;
    }


    /**
     * Post an async read request to read data from this IO object into the given buffer.
     * This method reads the data until the provided destination buffer is full.
     *
     * @param dest The provided destination buffer to read data into.
     * @return A future that will be resolved one the buffer has been filled.
     */
    Future<void>& asyncRead(ByteBuffer& dest) {
        return asyncRead(dest, dest.remaining());
    }

    /**
     * Post an async read request to read specified amount of data from this IO object into the given buffer.
     *
     * @param dest The provided destination buffer to read data into.
     * @param bytesToRead Amount of data (in bytes) to read from this IO object.
     * @return A future that will be resolved one the scpecified number of bytes has been read.
     *
     * @note If the provided destination buffer is too small to hold requested amount of data - an exception is raised.
     */
    Future<void>& asyncRead(ByteBuffer& dest, size_type bytesToRead);


    /**
     * Post an async write request to write specified amount of data into this IO object.
     * This method writes whole content of the provided buffer into the IO objec.
     *
     * @param src The provided source buffer to read data from.
     * @return A future that will be resolved one the scpecified number of bytes has been written into the IO object.
     */
    Future<void>& asyncWrite(ByteBuffer& src) {
        return asyncWrite(src, src.remaining());
    }

    /**
     * Post an async write request to write specified amount of data into this IO object.
     *
     * @param src The provided source buffer to read data from.
     * @param bytesToWrite Amount of data (in bytes) to write from the buffer into this IO object.
     * @return A future that will be resolved one the scpecified number of bytes has been written into the IO object.
     *
     * @note If the provided source buffer does not have requested amount of data - an exception is raised.
     */
    Future<void>& asyncWrite(ByteBuffer& src, size_type bytesToWrite);

private:

    Serial  _serial;
};


inline void swap(SerialChannel& lhs, SerialChannel& rhs) noexcept {
    lhs.swap(rhs);
}

}  // End of namespace async
}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_ASYNC_SERIAL_HPP
