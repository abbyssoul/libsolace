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
 * libSolace: Base of exception hierarchy
 *	@file		solace/exception.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	@brief		Base exectpion types used by core library
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_EXCEPTION_HPP
#define SOLACE_EXCEPTION_HPP

#include "solace/types.hpp"
#include "solace/stringView.hpp"

/* TODO(abbyssoul):
#if defined(SOLACE_DEBUG)
#include "solace/debug/stacktrace.hpp"
#endif  // SOLACE_DEBUG
*/

#include <exception>
#include <string>       // std::string (duh!)


namespace Solace {

/** Base of exceptions hierarchy.
 *
 */
class Exception :
        public std::exception {
public:

    ~Exception() noexcept override = default;

    //! Construct exception w. message
    Exception(std::string message) noexcept;

    Exception(Exception const& other)       = default;
    Exception(Exception&& other) noexcept   = default;

    virtual StringView getMessage() const noexcept;

    //! STD compatible message:
    const char* what() const noexcept override;

    //! Get message description of the exception.
    StringView toString() const;

private:

    std::string	_message;		//!< Message of the exception.

#ifdef SOLACE_DEBUG
//  const Debug::Trace 	_trace;			//!< Stack trace
#endif  // SOLACE_DEBUG

};


/**
 * An error type to signal that argument value is illegal.
*/
class IllegalArgumentException: public Exception {
public:
    IllegalArgumentException() noexcept;

    IllegalArgumentException(const char* argumentName) noexcept;

    explicit IllegalArgumentException(const std::string& msg) noexcept;
};


/**
 * An error type to signal that index value is outsige of acceptable range.
*/
class IndexOutOfRangeException : public Exception {
public:

    IndexOutOfRangeException() noexcept;

    //! Construct exception given expected range values:
    IndexOutOfRangeException(size_t index, size_t minValue, size_t maxValue) noexcept;

    //! Construct exception with index name
    IndexOutOfRangeException(const char* indexName, size_t index, size_t minValue, size_t maxValue) noexcept;

    //! Construct exception with custom message
    IndexOutOfRangeException(size_t index, size_t minValue, size_t maxValue, const char* messagePrefix) noexcept;
};

/**
 * An error to signal that operation leads to overflow of some internal buffer
 */
class OverflowException : public Exception {
public:

    OverflowException(const char* indexName, size_t index, size_t minValue, size_t maxValue) noexcept;

    OverflowException(size_t index, size_t minValue, size_t maxValue) noexcept;
};


/**
 * Raised by accessor methods to signal that requested element does not exist.
 */
class NoSuchElementException: public Exception {
public:

    NoSuchElementException() noexcept;

    NoSuchElementException(const char* elementName) noexcept;
};


/**
 * Raised by accessor methods to signal that requested element does not exist.
 */
class InvalidStateException: public Exception {
public:

    InvalidStateException() noexcept;

    InvalidStateException(const char* tag) noexcept;
};


/**
 * Exception during IO operations
 */
class IOException: public Exception {
public:

    IOException(int errorCode) noexcept;

    IOException(int errorCode, const std::string& msg) noexcept;

    IOException(const std::string& msg) noexcept;


    int getErrorCode() const noexcept {
        return _errorCode;
    }

private:
    int _errorCode;
};


/**
 * Special case of IOException for attemping to access not yet opened file
 */
class NotOpen: public IOException {
public:
    NotOpen() noexcept;
};


template <typename ExceptionType, typename... Args>
[[noreturn]]
void raise(Args&&... args) {
    throw ExceptionType(std::forward<Args>(args)...);
}

}  // End of namespace Solace

#endif  // SOLACE_EXCEPTION_HPP
