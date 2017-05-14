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

    SerialReadRequest(Serial& selectable, ByteBuffer& buffer) :
        Request(),
        _selectable(selectable),
        _buffer(buffer)
    {}

    void onReady(const Selector::Event& event) override {

        if (event.isSet(Selector::Events::Read)) {
            const auto r = _selectable.read(_buffer);
            if (r) {
                _promise.resolve();
            }
        }

        if (event.isSet(Selector::Events::Write)) {
            const auto r = _selectable.write(_buffer);
            if (r) {
                _promise.resolve();
            }
        }

        if (event.isSet(Selector::Events::Error)) {
            // TODO(abbyssoul): _promise.error();
        }
    }

    bool isAbout(const Selector::Event& e) const override {
       return (e.data == &_selectable);
    }

    async::Future<void>& promise() noexcept {
        return _promise;
    }

private:
    SerialReadRequest(const SerialReadRequest&) = delete;
    SerialReadRequest(SerialReadRequest&&) = delete;
    SerialReadRequest& operator =(const SerialReadRequest&) = delete;
    SerialReadRequest& operator =(SerialReadRequest&&) = delete;

    Serial&         _selectable;
    ByteBuffer&     _buffer;

    async::Future<void>    _promise;
};


async::Future<void>& SerialChannel::asyncRead(Solace::ByteBuffer& buffer) {
    auto& iocontext = getIOContext();

    auto request = std::make_shared<SerialReadRequest>(_serial, buffer);

    // Promiss to call back once this request has been resolved
    iocontext.submit(request);

    return request->promise();
}



SerialChannel::SerialChannel(EventLoop& ioContext,
       const Path& file,
       uint32 baudrate,
       Serial::Bytesize bytesize,
       Serial::Parity parity,
       Serial::Stopbits stopbits,
       Serial::Flowcontrol flowcontrol) :
    Channel(ioContext),
    _serial(file, baudrate, bytesize, parity, stopbits, flowcontrol)
{
    auto& selector = ioContext.getSelector();
    selector.add(&_serial,
                 Solace::IO::Selector::Events::Read);
}

SerialChannel::~SerialChannel() {
    auto& iocontext = getIOContext();
    auto& selector = iocontext.getSelector();

    selector.remove(_serial.getSelectId());
}
