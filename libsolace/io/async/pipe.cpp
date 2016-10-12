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


#include <unistd.h>
#include <fcntl.h>



using namespace Solace;
using namespace Solace::IO;
using namespace Solace::IO::async;



class PipeRequest : public EventLoop::Request {
public:

    explicit PipeRequest(File& fd, Solace::ByteBuffer& buffer, Selector::Events direction) :
        Request(),
        _fd(fd),
        _buffer(buffer),
        _direction(direction),
        _isComplete(false)
    {}


    void onReady(const Selector::Event& event) override {

        if (event.isSet(_direction)) {

            if (_direction == Selector::Events::Read) {
                const auto r = _fd.read(_buffer);
                if (r > 0) {
                    _promise.resolve();
                    _isComplete = !_buffer.hasRemaining();
                }
            }

            if (_direction == Selector::Events::Write) {
                const auto r = _fd.write(_buffer);
                if (r > 0) {
                    _promise.resolve();
                    _isComplete = !_buffer.hasRemaining();
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

    Result<void>& promise() noexcept {
        return _promise;
    }

private:

    File&    _fd;
    ByteBuffer&     _buffer;
    Selector::Events _direction;

    bool                    _isComplete;
    Result<void>             _promise;
};


Pipe::Pipe(Pipe&& rhs):
    Channel(std::move(rhs)),
    _fds(std::move(rhs._fds))
{
}


Pipe::~Pipe() {
    auto& iocontext = getIOContext();
    auto& selector = iocontext.getSelector();

    for (auto& fd : _fds) {
        if (fd.isOpen()) {
            selector.remove(fd.getSelectId());
        }
    }
}



Pipe::Pipe(EventLoop& ioContext) :
    Channel(ioContext)
{
    int fds[2];
    const auto r = pipe2(fds, O_NONBLOCK);
    if (r < 0) {
        Solace::raise<IOException>(errno, "pipe2");
    }

    _fds[0] = File::fromFd(fds[0]);
    _fds[1] = File::fromFd(fds[1]);

    auto& selector = ioContext.getSelector();
    selector.add(fds[0], Selector::Events::Read, this);
    selector.add(fds[1], Selector::Events::Write, this);
}


Result<void>& Pipe::asyncRead(Solace::ByteBuffer& buffer) {
    auto& iocontext = getIOContext();

    auto request = std::make_shared<PipeRequest>(_fds[0], buffer, Selector::Events::Read);

    // Promiss to call back once this request has been resolved
    iocontext.submit(request);

    return request->promise();
}


Result<void>& Pipe::asyncWrite(Solace::ByteBuffer& buffer) {
    auto& iocontext = getIOContext();

    auto request = std::make_shared<PipeRequest>(_fds[1], buffer, Selector::Events::Write);

    // Promiss to call back once this request has been resolved
    iocontext.submit(request);

    return request->promise();
}
