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

    SerialReadRequest(Serial& selectable, ByteBuffer& buffer, SerialChannel::size_type size, Selector::Events dir) :
        Request(),
        _selectable(selectable),
        _buffer(buffer),
        _size(size),
        _bytesPassed(0),
        _direction(dir),
        _isComplete(false)
    {}

    void onReady(const Selector::Event& event) override {

        if (event.isSet(_direction)) {
            if (_direction == Selector::Events::Read) {
                const auto r = _selectable.read(_buffer, _size - _bytesPassed);
                if (r) {
                    _bytesPassed += r.unwrap();
                    _isComplete = (_size >= _bytesPassed) || (r.unwrap() == 0);

                    if (_isComplete) {
                        _promise.setValue(_bytesPassed);
                    }
                }
            }

            if (_direction == Selector::Events::Write) {
                const auto r = _selectable.write(_buffer, _size - _bytesPassed);
                if (r) {
                    _bytesPassed += r.unwrap();
                    _isComplete = (_size >= _bytesPassed) || (r.unwrap() == 0);

                    if (_isComplete) {
                        _promise.setValue(_bytesPassed);
                    }
                }
            }
        }

        if (event.isSet(Selector::Events::Error)) {
            _isComplete = true;
            // TODO(abbyssoul): _promise.error();
        }
    }

    bool isComplete() const noexcept override {
        return _isComplete;
    }

    bool isAbout(const Selector::Event& e) const override {
       return (e.data == &_selectable);
    }

    Future<int> promise() noexcept {
        return _promise.getFuture();
    }

private:
    SerialReadRequest(const SerialReadRequest&) = delete;
    SerialReadRequest(SerialReadRequest&&) = delete;
    SerialReadRequest& operator =(const SerialReadRequest&) = delete;
    SerialReadRequest& operator =(SerialReadRequest&&) = delete;

private:

    Promise<int>    _promise;
    Serial&         _selectable;
    ByteBuffer&     _buffer;
    SerialChannel::size_type     _size;
    SerialChannel::size_type     _bytesPassed;
    Selector::Events    _direction;
    bool                    _isComplete;

};




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



Future<int>
SerialChannel::asyncRead(Solace::ByteBuffer& buffer, size_type bytesToRead) {
    auto& iocontext = getIOContext();

    auto request = std::make_shared<SerialReadRequest>(_serial, buffer, bytesToRead, Selector::Events::Read);

    // Promiss to call back once this request has been resolved
    iocontext.submit(request);

    return request->promise();
}

Future<int>
SerialChannel::asyncWrite(Solace::ByteBuffer& buffer, size_type bytesToWrite) {
    auto& iocontext = getIOContext();

    auto request = std::make_shared<SerialReadRequest>(_serial, buffer, bytesToWrite, Selector::Events::Write);

    // Promiss to call back once this request has been resolved
    iocontext.submit(request);

    return request->promise();
}
