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
 * @file: io/async/timer.cpp
 *
 *  Created by soultaker on 27/04/17.
*******************************************************************************/
#include <solace/io/async/timer.hpp>
#include <solace/exception.hpp>


#include <sys/timerfd.h>
#include <unistd.h>


using namespace Solace;
using namespace Solace::IO;
using namespace Solace::IO::async;



class TimeEventReadRequest :
        public EventLoop::Request {
public:

    explicit TimeEventReadRequest(ISelectable::poll_id fd) :
        Request(),
        _fd(fd),
        _isComplete(false)
    {}

    void onReady(const Selector::Event& event) override {

        if (event.isSet(Solace::IO::Selector::Events::Read)) {
            int64_t expCount = 0;
            const auto result = read(_fd, &expCount, sizeof(expCount));
            if (result < 0) {
                raise<IOException>(errno);
            } else if (expCount > 0) {
                _isComplete = !isPeriodic();
                _promise.setValue(expCount);
            }
        }

        if (event.isSet(Solace::IO::Selector::Events::Write)) {
            _isComplete = true;
//            _promise.setError();
        }

    }

    bool isPeriodic() const {
        itimerspec newValue;

        if (timerfd_gettime(_fd, &newValue) != 0) {
            Solace::raise<IOException>(errno, "timerfd_gettime");
        }

        return ((newValue.it_interval.tv_sec != 0) || (newValue.it_interval.tv_nsec != 0));
    }

    bool isComplete() const noexcept override {
        return _isComplete;
    }

     bool isAbout(const Selector::Event& e) const override {
        return (e.fd == _fd);
     }

     Future<int64_t> promise() noexcept {
         return _promise.getFuture();
     }

private:
    ISelectable::poll_id    _fd;
    bool                    _isComplete;
    Promise<int64_t>         _promise;
};


Timer::Timer(Timer&& rhs):
    Channel(std::move(rhs)),
    _fd(rhs._fd)
{
    rhs._fd = ISelectable::InvalidFd;
}

Timer::Timer(EventLoop& ioContext) :
    Channel(ioContext),
    _fd(timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC))
{
    if (_fd < 0) {
        Solace::raise<IOException>(errno, "timerfd_create");
    }

    auto& selector = ioContext.getSelector();
    selector.add(_fd, Solace::IO::Selector::Events::Read, this);
}


Timer::Timer(EventLoop& ioContext, const time_type& d) : Timer(ioContext)
{
    setTimeout(d);
}


Timer::~Timer() {
    if (_fd != ISelectable::InvalidFd) {
        auto& iocontext = getIOContext();
        auto& selector = iocontext.getSelector();

        selector.remove(_fd);

        // TODO(abbyssoul): do check return value
        close(_fd);

        _fd = ISelectable::InvalidFd;
    }
}


Future<int64_t> Timer::asyncWait() {
    auto& iocontext = getIOContext();

    // FIXME(abbyssoul): WTF?! Don't register fd with selector for each read/write!
    auto request = std::make_shared<TimeEventReadRequest>(_fd);

    // Promiss to call back once this request has been resolved
    iocontext.submit(request);

    // FIXME(abbyssoul): Actually timer might have already expired by this time. Such promise should be resolved!
    return request->promise();
}


Timer& Timer::cancel() {
    return setTimeout(std::chrono::seconds(0));
}


Timer& Timer::setTimeout(const time_type& d) {
    itimerspec newValue;
    newValue.it_value.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(d).count();
    newValue.it_value.tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(d).count();
    newValue.it_interval.tv_sec = 0;
    newValue.it_interval.tv_nsec = 0;

    if (timerfd_settime(_fd, 0, &newValue, nullptr) != 0) {
        Solace::raise<IOException>(errno, "timerfd_settime");
    }

    return (*this);
}


Timer& Timer::setTimeoutInterval(const time_type& initialDelay, const time_type& period) {
    itimerspec newValue;
    newValue.it_value.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(initialDelay).count();
    newValue.it_value.tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(initialDelay).count();
    newValue.it_interval.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(period).count();
    newValue.it_interval.tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(period).count();

    if (timerfd_settime(_fd, 0, &newValue, nullptr) != 0) {
        Solace::raise<IOException>(errno, "timerfd_settime");
    }

    return (*this);
}


Timer::time_type Timer::getTimeout() {
    itimerspec newValue;

    if (timerfd_gettime(_fd, &newValue) != 0) {
        Solace::raise<IOException>(errno, "timerfd_gettime");
    }

    return std::chrono::duration_cast<time_type>(std::chrono::seconds(newValue.it_value.tv_sec) +
                std::chrono::nanoseconds(newValue.it_value.tv_nsec));
}
