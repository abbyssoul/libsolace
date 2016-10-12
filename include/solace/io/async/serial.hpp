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
#include "solace/io/async/asyncResult.hpp"
#include "solace/io/async/channel.hpp"


namespace Solace { namespace IO { namespace async {

class SerialChannel :
        public Channel {
public:

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

    ~SerialChannel();

    // TODO(abbyssoul): implementation
    SerialChannel& operator = (SerialChannel&& rhs);

    Result<void>& asyncRead(Solace::ByteBuffer& buffer);
    Result<void>& asyncWrite(Solace::ByteBuffer& buffer);

private:

    Serial  _serial;
};

}  // End of namespace async
}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_ASYNC_SERIAL_HPP
