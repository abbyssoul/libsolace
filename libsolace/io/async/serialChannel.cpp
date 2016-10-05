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


class SerialReadRequest :
        public EventLoop::Request {
public:

    SerialReadRequest(Serial* selectable, ByteBuffer& buffer) :
        Request(),
        _selectable(selectable),
        _buffer(buffer)
    {}

    void onRead() override {
//        static_cast<Serial*>(_selectable)->read(_buffer);
        _selectable->read(_buffer);

        promiss().resolve();
    }

    void onWrite() override {
//        static_cast<Serial*>(_selectable)->write(_buffer);
        _selectable->write(_buffer);

        promiss().resolve();
    }

//    virtual ISelectable* getSelectable() {
//        return _selectable;
//    }

    poll_id getSelectId() const override {
        return _selectable->getSelectId();
    }

private:
    Serial*     _selectable;
    ByteBuffer& _buffer;
};


Solace::IO::async::Result&
SerialChannel::asyncRead(Solace::ByteBuffer& buffer) {
    auto& iocontext = getIOContext();
    auto& selector = iocontext.getSelector();

    auto request = std::make_shared<SerialReadRequest>(&_serial, buffer);
    selector.add(request.get(), Solace::IO::Selector::Events::Read);

    // Promiss to call back once this request has been resolved
    return iocontext.submit(request);
}

