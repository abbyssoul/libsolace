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
 * @file: solace/io/serial.cpp
 *
 *  Created by soultaker on 27/04/16.
*******************************************************************************/
#include <solace/io/ioexception.hpp>

#include <fmt/format.h>
#include <cstring>


using Solace::IO::IOException;
using Solace::IO::NotOpen;


const char* ExceptionType = "IOException";


IOException::IOException(): Exception(ExceptionType), _errorCode(-1) {

}

IOException::IOException(const std::string& msg): Exception(msg), _errorCode(-1) {
}


IOException::IOException(int errorCode):
	Exception(
        fmt::format("{0}: {1}: {2}",
                    ExceptionType,
                    errorCode,
                    strerror(errorCode))), 
    _errorCode(errorCode)
{
}


IOException::IOException(int errorCode, const std::string& msg):
    Exception(
        fmt::format("{0}:{2}:{1}: {3}",
                    ExceptionType,
                    errorCode,
                    msg,
                    strerror(errorCode))),
    _errorCode(errorCode)
{
}


NotOpen::NotOpen(): IOException("File descriptor not opened") {
    // No-op
}

