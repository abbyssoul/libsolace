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
 * @file: io/selector_epoll.cpp
 * epoll based implemenation of the selector
 *
 *  Created by soultaker on 27/04/16.
*******************************************************************************/
#include <solace/io/selector.hpp>
#include <solace/exception.hpp>
#include <solace/array.hpp>

#include "selector_impl.hpp"


#include <vector>
#include <algorithm>  // remove_if
#include <unistd.h>  // close()
#include <fcntl.h>

#ifdef SOLACE_PLATFORM_LINUX

#include <sys/epoll.h>


using namespace Solace;
using namespace Solace::IO;


bool operator== (const epoll_data_t& a, const epoll_data_t& b) {
    return (a.fd == b.fd);
}


bool operator== (const epoll_event& a, const epoll_event& b) {
    return ((a.events == b.events) && (a.data == b.data));
}

namespace /*anonymous*/ {

class EPollSelectorImpl :
        public Solace::IO::Selector::IPollerImpl {
public:

    ~EPollSelectorImpl() override {
        close(_epfd);  // TODO(abbyssoul): Maybe check return value?
    }

    EPollSelectorImpl(EPollSelectorImpl const&) = delete;
    EPollSelectorImpl& operator= (EPollSelectorImpl const&) = delete;

    // FIXME: evlist will actually leak if we throw here...
    explicit EPollSelectorImpl(uint maxReportedEvents)
        : _evlist(maxReportedEvents)
        , _epfd{epoll_create(maxReportedEvents)}
    {
        if (-1 == _epfd) {
            Solace::raise<IOException>(errno);
        }

        _selectables.reserve(maxReportedEvents);
    }

    void add(ISelectable* selectable, int events) override {
        add(selectable->getSelectId(), events, selectable);
    }


    void add(ISelectable::poll_id fd, int events, void* data) override {
        int nativeEvents = 0;

        if (events & Selector::Events::Read)
            nativeEvents |= EPOLLIN;
        if (events & Selector::Events::Write)
            nativeEvents |= EPOLLOUT;
        if (events & Selector::Events::Error)
            nativeEvents |= EPOLLERR;
        if (events & Selector::Events::Hup)
            nativeEvents|= EPOLLHUP;

        addRaw(fd, nativeEvents, data);
    }


    void addRaw(ISelectable::poll_id fd, int nativeEvents, void* data) override {

        Selector::Event ev;
        ev.data = data;
        ev.fd = fd;
        _selectables.push_back(ev);

        epoll_event epollEvent;
        epollEvent.data.ptr = &_selectables.back();
        epollEvent.events = nativeEvents;

        if (-1 == epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &epollEvent)) {
            Solace::raise<IOException>(errno);
        }
    }


    void remove(const ISelectable* selectable) override {
        remove(selectable->getSelectId());
    }


    void remove(ISelectable::poll_id fd) override {
        epoll_event ev;

        ev.events = 0;
        ev.data.fd = fd;

        if (-1 == epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, &ev)) {
            if (errno != ENOENT) {
                Solace::raise<IOException>(errno);
            }
        }

        _selectables.erase(
                    std::remove_if(_selectables.begin(), _selectables.end(), [fd](auto x) { return x.fd == fd; }),
                    _selectables.end());
    }


    std::tuple<uint32, uint32> poll(int msec) override {

        for (int i = 0; i < 3; ++i) {   // Allow for 3 interapts in a row
            const int ready = epoll_wait(_epfd, _evlist.data(), _evlist.size(), msec);

            if (ready < 0) {
                if (errno != EINTR) {
                    Solace::raise<IOException>(errno);
                }
            } else {
                return std::make_tuple(0, ready);
            }
        }

        return std::make_tuple(0, 0);
    }


    Selector::Event getEvent(size_t i) override {
        const auto& ev = _evlist[i];
        auto const selected = static_cast<Selector::Event*>(ev.data.ptr);

        Selector::Event event;
        event.fd = selected->fd;
        event.data = selected->data;
        event.events = 0;

        if (ev.events & EPOLLIN)
            event.events |= Selector::Events::Read;
        if (ev.events & EPOLLOUT)
            event.events |= Selector::Events::Write;
        if (ev.events & EPOLLERR)
            event.events |= Selector::Events::Error;
        if (ev.events & EPOLLHUP)
            event.events |= Selector::Events::Hup;

        return event;
    }

    size_t advance(size_t offsetIndex) override {
        return offsetIndex + 1;
    }


private:
    std::vector<Selector::Event>    _selectables;
    std::vector<epoll_event>        _evlist;
    int                             _epfd;
};

}  // namespace


Selector Selector::createEPoll(uint eventSize) {
    return Selector(std::make_unique<EPollSelectorImpl>(eventSize));
}

#endif  // SOLACE_PLATFORM_LINUX
