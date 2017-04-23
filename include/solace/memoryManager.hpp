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
 * libSolace: MemoryManager
 *	@file		solace/memoryManager.hpp
 *	@author		$LastChangedBy: $
 *	@date		$LastChangedDate: $
 *	@brief		Platform memory manager interface
 *	ID:			$Id: $
 ******************************************************************************/
#pragma once
#ifndef SOLACE_MEMORYMANAGER_HPP
#define SOLACE_MEMORYMANAGER_HPP

#include "solace/memoryView.hpp"

namespace Solace {

/**
 * An interface for platform's virtual memory manager.
 * An object of this class is to be used for all operations that require memory allocation.
 * The purpose of this interface is to make memory allocation explicit.
 * This enables a fine control over when memory allocation is allowed. For instance for an application to
 * respect the "Power of 10" rules - memory allocation only allowed during initialization phase.
 * Once initialized application should not allocate memory. This class allows for such implimentations.
 */
class MemoryManager {
public:
    typedef MemoryView::size_type       size_type;
    typedef MemoryView::value_type      value_type;

    typedef void* MemoryAddress;

public:

    /** Construct a new memory manager with the given capacity
     *
     * @param allowedCapacity The memory capacity this manager allowed to allocate.
     */
    explicit MemoryManager(size_type allowedCapacity);

    MemoryManager(const MemoryManager&) = delete;

    /** Deallocate memory */
    virtual ~MemoryManager() = default;

    MemoryManager& swap(MemoryManager& rhs) noexcept;
    MemoryManager& operator= (MemoryManager&& rhs) noexcept;

    /**
     * Check if this memory manager has no allocated memory.
     * @return True if no memory is allocated by this manager.
     */
    bool empty() const noexcept;

    /** Get amount of memory in bytes allocated by the memory manager.
     * @return Total amount of memory allocated by this manager.
     */
    size_type size() const noexcept;

    /**
     * @return Total amount of memory manager is allowed to allocate.
     */
    size_type capacity() const noexcept;

    /** Get size of a memory page in bytes.
     * @return Size of the system's memory page in bytes.
     */
    size_type getPageSize() const;

    /** Get the number of pages of physical memory.
     * @return The number of pages of physical memory.
     */
    size_type getNbPages() const;

    /** Get the number of currently available pages of physical memory.
     * @return The number of currently available pages of physical memory.
     */
    size_type getNbAvailablePages() const;

    /**
     * Allocate a memory segment of the give size.
     * @note Memory allocation may be explicitly prohibited by calling lock on this instance.
     * In this case the call to allocate a new memory segment will fail.
     *
     * @param dataSize The size of the memory segment to allocate.
     * @return A newly allocated memory segment.
     * TODO(abbyssoul): should return Result<>
     */
    MemoryView create(size_type dataSize);

    /**
     * Prohibit memory allocations.
     * Any calls to create to allocate a new memry segment will fail.
     * @see create()
     */
    void lock();

    /**
     * Check if memory allocation is locked.
     * @return True if memory allocation is locked.
     */
    bool isLocked() const;

    /**
     * Allow memory allocation.
     * @see create()
     */
    void unlock();


    template<typename T>
    T* construct(MemoryAddress address) {
        return new (address) T();
    }

    template<typename T>
    void destruct(T* t) {
        t->~T();
    }

protected:

    void free(MemoryView* view);

private:

    /** Amount of memeory in bytes allocatable by this manager */
    size_type   _capacity;

    /** Amount of memeory in bytes currently allocated by this manager */
    size_type   _size;

    /** */
    bool        _isLocked;

};

inline void swap(MemoryManager& lhs, MemoryManager& rhs) noexcept {
    lhs.swap(rhs);
}

}  // End of namespace Solace
#endif  // SOLACE_MEMORYMANAGER_HPP
