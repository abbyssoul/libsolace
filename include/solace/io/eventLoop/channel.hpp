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
#ifndef SOLACE_IO_EVENTLOOP_CHANNEL_HPP
#define SOLACE_IO_EVENTLOOP_CHANNEL_HPP

#include "solace/io/selectable.hpp"

namespace Solace { namespace IO { namespace EventLoop {

class Channel: public ISelectable {
public:

    virtual ~Channel();

    virtual bool onRead() = 0;
    virtual bool onWrite() = 0;
};

}  // End of namespace EventLoop
}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_EVENTLOOP_CHANNEL_HPP
