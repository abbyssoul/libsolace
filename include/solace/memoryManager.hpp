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
 *	@brief		Platform memory manager interface
 ******************************************************************************/
#pragma once
#ifndef SOLACE_MEMORYMANAGER_HPP
#define SOLACE_MEMORYMANAGER_HPP

#include "solace/memoryResource.hpp"
#include "solace/result.hpp"
#include "solace/error.hpp"


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
	using size_type = MemoryView::size_type;
	using value_type = MemoryView::value_type;

	using MemoryAddress = MemoryView::MutableMemoryAddress;

public:

    /** Destruct memory manager
     * Note: All the memory allocated by the memory manager instance must be freed by the time
     * when the manager is destroyed because allocated memory references this instance.
     */
    virtual ~MemoryManager() = default;

    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator= (MemoryManager const& rhs) = delete;

    /** Construct a new memory manager with the given capacity
     *
     * @param allowedCapacity The memory capacity this manager allowed to allocate.
     */
    explicit MemoryManager(size_type allowedCapacity);

    MemoryManager(MemoryManager&&) noexcept = default;
    MemoryManager& operator= (MemoryManager&& rhs) noexcept {
        return swap(rhs);
    }

    MemoryManager& swap(MemoryManager& rhs) noexcept;

    /**
     * Check if this memory manager has no allocated memory.
     * @return True if no memory is allocated by this manager.
     */
    constexpr bool empty() const noexcept {
        return (_size == 0);
    }

    /** Get amount of memory in bytes allocated by the memory manager.
     * @return Total amount of memory allocated by this manager.
     */
    constexpr size_type size() const noexcept {
        return _size;
    }

    /**
     * @return Total amount of memory manager is allowed to allocate.
     */
    constexpr size_type capacity() const noexcept {
        return _capacity;
    }

    /**
     * @return Maxumum number of bytes that can be allocated
     */
    constexpr size_type limit() const noexcept {
        return _capacity - _size;
    }

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
	 * @param nbBytes The size of the memory segment in bytes to allocate.
     * @return A newly allocated memory segment.
     * TODO(abbyssoul): should return Result<>
     */
    [[nodiscard]]
	Result<MemoryResource, Error> allocate(size_type nbBytes) noexcept;

    /**
     * Prohibit memory allocations.
     * Any calls to create to allocate a new memry segment will fail.
     * @see create()
     *
     * FIXME(abbyssoul): Lock must produce lock object
     */
    void lock();

    /**
     * Check if memory allocation is locked.
     * @return True if memory allocation is locked.
     */
	bool isLocked() const noexcept;

    /**
     * Allow memory allocation.
     * @see create()
     */
    void unlock();

protected:

    /**
     * A specialization of memory dispozer used by memory buffer to deallocate memory allocated via this manager.
     */
    class HeapMemoryDisposer : public MemoryResource::Disposer {
    public:
        HeapMemoryDisposer(MemoryManager& self) : _self(&self)
        {}

        void dispose(MemoryView* view) const override;

    private:
        MemoryManager* _self;
    };

    friend class HeapMemoryDisposer;

    void free(MemoryView* view);

private:

    /** Amount of memeory in bytes allocatable by this manager */
    size_type   _capacity;

    /** Amount of memeory in bytes currently allocated by this manager */
    size_type   _size;

    /** */
    bool        _isLocked;

    HeapMemoryDisposer _disposer;

};


inline void swap(MemoryManager& lhs, MemoryManager& rhs) noexcept {
    lhs.swap(rhs);
}

/**
 * Return global system memory manager.
 * @return Heap memory manager
 */
MemoryManager& getSystemHeapMemoryManager();

}  // End of namespace Solace
#endif  // SOLACE_MEMORYMANAGER_HPP
