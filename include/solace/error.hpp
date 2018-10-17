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
#include "solace/atom.hpp"
#include "solace/stringView.hpp"
#include "solace/errorDomain.hpp"



namespace Solace {



/**
 * Error type
 */
class Error {
public:

    //! Construct error with a message
    Error(AtomValue errorDomain, int code, StringLiteral tag) noexcept
        : _domain(errorDomain)
        , _code(code)
        , _tag(std::move(tag))
    {}

    //! Construct error with a message
    Error(AtomValue errorDomain, int code) noexcept
        : _domain(errorDomain)
        , _code(code)
    {}


    int value() const noexcept {
        return _code;
    }

    StringView tag() const noexcept {
        return _tag;
    }

    AtomValue domain() const noexcept {
        return _domain;
    }

    bool operator== (Error const& rhs) const noexcept {
        return ((_domain == rhs._domain) &&
                (_code == rhs._code));
    }

    Error& swap(Error& rhs) noexcept {
        using std::swap;
        swap(_tag, rhs._tag);
        swap(_code, rhs._code);
        swap(_domain, rhs._domain);

        return (*this);
    }

    explicit operator bool () const noexcept {
        return (value() != 0);
    }

    //! Get message description of the exception.
    StringView toString() const;

private:

    AtomValue       _domain;
    int             _code;
    StringLiteral   _tag;		//!< Tag for the error.
};


inline void swap(Error& lhs, Error& rhs) noexcept {
    lhs.swap(rhs);
}





inline
Error makeError(BasicError errCode, StringLiteral tag) {
    return Error{kSystemCatergory, static_cast<int>(errCode), tag};
}

inline
Error makeError(GenericError errCode, StringLiteral tag) {
    return Error{kSystemCatergory, static_cast<int>(errCode), tag};
}

inline
Error makeError(AsyncError errCode, StringLiteral tag) {
    return Error{kSystemCatergory, static_cast<int>(errCode), tag};
}

inline
Error makeError(SystemErrors errCode, StringLiteral tag) {
    return Error{kSystemCatergory, static_cast<int>(errCode), tag};
}



Error makeErrno(int errCode, StringLiteral tag);
Error makeErrno(int errCode);
Error makeErrno(StringLiteral tag);
Error makeErrno();


static_assert(sizeof(Error) <= 4*sizeof(void*),
              "Error must be no more then 2 pointers in size");

static_assert(std::is_trivially_copyable<Error>::value,
              "Error is not trivially copyable");

}  // End of namespace Solace
#endif  // SOLACE_ERROR_HPP_
