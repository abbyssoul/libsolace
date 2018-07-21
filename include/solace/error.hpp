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
 * libSolace: Basic non-throwable error
 *	@file		solace/error.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	@brief		Basic class of errors
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_ERROR_HPP_
#define SOLACE_ERROR_HPP_

#include "solace/types.hpp"
#include "solace/stringView.hpp"


/* TODO(abbyssoul): Add interop SUPPORT for std::error
#include <system_error>
*/

namespace Solace {

class Error {
public:

    ~Error() = default;

    Error(const char* message, int code = -1) noexcept :
        _code(code),
        _message(message)
    {}

    //! Construct error with a message
    Error(StringView message, int code = -1) noexcept :
        _code(code),
        _message(message.data(), message.size())
    {}

    //! Construct error with a message
    explicit Error(std::string&& message, int code = -1) noexcept
        : _code(code)
        , _message(std::move(message))
    {}

    Error(const Error& other) = default;

    Error(Error&& other) noexcept = default;


    int value() const noexcept {
        return _code;
    }

    bool operator== (const Error& rhs) const noexcept {
        return value() == rhs.value();
    }

    Error& operator= (const Error& rhs) noexcept {
        Error(rhs).swap(*this);

        return *this;
    }

    Error& operator= (Error&& rhs) noexcept {
        return swap(rhs);
    }

    Error& swap(Error& rhs) noexcept {
        using std::swap;
        swap(_code, rhs._code);
        swap(_message, rhs._message);

        return (*this);
    }

    explicit operator bool () const noexcept {
        return (value() != 0);
    }


    //! Get message description of the exception.
    StringView toString() const;

private:

    int             _code;
    std::string     _message;		//!< Message of the exception.
};

inline void swap(Error& lhs, Error& rhs) noexcept {
    lhs.swap(rhs);
}


Error make_error(char const* msg);
Error make_errno();



}  // End of namespace Solace
#endif  // SOLACE_ERROR_HPP_
