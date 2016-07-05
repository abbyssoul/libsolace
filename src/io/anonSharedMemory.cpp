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
 * @file: io/sharedMemory.cpp
 *
 *  Created by soultaker on 03/07/16.
*******************************************************************************/
#include <solace/io/anonSharedMemory.hpp>
#include <solace/io/ioexception.hpp>


#include <sys/mman.h>
#include <sys/types.h>


using Solace::MemoryView;
using Solace::ByteBuffer;
using Solace::IO::MappedMemoryView;

using Solace::IllegalArgumentException;
using Solace::IO::IOException;


const int MappedMemoryView::Protection::None = PROT_NONE;
const int MappedMemoryView::Protection::Read = PROT_READ;
const int MappedMemoryView::Protection::Write = PROT_WRITE;
const int MappedMemoryView::Protection::Exec = PROT_EXEC;




MappedMemoryView
MappedMemoryView::create(size_type memSize, MappedMemoryView::Access mapping, int protection) {
    if (memSize == 0) {
        raise<IllegalArgumentException>("size");
    }

    int flags = MAP_ANONYMOUS;
    switch (mapping) {
    case Access::Private: flags |= MAP_PRIVATE; break;
    case Access::Shared: flags |= MAP_SHARED; break;
    }

    auto addr = mmap(NULL, memSize, protection, flags, -1, 0);
    if (addr == MAP_FAILED) {
        raise<IOException>(errno, "mmap");
    }

    return MappedMemoryView(memSize, addr);
}


MappedMemoryView::MappedMemoryView(size_type newSize, void* data):
    MemoryView(newSize, data)
{

}


MappedMemoryView::~MappedMemoryView() {
    // FIXME(abbyssoul): Some return result check might help.
    munmap(dataAddress(), size());

    // raise<IOException>(errno, "munmap");
}
