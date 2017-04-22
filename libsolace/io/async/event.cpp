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
 * @file: io/async/event.cpp
 *
 *  Created by soultaker on 01/10/16.
*******************************************************************************/
#include <solace/io/async/event.hpp>
#include <solace/exception.hpp>


#include <sys/eventfd.h>
#include <unistd.h>


using namespace Solace;
using namespace Solace::IO;
using namespace Solace::IO::async;



class EventReadRequest :
        public EventLoop::Request {
public:

    EventReadRequest(ISelectable::poll_id fd, const Event* event) :
        Request(),
        _fd(fd),
        _isComplete(false),
        _event(event)
    {}

    void onReady(const Selector::Event& event) override {

        if (event.isSet(Solace::IO::Selector::Events::Read)) {
            eventfd_t eventValue;
            const auto result = eventfd_read(_fd, &eventValue);
            if (result < 0) {
                raise<IOException>(errno);
            } else {
                _isComplete = true;
                _promise.resolve();
            }
        }

        if (event.isSet(Solace::IO::Selector::Events::Write)) {
            _isComplete = true;
//            _promise.error();
        }

    }

    bool isComplete() const noexcept  {
        return _isComplete;
    }

     bool isAbout(const Selector::Event& e) const override {
        return (e.fd == _fd);
     }

     Result<void>& promise() noexcept {
         return _promise;
     }

private:
    ISelectable::poll_id    _fd;
    bool                    _isComplete;
    const Event*            _event;
    Result<void>            _promise;
};


Event::Event(Event&& rhs):
    Channel(std::move(rhs)),
    _fd(rhs._fd)
{
    rhs._fd = ISelectable::InvalidFd;
}


Event::Event(EventLoop& ioContext) :
    Channel(ioContext),
    _fd(eventfd(0, EFD_NONBLOCK))
{
    if (_fd < 0) {
        Solace::raise<IOException>(errno);
    }

    auto& selector = ioContext.getSelector();
    selector.add(_fd, Solace::IO::Selector::Events::Read, this);
}


Event::~Event() {
    if (_fd != ISelectable::InvalidFd) {
        auto& iocontext = getIOContext();
        auto& selector = iocontext.getSelector();

        selector.remove(_fd);

        // TODO(abbyssoul): do check return value
        close(_fd);

        _fd = ISelectable::InvalidFd;
    }
}


Result<void>& Event::asyncWait() {
    auto& iocontext = getIOContext();

    // FIXME(abbyssoul): WTF?! Don't register fd with selector for each read/write!
    auto request = std::make_shared<EventReadRequest>(_fd, this);

    // Promiss to call back once this request has been resolved
    iocontext.submit(request);

    return request->promise();
}


void Event::notify() {

    auto result = eventfd_write(_fd, 1);
    if (result < 0) {
        raise<IOException>(errno);
    }

}
