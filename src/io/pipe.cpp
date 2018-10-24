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
 * @file: solace/io/pipe.cpp
 *
 *  Created by soultaker on 10/10/16.
*******************************************************************************/
#include <solace/io/pipe.hpp>
#include <solace/exception.hpp>


#include <unistd.h>
#include <fcntl.h>


using namespace Solace;
using namespace Solace::IO;


Duplex createPipe() {
    int fds[2];
    auto const r = pipe(fds);
    if (r < 0) {
        Solace::raise<IOException>(errno, "pipe");
    }

    return Duplex(fds[0], fds[1]);
}


Pipe::Pipe() :
    Pipe(createPipe())
{
}

Pipe::Pipe(Duplex&& duplex) : Duplex(std::move(duplex))
{
}

Pipe::Pipe(Pipe&& duplex) : Duplex(std::move(duplex))
{
}

