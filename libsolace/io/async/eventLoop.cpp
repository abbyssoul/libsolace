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
 * @file: io/async/eventLoop.cpp
 *
 *  Created by soultaker on 01/10/16.
*******************************************************************************/
#include <solace/io/async/eventloop.hpp>
#include <solace/exception.hpp>


#include <algorithm>
#include <chrono>


#include <sys/eventfd.h>
#include <unistd.h>


using namespace Solace;
using namespace Solace::IO;
using namespace Solace::IO::async;


EventLoop::EventLoop(uint32 backlogCapacity) :
    EventLoop(backlogCapacity, Selector::createEPoll(backlogCapacity))
{
}


EventLoop::EventLoop(uint32 backlogCapacity, Selector&& selector) :
    _keepOnRunning(true),
    _interruptFd { eventfd(0, EFD_NONBLOCK) },
    _backlog(),
    _selector(std::move(selector))
{
    if (_interruptFd < 0) {
        Solace::raise<IOException>(errno);
    }
    _selector.add(_interruptFd, Solace::IO::Selector::Events::Read, this);

    _backlog.reserve(backlogCapacity);
}


EventLoop::EventLoop(EventLoop&& rhs) noexcept :
    _keepOnRunning(rhs._keepOnRunning),
    _interruptFd(rhs._interruptFd),
    _backlog(std::move(rhs._backlog)),
    _selector(std::move(rhs._selector))
{
    rhs._keepOnRunning = false;
    rhs._interruptFd = -1;
}


EventLoop::~EventLoop() {
    if (_interruptFd != -1) {
        _selector.remove(_interruptFd);

        // TODO(abbyssoul): do check return value
        close(_interruptFd);

        _interruptFd = -1;
    }
}


EventLoop& EventLoop::swap(EventLoop& rhs) noexcept {
    using std::swap;

    swap(_keepOnRunning, rhs._keepOnRunning);
    swap(_interruptFd, rhs._interruptFd);
    swap(_backlog, rhs._backlog);
    swap(_selector, rhs._selector);

    return (*this);
}


void EventLoop::stop() {
    if (_keepOnRunning) {
        _keepOnRunning = false;

        // Write interaption into interapt event
        const auto result = eventfd_write(_interruptFd, 1);
        if (result < 0) {
            raise<IOException>(errno);
        }
    }
}


void EventLoop::submit(const std::shared_ptr<Request>& request) {
    _backlog.push_back(request);

    return;
}


void EventLoop::dispatchEvents(const Selector::Iterator& readyEvents) {
    for (auto event : readyEvents) {
        // Make sure that event.data is a request and not something else that was manually added into selector
        auto p = std::find_if(_backlog.begin(), _backlog.end(), [&event](auto i) { return i->isAbout(event); });

        if (p != _backlog.end()) {
            auto request = *p;
            request->onReady(event);
        }
    }

    _backlog.erase(std::remove_if(_backlog.begin(), _backlog.end(), [](auto r) { return r->isComplete(); }),
                   _backlog.end());
}


bool EventLoop::poll() {
    if (_keepOnRunning && !_backlog.empty()) {
        dispatchEvents(_selector.poll(0));
    }

    return _keepOnRunning;
}

void EventLoop::run() {
    while (_keepOnRunning && !_backlog.empty()) {
        dispatchEvents(_selector.poll());
    }
}

void EventLoop::runFor(int msec) {
    auto start = std::chrono::high_resolution_clock::now();

    while (_keepOnRunning && !_backlog.empty()) {
        const auto end = std::chrono::high_resolution_clock::now();
        const auto timeLeft = msec - std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        if (timeLeft < 0) {
            break;
        }

        const auto readyEvents = _selector.poll(timeLeft);
        if (readyEvents.size() == 0) {
            return;  // Timeout
        }

        dispatchEvents(readyEvents);
    }
}
