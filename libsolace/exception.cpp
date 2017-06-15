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
 *	@file		exception.cpp
 *	@author		$LastChangedBy: $
 *	@date		$LastChangedDate: $
 *	@brief		Implementation of exception types
 *	ID:			$Id: $
 ******************************************************************************/
#include "solace/exception.hpp"
#include "solace/string.hpp"

#include <fmt/format.h>



using Solace::Exception;
using Solace::String;
using Solace::IllegalArgumentException;
using Solace::IndexOutOfRangeException;
using Solace::OverflowException;
using Solace::NoSuchElementException;
using Solace::InvalidMarkException;
using Solace::IOException;
using Solace::NotOpen;


static const char* IOExceptionType = "IOException";



std::string formatErrono(int errorCode) {
    const auto s = String::join(":", {IOExceptionType, String::valueOf(errorCode), strerror(errorCode)});

    return s.to_str();
}

std::string formatErrono(int errorCode, const std::string& msg) {
    const auto s = String::join(":", {IOExceptionType, String::valueOf(errorCode), msg, strerror(errorCode)});

    return s.to_str();
}




Exception::Exception(const std::string& message) noexcept :
        _message(message)
{
    // Nothing else to do here
}

Exception::Exception(const Exception& other) noexcept :
    _message(other._message)
{
    // Nothing else to do here
}

Exception::Exception(Exception&& other) noexcept :
    _message(std::move(other._message))
{
    // Nothing else to do here
}


String Exception::getMessage() const noexcept {
    return _message;
}


const char* Exception::what() const noexcept {
    return _message.c_str();
}

String Exception::toString() const {
    return _message;
}




IllegalArgumentException::IllegalArgumentException() noexcept:
    Exception("IllegalArgumentException")
{

}


IllegalArgumentException::IllegalArgumentException(const char* argumentName) noexcept:
    Exception(fmt::format("Illegal argument '{}'", argumentName))
{
    // Nothing to do here
}


IllegalArgumentException::IllegalArgumentException(const std::string& msg) noexcept:
    Exception(msg)
{

}


IndexOutOfRangeException::IndexOutOfRangeException() noexcept:
    Exception("Index out of range")
{
    // No-op
}



IndexOutOfRangeException::IndexOutOfRangeException(size_t index, size_t minValue, size_t maxValue) noexcept :
    Exception(fmt::format("Value '{}' is out of range [{}, {})", index, minValue, maxValue))
{
    // No-op
}


IndexOutOfRangeException::IndexOutOfRangeException(const char* indexName,
                                                   size_t index, size_t minValue, size_t maxValue) noexcept :
    Exception(fmt::format("Index '{}'={} is out of range [{}, {})", indexName, index, minValue, maxValue))
{
    // No-op
}


OverflowException::OverflowException(const char* indexName,
                                     size_t index, size_t minValue, size_t maxValue) noexcept :
    Exception(fmt::format("Value '{}'={} overflows range [{}, {})",
                          indexName, index, minValue, maxValue))
{
    // Nothing else to do here
}


OverflowException::OverflowException(size_t index, size_t minValue, size_t maxValue) noexcept :
    Exception(fmt::format("Value {} overflows range [{}, {})", index, minValue, maxValue))
{
}


NoSuchElementException::NoSuchElementException() noexcept:
        Exception("No such element")
{
    // No-op
}


NoSuchElementException::NoSuchElementException(const char* elementName) noexcept:
    Exception(fmt::format("No such element: '{}'", elementName))
{
    // Nothing else to do here
}


InvalidMarkException::InvalidMarkException() noexcept:
    Exception("InvalidMarkException")
{
    // Nothing else to do here
}


IOException::IOException(const std::string& msg) noexcept: Exception(msg), _errorCode(-1) {
}


IOException::IOException(int errorCode) noexcept:
    Exception(formatErrono(errorCode)),
    _errorCode(errorCode)
{
}


IOException::IOException(int errorCode, const std::string& msg) noexcept:
    Exception(formatErrono(errorCode, msg)),
    _errorCode(errorCode)
{
}


NotOpen::NotOpen() noexcept:
    IOException("File descriptor not opened")
{
    // No-op
}

