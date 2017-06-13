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
 * @file: io/async/signalSet.cpp
 *
 *  Created by soultaker on 10/10/16.
*******************************************************************************/
#include <solace/io/async/signalSet.hpp>
#include <solace/exception.hpp>


#include <sys/signalfd.h>
#include <signal.h>
#include <unistd.h>


using namespace Solace;
using namespace Solace::IO;
using namespace Solace::IO::async;



class SignalReadRequest :
        public EventLoop::Request {
public:

    explicit SignalReadRequest(ISelectable::poll_id fd) :
        Request(),
        _fd(fd),
        _isComplete(false)
    {}


    void onReady(const Selector::Event& event) override {

        if (event.isSet(Solace::IO::Selector::Events::Read)) {
            signalfd_siginfo fdsi;
            const auto r = read(_fd, &fdsi, sizeof(fdsi));
            if (r != sizeof(fdsi)) {
                if (errno != EAGAIN) {
                    raise<IOException>(errno, "read");
                }
            } else {
                _isComplete = true;
                _promise.setValue(fdsi.ssi_signo);
            }
        }

        if (event.isSet(Solace::IO::Selector::Events::Write) ||
            event.isSet(Solace::IO::Selector::Events::Error)) {
            _isComplete = true;
//            _promise.setError();
        }

    }

    bool isComplete() const noexcept override {
        return _isComplete;
    }

    bool isAbout(const Selector::Event& e) const override {
       return (e.fd == _fd);
    }

    Future<int> promise() noexcept {
        return _promise.getFuture();
    }

private:

    ISelectable::poll_id    _fd;
    bool                    _isComplete;

    Promise<int>             _promise;
};


SignalSet::SignalSet(SignalSet&& rhs):
    Channel(std::move(rhs)),
    _fd(rhs._fd)
{
    rhs._fd = ISelectable::InvalidFd;
}


SignalSet::~SignalSet() {
    if (_fd != ISelectable::InvalidFd) {
        auto& iocontext = getIOContext();
        auto& selector = iocontext.getSelector();

        selector.remove(_fd);

        // TODO(abbyssoul): do check return value
        close(_fd);

        _fd = ISelectable::InvalidFd;
    }
}



SignalSet::SignalSet(EventLoop& ioContext, std::initializer_list<int> sigs) :
    Channel(ioContext)
{
    sigset_t mask;

    sigemptyset(&mask);
    for (auto i : sigs) {
        sigaddset(&mask, i);
    }

    // Block signals so that they aren't handled according to their default dispositions
    if (sigprocmask(SIG_BLOCK, &mask, nullptr) == -1) {
        Solace::raise<IOException>(errno, "sigprocmask");
    }

    _fd = signalfd(-1, &mask, SFD_NONBLOCK);
    if (_fd < 0) {
        Solace::raise<IOException>(errno, "signalfd");
    }

    auto& selector = ioContext.getSelector();
    selector.add(_fd, Solace::IO::Selector::Events::Read, this);
}


Future<int> SignalSet::asyncWait() {
    auto& iocontext = getIOContext();

    auto request = std::make_shared<SignalReadRequest>(_fd);

    // Promiss to call back once this request has been resolved
    iocontext.submit(request);

    return request->promise();
}
