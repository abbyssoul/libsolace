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


#include "solace/byteBuffer.hpp"
#include "solace/io/selector.hpp"
#include "solace/io/async/asyncResult.hpp"


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

        Request()
        {}

        Request(Request&& rhs) :
            _future(std::move(rhs._future))
        {}

        virtual ~Request() = default;

        /**
         * Check if this request has been resolved.
         * If the request has been resolved - it will be removed from the backlog.
         *
         * @return True, if this request has been resolve, false otherwise.
         */
        virtual bool isComplete() const noexcept {
            return false;
        }

        Result& promiss() noexcept {
            return _future;
        }

        virtual poll_id getSelectId() const = 0;

        virtual void onRead() = 0;
        virtual void onWrite() = 0;
        virtual void onError();

    private:
        Result          _future;
    };


public:

    /**
     * Construct a new event loop/io context.
     *
     * @param backlogCapacity Maximum number of concurent request in flight.
     * @param selector Selector service to use to dispatch requests.
     */
    EventLoop(uint32 backlogCapacity, Selector&& selector);

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

    void stop() noexcept {
        _keepOnRunning = false;
    }

    bool isStopped() const noexcept {
        return _keepOnRunning;
    }

    /**
     * Submit a request to the event loop.
     * This method is designed to be used by IO objects which createa custom request objects.
     *
     * @param request A request object to execute.
     * @return A promiss that will be called once request has been processed.
     */
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

