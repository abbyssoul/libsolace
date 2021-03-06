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
 *	@brief		Base exectpion types used by core library
 ******************************************************************************/
#pragma once
#ifndef SOLACE_EXCEPTION_HPP
#define SOLACE_EXCEPTION_HPP

#include "solace/types.hpp"
#include "solace/stringView.hpp"
#include "solace/details/string_utils.hpp"

/* TODO(abbyssoul):
#if defined(SOLACE_DEBUG)
#include "solace/debug/stacktrace.hpp"
#endif  // SOLACE_DEBUG
*/

#include <exception>


namespace Solace {

/** Base of exceptions hierarchy.
 *
 */
class Exception :
        public std::exception {
public:

    ~Exception() noexcept override = default;

    //! Construct exception w. message
	Exception(StringView message) noexcept;
	Exception(Exception&& other) noexcept = default;

	virtual StringView getMessage() const noexcept {
		return _message.view();
	}

    //! STD compatible message:
    const char* what() const noexcept override;

    //! Get message description of the exception.
	StringView toString() const noexcept {
		return _message.view();
	}

protected:

	/*constexpr */Exception(details::ErrorString&& msg) noexcept
		: _message{mv(msg)}
	{}

private:

	details::ErrorString	_message;		//!< Message of the exception.

#ifdef SOLACE_DEBUG
//  const Debug::Trace 	_trace;			//!< Stack trace
#endif  // SOLACE_DEBUG

};


/**
 * An error type to signal that argument value is illegal.
*/
struct IllegalArgumentException final : public Exception {

	IllegalArgumentException() noexcept;

	IllegalArgumentException(StringLiteral msg) noexcept;
};


/**
 * An error type to signal that index value is outsige of acceptable range.
*/
struct IndexOutOfRangeException final : public Exception {
    IndexOutOfRangeException() noexcept;

	//! Construct exception given expected range values:
	IndexOutOfRangeException(uint64 index, uint64 minValue, uint64 maxValue) noexcept;

	//! Construct exception given expected range values:
	IndexOutOfRangeException(uint32 index, uint32 minValue, uint32 maxValue) noexcept;

	//! Construct exception given expected range values:
	IndexOutOfRangeException(uint16 index, uint16 minValue, uint16 maxValue) noexcept;

    //! Construct exception with custom message
	IndexOutOfRangeException(uint64 index, uint64 minValue, uint64 maxValue, const char* messagePrefix) noexcept;
	IndexOutOfRangeException(uint32 index, uint32 minValue, uint32 maxValue, const char* messagePrefix) noexcept;
	IndexOutOfRangeException(uint16 index, uint16 minValue, uint16 maxValue, const char* messagePrefix) noexcept;

};

/**
 * An error to signal that operation leads to overflow of some internal buffer
 */
struct OverflowException final : public Exception {
	OverflowException(StringLiteral indexName, uint16 index, uint16 minValue, uint16 maxValue) noexcept;
	OverflowException(StringLiteral indexName, uint32 index, uint32 minValue, uint32 maxValue) noexcept;
	OverflowException(StringLiteral indexName, uint64 index, uint64 minValue, uint64 maxValue) noexcept;

	OverflowException(uint16 index, uint16 minValue, uint16 maxValue) noexcept;
	OverflowException(uint32 index, uint32 minValue, uint32 maxValue) noexcept;
	OverflowException(uint64 index, uint64 minValue, uint64 maxValue) noexcept;
};


/**
 * Raised by accessor methods to signal that requested element does not exist.
 */
struct NoSuchElementException final : public Exception {
    NoSuchElementException() noexcept;

	NoSuchElementException(StringLiteral elementName) noexcept;
};


/**
 * Raised by accessor methods to signal that requested element does not exist.
 */
struct InvalidStateException final : public Exception {
    InvalidStateException() noexcept;

    InvalidStateException(const char* tag) noexcept;
};


/**
 * Exception during IO operations
 */
struct IOException: public Exception {

	IOException(int errorCode) noexcept;

	IOException(int errorCode, StringView msg) noexcept;

	IOException(StringView msg) noexcept;

    int getErrorCode() const noexcept {
        return _errorCode;
    }

private:
    int _errorCode;
};


/**
 * Special case of IOException for attemping to access not yet opened file
 */
struct NotOpen final : public IOException {
    NotOpen() noexcept;
};


template <typename ExceptionType, typename... Args>
[[noreturn]]
void raise(Args&&... args) {
	throw ExceptionType(fwd<Args>(args)...);
}

}  // End of namespace Solace
#endif  // SOLACE_EXCEPTION_HPP
