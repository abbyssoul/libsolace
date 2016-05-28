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
 * libSolace: Exeption hierarchy of IO operations
 *  @file:      solace/io/ioexception.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IO_IOEXCEPTION_HPP
#define SOLACE_IO_IOEXCEPTION_HPP


#include "solace/exception.hpp"


namespace Solace { namespace IO {

/**
 * Error type thrown by IO operations
 */
class IOException: public Exception {
public:

    IOException();

    IOException(int errorCode);

    IOException(int errorCode, const std::string& msg);

    IOException(const std::string& msg);

    virtual ~IOException() noexcept = default;

private:
	int errorCode;
};


/**
 * Special case of IOException for attemping to access not yet opened file
 */
class NotOpen: public IOException {
public:
    NotOpen();
};


}  // namespace IO
}  // namespace Solace
#endif  // SOLACE_IO_IOEXCEPTION_HPP
