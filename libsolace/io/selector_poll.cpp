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
 * @file: io/selector_poll.cpp
 *
 *  Created by soultaker on 27/04/16.
*******************************************************************************/
#include <solace/io/selector.hpp>
#include <solace/io/ioexception.hpp>

#include "selector_impl.hpp"


#include <vector>
#include <algorithm>

#include <poll.h>   // poll()
#include <fcntl.h>
#include <unistd.h>  // close()


using namespace Solace;
using namespace Solace::IO;




class PollSelectorImpl :
        public Solace::IO::Selector::IPollerImpl {
public:

    // FIXME: evlist will actually leak if we throw here...
    explicit PollSelectorImpl(uint maxPollables) {
        _selectables.reserve(maxPollables);
        _pollfds.reserve(maxPollables);
    }

    ~PollSelectorImpl() {
    }

    void add(ISelectable* selectable, int events) override {
        add(selectable->getSelectId(), events, selectable);
    }

    void add(ISelectable::poll_id fd, int events, void* data) override {
        int pollEvents = 0;

        if (events & Selector::Events::Read)
            pollEvents |= POLLIN | POLLPRI;
        if (events & Selector::Events::Write)
            pollEvents |= POLLOUT;
        if (events & Selector::Events::Hup)
            pollEvents |= POLLRDHUP;

        addRaw(fd, pollEvents, data);
    }


    void addRaw(ISelectable::poll_id fd, int nativeEvents, void* data) override {
        pollfd pollEvent = {
            fd,
            static_cast<int16>(nativeEvents),
            0
        };

        Selector::Event ev;
        ev.data = data;
        ev.fd = fd;

        _selectables.push_back(ev);
        _pollfds.push_back(pollEvent);
    }


    void remove(const ISelectable* selectable) override {
        remove(selectable->getSelectId());
    }


    void remove(ISelectable::poll_id fd) override {
        // FIXME(abbyssoul): Make sure thouse are in order!!!

        _selectables.erase(
                    std::remove_if(_selectables.begin(), _selectables.end(), [fd](auto x) { return x.fd == fd; }),
                    _selectables.end());
        _pollfds.erase(
                    std::remove_if(_pollfds.begin(), _pollfds.end(), [fd](auto x) { return x.fd == fd; }),
                    _pollfds.end());
    }


    std::tuple<uint, uint> poll(int msec) override {
        const auto r = ::poll(_pollfds.data(), _pollfds.size(), msec);
        if (r < 0) {
            Solace::raise<IOException>(errno);
        } else if (r == 0) {
            return std::make_tuple(0, 0);
        }

        const auto pollCount = _pollfds.size();
        return std::make_tuple(findFirstReady(0, pollCount), pollCount);
    }


    Selector::Event getEvent(uint i) override {
        const auto& ev = _pollfds[i];
        const auto& selected = _selectables[i];

        Selector::Event event;
        event.data = selected.data;
        event.fd = selected.fd;
        event.events = 0;

        if ((ev.revents & POLLIN) || (ev.revents & POLLPRI))
            event.events |= Selector::Events::Read;
        if (ev.revents & POLLOUT)
            event.events |= Selector::Events::Write;
        if (ev.revents & POLLERR)
            event.events |= Selector::Events::Error;
        if (ev.revents & POLLRDHUP)
            event.events |= Selector::Events::Hup;

        return event;
    }


    uint advance(uint offsetIndex) override {
        const auto pollCount = _pollfds.size();

        // Overflow check
        if (offsetIndex >= _pollfds.size()) {
            return pollCount;
        }


        return findFirstReady(offsetIndex + 1, pollCount);
    }

protected:

    uint findFirstReady(uint offsetIndex, uint pollCount) {
        for (uint i = offsetIndex; i < pollCount; ++i) {
            const auto& p = _pollfds[i];
            if (p.revents) {
                return i;
            }
        }

        return pollCount;
    }

private:
    PollSelectorImpl(const PollSelectorImpl&) = delete;
    PollSelectorImpl& operator= (const PollSelectorImpl&) = delete;

    // This two are tightly coupled
    std::vector<Selector::Event>    _selectables;
    std::vector<pollfd>             _pollfds;
};


Selector Selector::createPoll(uint eventSize) {
    auto pimpl = std::make_shared<PollSelectorImpl>(eventSize);

    return Selector(std::move(pimpl));
}
