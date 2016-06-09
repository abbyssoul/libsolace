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
 * @file: io/selector.cpp
 *
 *  Created by soultaker on 27/04/16.
*******************************************************************************/
#include <solace/io/selector.hpp>
#include <solace/io/ioexception.hpp>
#include <solace/array.hpp>

#include <memory>   // std::unique_ptr<>


#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>  // close()


using Solace::uint32;
using Solace::String;
using Solace::IO::ISelectable;
using Solace::IO::Selector;

using Solace::IndexOutOfRangeException;
using Solace::IO::IOException;


bool operator== (const epoll_data_t& a, const epoll_data_t& b) {
    return ((a.fd == b.fd) && (a.ptr == b.ptr) && (a.u32 == b.u32) && (a.u64 == b.u64));
}

bool operator== (const epoll_event& a, const epoll_event& b) {
    return ((a.events == b.events) && (a.data == b.data));
}

class Solace::IO::Selector::IPollerImpl {
public:

    virtual ~IPollerImpl() = default;

    virtual void add(ISelectable* selectable, int events) = 0;

    virtual void remove(const ISelectable* selectable) = 0;

    virtual int poll(uint32 msec) = 0;

    virtual Selector::Event getEvent(uint i) = 0;
};


class EPoll_PollerImpl : public Solace::IO::Selector::IPollerImpl {
public:

    // FIXME: evlist will actually leak if we throw here...
    EPoll_PollerImpl(uint maxReportedEvents): _evlist(maxReportedEvents) {
        _epfd = epoll_create(maxReportedEvents);

        if (-1 == _epfd) {
            Solace::raise<IOException>(errno);
        }
    }

    ~EPoll_PollerImpl() {
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
            Solace::raise<IOException>(errno);
        }
    }

    int poll(uint32 msec) override {
        const int ready = epoll_wait(_epfd, _evlist.data(), _evlist.size(), msec);

        if (ready < 0) {
            Solace::raise<IOException>(errno);
        }

        return ready;
    }

    Selector::Event getEvent(uint i) override {
        const epoll_event& ev = _evlist[i];

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

private:
    EPoll_PollerImpl(const EPoll_PollerImpl&) = delete;
    EPoll_PollerImpl& operator= (const EPoll_PollerImpl&) = delete;
    bool operator== (const EPoll_PollerImpl&) = delete;

    Solace::Array<epoll_event> _evlist;
    int _epfd;

};


const Selector::Iterator& Selector::Iterator::operator++ () {
    if (_index < _size)
        ++_index;
    else {
        // Actually we are better off raising an exception:
        Solace::raise<IndexOutOfRangeException>(static_cast<size_t>(_index),
                                                static_cast<size_t>(0),
                                                static_cast<size_t>(_size),
                                                "iterator");
        _index = _size;
    }

    return (*this);
}

Selector::Event Selector::Iterator::operator-> () const {
    return _pimpl->getEvent(_index);
}


Selector Selector::epoll(uint eventSize) {
    auto pimpl = std::make_shared<EPoll_PollerImpl>(eventSize);

    return Selector(std::move(pimpl));
}

Selector::Selector(const std::shared_ptr<IPollerImpl>& impl): _pimpl(impl) {
}

Selector::Selector(std::shared_ptr<IPollerImpl>&& impl): _pimpl(std::move(impl)) {
}

void Selector::add(ISelectable* selectable, int events) {
    _pimpl->add(selectable, events);
}

void Selector::remove(const ISelectable* selectable) {
    _pimpl->remove(selectable);
}

Selector::Iterator Selector::poll(uint msec) {

    const auto nbReady = _pimpl->poll(msec);

    return Selector::Iterator(nbReady, _pimpl);
}
