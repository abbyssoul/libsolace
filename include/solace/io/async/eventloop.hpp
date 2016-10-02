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
 * libSolace: Event loop based on selectors
 *	@file		solace/io/async/eventLoop.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IO_ASYNC_EVENTLOOP_HPP
#define SOLACE_IO_ASYNC_EVENTLOOP_HPP

//#include "solace/array.hpp"
#include "solace/byteBuffer.hpp"

#include "solace/io/selector.hpp"
#include "solace/io/async/asyncResult.hpp"
//#include "solace/io/async/channel.hpp"

#include <memory>
#include <vector>


namespace Solace { namespace IO { namespace async {

/**
 * Event Loop.
 *
 * This package allows developers to use heigher level concept of the event loop to write reactive applicatoins.
 * Event loop abstacts data sources, polling all inputs and outputs and
 * triggers all event handlers when a channel is ready.
 *
 */
class EventLoop {
public:

    class Request :
            public ISelectable {
    public:

        Request(ISelectable* selectable, ByteBuffer& buffer) :
            _selectable(selectable),
            _buffer(buffer)
        {}

        Request(Request&& rhs) :
            _selectable(rhs._selectable),
            _buffer(rhs._buffer),
            _future(std::move(rhs._future))
        {}

        Result& promiss() {
            return _future;
        }

        poll_id getSelectId() const {
            return _selectable->getSelectId();
        }

        virtual void onRead();
        virtual void onWrite();
        virtual void onError();

    private:
        ISelectable*    _selectable;
        ByteBuffer&     _buffer;
        Result          _future;
    };


public:

    EventLoop(uint32 backlogCapacity, Selector&& selector) :
        _keepOnRunning(true),
        _backlog(backlogCapacity),
        _selector(std::move(selector))
    {}

    EventLoop(EventLoop&& rhs);
    ~EventLoop() = default;

    EventLoop& operator= (EventLoop&& rhs);
    EventLoop& swap(EventLoop& rhs);

    Selector& getSelector() noexcept {
        return _selector;
    }

    const Selector& getSelector() const noexcept {
        return _selector;
    }

    /**
     * Run a single iteration of the event loop.
     *
     * @return True, if there are still more iteration to run
     */
    bool iterate() {
        return false;
    }

    void run();

    Result& submit(const std::shared_ptr<Request>& request);

private:

    bool _keepOnRunning;
    std::vector<std::shared_ptr<Request>>   _backlog;
    Selector _selector;

};

}  // End of namespace async
}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_ASYNC_EVENTLOOP_HPP

