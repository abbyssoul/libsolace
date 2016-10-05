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
 * libSolace: Channel for event loops
 *	@file		solace/io/eventloop/channel.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IO_EVENTLOOP_ASYNCRESULT_HPP
#define SOLACE_IO_EVENTLOOP_ASYNCRESULT_HPP

#include <functional>

namespace Solace { namespace IO { namespace async {

class Result {
public:

    Result() :
        _handler()
    {}

    Result(Result&& rhs) :
        _handler(std::move(rhs._handler))
    {}

    ~Result() = default;

//    template<typename T>
//    typename std::result_of<T()>::type then(const T& handler);

//    template<typename T>
//    T then(const std::function<T()>& handler);

    void then(const std::function<void()>& handler) {
        _handler = handler;
    }

    // Resolve this future and call the handler.
    void resolve() {
        if (_handler) {
            _handler();
        }
    }


private:

    std::function<void()> _handler;
};

}  // End of namespace async
}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_EVENTLOOP_ASYNCRESULT_HPP
