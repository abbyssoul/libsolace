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



Exception::Exception(const std::string& message, const char* file, int line) noexcept :
      #if defined(SOLACE_DEBUG)
        _message(file
                 ? fmt::format("{}, File: '{}':{}", message, file, line)
                 : message)
          , _file(file)
          , _line(line)
      #else
        _message(message)
      #endif
{
    // Nothing else to do here
}

Exception::Exception(const Exception& other) noexcept :
    _message(other._message)
  #if defined(SOLACE_DEBUG)
      , _file(other._file)
      , _line(other._line)
  #endif
{
    // Nothing else to do here
}

Exception::Exception(Exception&& other) noexcept :
    _message(std::move(other._message))
  #if defined(SOLACE_DEBUG)
      , _file(other._file)
      , _line(other._line)
  #endif
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




IllegalArgumentException::IllegalArgumentException() :
    Exception("IllegalArgumentException")
{

}


IllegalArgumentException::IllegalArgumentException(const char* argumentName) :
    Exception(fmt::format("Illegal argument '{}'", argumentName))
{
    // Nothing to do here
}


IllegalArgumentException::IllegalArgumentException(const std::string& msg,
                         const char* file, int line)
        : Exception(msg, file, line)
{

}


IndexOutOfRangeException::IndexOutOfRangeException():
    Exception("Index out of range")
{
    // No-op
}



IndexOutOfRangeException::IndexOutOfRangeException(size_t index, size_t minValue, size_t maxValue,
                                                   const char* file, const int line) noexcept :
    Exception(fmt::format("Value '{}' is out of range [{}, {})", index, minValue, maxValue),
              file, line)
{
    // No-op
}


IndexOutOfRangeException::IndexOutOfRangeException(const char* indexName,
                                                   size_t index, size_t minValue, size_t maxValue,
                                                   const char* file, const int line) noexcept :
    Exception(fmt::format("Index '{}'={} is out of range [{}, {})",
                          indexName, index, minValue, maxValue),
              file, line)
{
    // No-op
}


OverflowException::OverflowException(const char* indexName,
                                     size_t index, size_t minValue, size_t maxValue,
                                     const char* file, const int line) noexcept :
    Exception(fmt::format("Value '{}'={} overflows range [{}, {})",
                          indexName, index, minValue, maxValue),
              file, line)
{
    // Nothing else to do here
}

OverflowException::OverflowException(size_t index, size_t minValue, size_t maxValue,
                                     const char* file, const int line) noexcept :
    Exception(fmt::format("Value {} overflows range [{}, {})",
                          index, minValue, maxValue),
              file, line)
{
}

NoSuchElementException::NoSuchElementException() :
        Exception("No such element")
{
    // No-op
}

NoSuchElementException::NoSuchElementException(const char* elementName,
                                               const char* file, const int line) :
    Exception(fmt::format("No such element: '{}'", elementName),
              file, line)
{
    // Nothing else to do here
}

InvalidMarkException::InvalidMarkException(const char* file, int line) :
    Exception("InvalidMarkException", file, line)
{
    // Nothing else to do here
}


size_t Solace::validateIndex(size_t index, size_t from, size_t to) {
    if (index >= to) {
        Solace::raise<Solace::IndexOutOfRangeException>(index, from, to);
    }

    return index;
}

// From optional:
void Solace::raiseInvalidStateError() {
    Solace::raise<NoSuchElementException>("None");
}
