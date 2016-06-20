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
 * libSolace: Interface of selectable / pollable objects
 *	@file		solace/io/selectable.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IO_SELECTABLE_HPP
#define SOLACE_IO_SELECTABLE_HPP

namespace Solace { namespace IO {

/**
 * A base interface for an object that can be polled by select/poll/epoll
 * family of functions
 */
class ISelectable {
public:
    //!< File Id is defined as int by POSIX
	typedef int poll_id;

public:

    /**
     * Default virtual destructor is fine for a base virtual class.
     */
	virtual ~ISelectable() = default;

    /**
     * Get a 'native' file id of this object to be used by poll/select functions.
     * @return
     */
    virtual poll_id getSelectId() const = 0;
};


}  // namespace IO
}  // namespace Solace
#endif  // SOLACE_IO_SELECTABLE_HPP
