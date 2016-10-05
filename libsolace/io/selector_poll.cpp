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
        add(selectable->getSelectId(), selectable, events);
    }

    void add(ISelectable::poll_id fd, ISelectable* selectable, int events) override {
        int pollEvents = 0;

        if (events & Selector::Events::Read)
            pollEvents |= POLLIN | POLLPRI;
        if (events & Selector::Events::Write)
            pollEvents |= POLLOUT;
        if (events & Selector::Events::Hup)
            pollEvents |= POLLRDHUP;

        addRaw(fd, pollEvents, selectable);
    }


    void addRaw(ISelectable::poll_id fd, int nativeEvents, void* data) override {
        pollfd ev = {
            fd,
            static_cast<short int>(nativeEvents),
            0
        };

        _selectables.push_back(data);
        _pollfds.push_back(ev);
    }


    void remove(const ISelectable* selectable) override {
        // Find the index if this selectable in _selectables
        auto it = std::find(_selectables.begin(), _selectables.end(), selectable);

        if (it == _selectables.end()) {
            return;
        } else {
            const auto index = std::distance(_selectables.begin(), it);
            _selectables.erase(it);
            _pollfds.erase(_pollfds.begin() + index);
        }
    }


    void remove(ISelectable::poll_id fd) override {
        // Find the index if this selectable in _selectables
        // TODO:!!
    }


    std::tuple<uint, uint> poll(uint32 msec) override {
        const auto r = ::poll(_pollfds.data(), _pollfds.size(), msec);
        if (r < 0) {
            Solace::raise<IOException>(errno);
        }

        return std::make_tuple(advance(0), _pollfds.size());
    }


    Selector::Event getEvent(uint i) override {
        const auto& ev = _pollfds[i];

        Selector::Event event;
        event.data = _selectables[i];
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


    int advance(uint offsetIndex) {
        const auto pollCount = _pollfds.size();

        // Overflow check
        if (offsetIndex >= _pollfds.size()) {
            return pollCount;
        }

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
    std::vector<void*>      _selectables;
    std::vector<pollfd>     _pollfds;
};


Selector Selector::createPoll(uint eventSize) {
    auto pimpl = std::make_shared<PollSelectorImpl>(eventSize);

    return Selector(std::move(pimpl));
}
