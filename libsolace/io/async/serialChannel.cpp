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
 * @file: io/async/serialChannel.cpp
 *
 *  Created by soultaker on 01/10/16.
*******************************************************************************/
#include <solace/io/async/serial.hpp>

using namespace Solace;
using namespace Solace::IO;
using namespace Solace::IO::async;


Solace::IO::async::Result&
SerialChannel::asyncRead(Solace::ByteBuffer& buffer) {
    auto& iocontext = getIOContext();
    auto& selector = iocontext.getSelector();

    auto request = std::make_shared<EventLoop::Request>(&_serial, buffer);
    selector.add(request.get(), Solace::IO::Selector::Events::Read);

    // Promiss to call back once this request has been resolved
    return iocontext.submit(request); //request.promise();
}

