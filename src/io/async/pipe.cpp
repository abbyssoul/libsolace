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
 * @file: io/async/pipe.cpp
 *
 *  Created by soultaker on 10/10/16.
*******************************************************************************/
#include <solace/io/async/pipe.hpp>
#include <solace/exception.hpp>


#include <unistd.h>
#include <fcntl.h>



using namespace Solace;
using namespace Solace::IO;
using namespace Solace::IO::async;



class PipeRequest : public EventLoop::Request {
public:

    explicit PipeRequest(File& fd, Solace::ByteBuffer& buffer, Pipe::size_type size, Selector::Events direction) :
        Request(),
        _fd(fd),
        _buffer(buffer),
        _size(size),
        _bytesPassed(0),
        _direction(direction),
        _isComplete(false)
    {}


    void onReady(const Selector::Event& event) override {

        if (event.isSet(_direction)) {

            if (_direction == Selector::Events::Read) {
                const auto r = _fd.read(_buffer, _size - _bytesPassed);
                if (r) {
                    _bytesPassed += r.unwrap();
                    _isComplete = (_size >= _bytesPassed) || (r.unwrap() == 0);

                    if (_isComplete) {
                        _promise.setValue();
                    }
                }
            }

            if (_direction == Selector::Events::Write) {
                const auto r = _fd.write(_buffer, _size - _bytesPassed);
                if (r) {
                    _bytesPassed += r.unwrap();
                    _isComplete = (_size >= _bytesPassed) || (r.unwrap() == 0);

                    if (_isComplete) {
                        _promise.setValue();
                    }
                }
            }
        }

        if (event.isSet(Selector::Events::Error)) {
            _isComplete = true;
//            _promiss.error();
        }

    }

    bool isComplete() const noexcept override {
        return _isComplete;
    }

    bool isAbout(const Selector::Event& e) const override {
       return (e.fd == _fd.getSelectId());
    }

    Future<void> promise() noexcept {
        return _promise.getFuture();
    }

private:
    Promise<void>       _promise;

    File&               _fd;
    ByteBuffer&         _buffer;
    Pipe::size_type     _size;
    Pipe::size_type     _bytesPassed;
    Selector::Events    _direction;

    bool                _isComplete;
};


Duplex createNonblockingPipe() {
    int fds[2];
    const auto r = pipe2(fds, O_NONBLOCK);
    if (r < 0) {
        Solace::raise<IOException>(errno, "pipe2");
    }

    return Duplex(fds[0], fds[1]);
}


Pipe::~Pipe() {
    auto& iocontext = getIOContext();
    auto& selector = iocontext.getSelector();

    selector.remove(_duplex.getReadEnd().getSelectId());
    selector.remove(_duplex.getWriteEnd().getSelectId());
}


Pipe::Pipe(Pipe&& rhs):
    Channel(std::move(rhs)),
    _duplex(std::move(rhs._duplex))
{
}


Pipe::Pipe(EventLoop& ioContext) :
    Channel(ioContext),
    _duplex(createNonblockingPipe())
{
    auto& selector = ioContext.getSelector();
    selector.add(_duplex.getReadEnd().getSelectId(), Selector::Events::Read, this);
    selector.add(_duplex.getWriteEnd().getSelectId(), Selector::Events::Write, this);
}


Future<void>
Pipe::asyncRead(ByteBuffer& dest, size_type bytesToRead) {
    auto& iocontext = getIOContext();

    auto request = std::make_shared<PipeRequest>(_duplex.getReadEnd(), dest, bytesToRead, Selector::Events::Read);

    // Promiss to call back once this request has been resolved
    iocontext.submit(request);

    return request->promise();
}


Future<void>
Pipe::asyncWrite(ByteBuffer& src, size_type bytesToWrite) {
    auto& iocontext = getIOContext();

    auto request = std::make_shared<PipeRequest>(_duplex.getWriteEnd(), src, bytesToWrite, Selector::Events::Write);

    // Promiss to call back once this request has been resolved
    iocontext.submit(request);

    return request->promise();
}
