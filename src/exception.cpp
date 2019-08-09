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
 *	@brief		Implementation of exception types
 ******************************************************************************/
#include "solace/exception.hpp"
#include "solace/string.hpp"

#include <cstring>


using namespace Solace;


static constexpr StringLiteral IOExceptionType{"IOException"};



std::string formatErrono(int errorCode) {
    std::string msg{IOExceptionType.data(), IOExceptionType.size()};

    msg.append("[");
    msg.append(std::to_string(errorCode));
    msg.append("]: ");
    msg.append(strerror(errorCode));

    return msg;
}

std::string formatErrono(int errorCode, std::string const& msgex) {
    std::string msg{IOExceptionType.data(), IOExceptionType.size()};

    msg.append("[");
    msg.append(std::to_string(errorCode));
    msg.append("]: ");
    msg.append(msgex);
    msg.append(" - ");
    msg.append(strerror(errorCode));

    return msg;
}




// cppcheck-suppress passedByValue
Exception::Exception(std::string message) noexcept
    : _message(std::move(message))
{
    // Nothing else to do here
}


StringView Exception::getMessage() const noexcept {
    return StringView{_message.c_str(), static_cast<StringView::size_type>(_message.size())};
}


const char* Exception::what() const noexcept {
    return _message.c_str();
}

StringView Exception::toString() const {
    return StringView{_message.c_str(), static_cast<StringView::size_type>(_message.size())};
}




IllegalArgumentException::IllegalArgumentException() noexcept:
    Exception("IllegalArgumentException")
{

}

std::string formatIlligalArgName(const char* argName) {
    std::string msg{"Illegal argument '"};
    msg.append(argName);
    msg.append("'");

    return msg;
}

IllegalArgumentException::IllegalArgumentException(const char* argumentName) noexcept:
    Exception(formatIlligalArgName(argumentName))
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


std::string formatIndexOutOfRangeError(const char* messagePrefix, const char* indexName,
                                       size_t index, size_t minValue, size_t maxValue) {
    std::string msg{messagePrefix};

    if (indexName) {
        msg.append(" '");
        msg.append(indexName);
        msg.append("'=");
    } else {
        msg.append(": ");
    }

    msg.append(std::to_string(index));
    msg.append(" is out of range [");
    msg.append(std::to_string(minValue));
    msg.append(", ");
    msg.append(std::to_string(maxValue));
    msg.append(")");

    return msg;
}


IndexOutOfRangeException::IndexOutOfRangeException(size_t index, size_t minValue, size_t maxValue) noexcept :
    Exception(formatIndexOutOfRangeError("Error", nullptr, index, minValue, maxValue))
{
    // No-op
}


IndexOutOfRangeException::IndexOutOfRangeException(const char* indexName,
                                                   size_t index, size_t minValue, size_t maxValue) noexcept :
    Exception(formatIndexOutOfRangeError("Index", indexName, index, minValue, maxValue))
{
    // No-op
}


IndexOutOfRangeException::IndexOutOfRangeException(size_t index, size_t minValue, size_t maxValue,
                                                   const char* messagePrefix) noexcept :
    Exception(formatIndexOutOfRangeError(messagePrefix, nullptr, index, minValue, maxValue))
{
    // No-op
}



std::string formatOverflowError(const char* indexName,
                                size_t index, size_t minValue, size_t maxValue) {
    std::string msg{"Value"};

    if (indexName) {
        msg.append(" '");
        msg.append(indexName);
        msg.append("'=");
    } else {
        msg.append(" ");
    }

    msg.append(std::to_string(index));
    msg.append(" overflows range [");
    msg.append(std::to_string(minValue));
    msg.append(", ");
    msg.append(std::to_string(maxValue));
    msg.append(")");

    return msg;

}


OverflowException::OverflowException(const char* indexName,
                                     size_t index, size_t minValue, size_t maxValue) noexcept :
    Exception(formatOverflowError(indexName, index, minValue, maxValue))
{
    // Nothing else to do here
}


OverflowException::OverflowException(size_t index, size_t minValue, size_t maxValue) noexcept :
    Exception(formatOverflowError(nullptr, index, minValue, maxValue))
{
}


NoSuchElementException::NoSuchElementException() noexcept:
        Exception("No such element")
{
    // No-op
}


NoSuchElementException::NoSuchElementException(const char* elementName) noexcept:
    Exception(std::string("No such element: " + std::string(elementName)))
{
    // Nothing else to do here
}


InvalidStateException::InvalidStateException() noexcept
    : Exception("Invalid State")
{

}

InvalidStateException::InvalidStateException(const char* tag) noexcept
    : Exception(std::string("Invalid state: " + std::string(tag)))
{

}


IOException::IOException(const std::string& msg) noexcept
    : Exception(msg)
    , _errorCode(-1)
{
}


IOException::IOException(int errorCode) noexcept
    : Exception(formatErrono(errorCode))
    , _errorCode(errorCode)
{
}


IOException::IOException(int errorCode, const std::string& msg) noexcept
    : Exception(formatErrono(errorCode, msg))
    , _errorCode(errorCode)
{
}


NotOpen::NotOpen() noexcept
    : IOException("File descriptor not opened")
{
    // No-op
}

