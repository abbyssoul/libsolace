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
 * libSolace: shared(mapped) memory object
 *	@file		solace/io/anonSharedMemory.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IO_ANONSHAREDMEMORY_HPP
#define SOLACE_IO_ANONSHAREDMEMORY_HPP


#include "solace/byteBuffer.hpp"


namespace Solace { namespace IO {

/** Mapped shared memory segment
 *
 */
class MappedMemoryView: public MemoryView {
public:

    using MemoryView::size_type;

    /** Desired protection of the mapping
     * It is either None for no protection
     * or the bitwise OR of one or more of the flags
     */
    struct Protection {
        static const int None;   //!< Pages may not be accessed.
        static const int Exec;   //!< Pages may be executed.
        static const int Read;   //!< Pages may be read.
        static const int Write;  //!< Pages may be written.
    };


    /** Sharing access mode
     * Determines whether updates to the mapping are visible to other processes mapping the same region,
     * and whether updates are carried through to the underlying file (if any)
     */
    enum class Access {

        /** Share this mapping.
         * Updates to the mapping are visible to other processes that map this file,
         * and are carried through to the underlying file.
         */
        Shared,

        /** Create a private copy-on-write mapping.
         * Updates to the mapping are not visible to other processes mapping the same file,
         * and are not carried through to the underlying file.
         */
        Private
    };

public:

    /**
     * Create new ANONYMOUS shared memory segment.
     *
     * @param mapSize The size of a new memory segment to create.
     * @param mapping Mapping sharing mode. @see MappedMemoryView::Access
     * @param protection Mapped page protection. @see MappedMemoryView::Protection
     *
     * @return Memory view of the mapped shared memory region.
     */
    static MappedMemoryView create(size_type mapSize,
                                Access mapping = Access::Private,
                                int protection = Protection::Read | Protection::Write);

public:

    MappedMemoryView(size_type newSize, void* dataAddress);

    MappedMemoryView(MappedMemoryView&& rhs): MemoryView(std::move(rhs))
    {
    }

    ~MappedMemoryView();

    MappedMemoryView& swap(MappedMemoryView& rhs) noexcept {
        MemoryView::swap(rhs);

        return (*this);
    }

    MappedMemoryView& operator= (MappedMemoryView&& rhs) noexcept {
        return swap(rhs);
    }

};


}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_ANONSHAREDMEMORY_HPP
