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
#include <solace/io/ioexception.hpp>
#include <solace/array.hpp>

#include "selector_impl.hpp"

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>  // close()


using namespace Solace;
using namespace Solace::IO;


bool operator== (const epoll_data_t& a, const epoll_data_t& b) {
    return ((a.fd == b.fd) && (a.ptr == b.ptr) && (a.u32 == b.u32) && (a.u64 == b.u64));
}

bool operator== (const epoll_event& a, const epoll_event& b) {
    return ((a.events == b.events) && (a.data == b.data));
}



class EPollSelectorImpl : public Solace::IO::Selector::IPollerImpl {
public:

    // FIXME: evlist will actually leak if we throw here...
    explicit EPollSelectorImpl(uint maxReportedEvents): _evlist(maxReportedEvents) {
        _epfd = epoll_create(maxReportedEvents);

        if (-1 == _epfd) {
            Solace::raise<IOException>(errno);
        }
    }

    ~EPollSelectorImpl() {
        close(_epfd);  // TODO(abbyssoul): Maybe check return value?
    }

    void add(ISelectable* selectable, int events) override {
        epoll_event ev;
        ev.data.ptr = selectable;
        ev.events = 0;

        if (events & Selector::Events::Read)
            ev.events |= EPOLLIN;
        if (events & Selector::Events::Write)
            ev.events |= EPOLLOUT;
        if (events & Selector::Events::Error)
            ev.events |= EPOLLERR;
        if (events & Selector::Events::Hup)
            ev.events |= EPOLLHUP;

        if (-1 == epoll_ctl(_epfd, EPOLL_CTL_ADD, selectable->getSelectId(), &ev)) {
            Solace::raise<IOException>(errno);
        }
    }

    void remove(const ISelectable* selectable) override {
        epoll_event ev;

        ev.data.fd = selectable->getSelectId();
        ev.events = 0;

        if (-1 == epoll_ctl(_epfd, EPOLL_CTL_DEL, selectable->getSelectId(), &ev)) {
            if (errno != ENOENT)
                Solace::raise<IOException>(errno);
        }
    }


    std::tuple<uint, uint> poll(uint32 msec) override {
        const int ready = epoll_wait(_epfd, _evlist.data(), _evlist.size(), msec);

        if (ready < 0) {
            Solace::raise<IOException>(errno);
        }

        return std::make_tuple(0, ready);
    }


    Selector::Event getEvent(uint i) override {
        const auto& ev = _evlist[i];

        Selector::Event event;
        event.pollable = static_cast<ISelectable*>(ev.data.ptr);
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

    int advance(uint offsetIndex) {
        return offsetIndex + 1;
    }


private:
    EPollSelectorImpl(const EPollSelectorImpl&) = delete;
    EPollSelectorImpl& operator= (const EPollSelectorImpl&) = delete;

    Solace::Array<epoll_event> _evlist;
    int _epfd;
};



Selector Selector::createEPoll(uint eventSize) {
    auto pimpl = std::make_shared<EPollSelectorImpl>(eventSize);

    return Selector(std::move(pimpl));
}
