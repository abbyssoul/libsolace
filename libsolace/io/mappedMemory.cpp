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
#include <solace/io/mappedMemory.hpp>
#include <solace/exception.hpp>


#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>


using Solace::MemoryView;
using Solace::ByteBuffer;
using Solace::IO::MappedMemoryView;

using Solace::IllegalArgumentException;
using Solace::IOException;


const int MappedMemoryView::Protection::None = PROT_NONE;
const int MappedMemoryView::Protection::Read = PROT_READ;
const int MappedMemoryView::Protection::Write = PROT_WRITE;
const int MappedMemoryView::Protection::Exec = PROT_EXEC;



void* mapMemory(MappedMemoryView::size_type memSize, int protection, MappedMemoryView::Access mapping, int fd) {
    if (memSize == 0) {
        Solace::raise<IllegalArgumentException>("size");
    }

    int flags = (fd == -1) ? MAP_ANONYMOUS : 0;
    switch (mapping) {
    case MappedMemoryView::Access::Private: flags |= MAP_PRIVATE; break;
    case MappedMemoryView::Access::Shared: flags |= MAP_SHARED; break;
    }

    auto addr = mmap(nullptr, memSize, protection, flags, fd, 0);
    if (addr == MAP_FAILED) {
        Solace::raise<IOException>(errno, "mmap");
    }

    return addr;
}


MappedMemoryView
MappedMemoryView::create(size_type memSize, MappedMemoryView::Access mapping, int protection) {
    return map(-1, memSize, mapping, protection);
}


MappedMemoryView
MappedMemoryView::map(int fd, size_type memSize, MappedMemoryView::Access mapping, int protection) {
    auto addr = mapMemory(memSize, protection, mapping, fd);

    return MappedMemoryView(memSize, addr);
}


MappedMemoryView::MappedMemoryView(size_type newSize, void* data):
    MemoryView(newSize, data, 0)
{
    // No-op
}

MappedMemoryView::MappedMemoryView() : MemoryView()
{

}


MappedMemoryView::~MappedMemoryView() {
    // FIXME(abbyssoul): Some return result check might help.
    const auto s = size();
    const auto addr = dataAddress();
    if (addr && s > 0) {
        munmap(dataAddress(), size());
    }
    // raise<IOException>(errno, "munmap");
}
