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
 *	@file		assert.cpp
 *	@brief		Implementation of assertion utils
 ******************************************************************************/
#include "solace/assert.hpp"
#include "solace/exception.hpp"

#include <cerrno>  // BSD defines errno in a separate header


using namespace Solace;


uint16
Solace::assertIndexInRange(uint16 index, uint16 from, uint16 to) {
    if (index >= to || index < from) {
        Solace::raise<IndexOutOfRangeException>(index, from, to);
    }

    return index;
}


uint32
Solace::assertIndexInRange(uint32 index, uint32 from, uint32 to) {
    if (index >= to || index < from) {
        Solace::raise<IndexOutOfRangeException>(index, from, to);
    }

    return index;
}

uint64 Solace::assertIndexInRange(uint64 index, uint64 from, uint64 to) {
    if (index >= to || index < from) {
        Solace::raise<IndexOutOfRangeException>(index, from, to);
    }

    return index;
}


uint64 Solace::assertIndexInRange(uint64 index, uint64 from, uint64 to, const char* message) {
    if (index >= to || index < from) {
        Solace::raise<IndexOutOfRangeException>(index, from, to, message);
    }

    return index;
}

uint32 Solace::assertIndexInRange(uint32 index, uint32 from, uint32 to, const char* message) {
    if (index >= to || index < from) {
        Solace::raise<IndexOutOfRangeException>(index, from, to, message);
    }

    return index;
}

uint16 Solace::assertIndexInRange(uint16 index, uint16 from, uint16 to, const char* message) {
    if (index >= to || index < from) {
        Solace::raise<IndexOutOfRangeException>(index, from, to, message);
    }

    return index;
}

[[noreturn]]
void Solace::raiseInvalidStateError() {
    Solace::raise<InvalidStateException>();
}

[[noreturn]]
void Solace::raiseInvalidStateError(const char* tag) {
    Solace::raise<InvalidStateException>(tag);
}


[[noreturn]]
void Solace::assertFail(const char* tag) {
    Solace::raise<Exception>(tag);
}

[[noreturn]]
void Solace::assertFailErrno(const char* tag) {
    Solace::raise<IOException>(errno, tag);
}
