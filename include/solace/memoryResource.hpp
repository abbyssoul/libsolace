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
 * libSolace: MemoryResource
 *	@file		solace/memoryResource.hpp
 *	@brief		A resource that represents a chunk of raw system memory.
 ******************************************************************************/
#pragma once
#ifndef SOLACE_MEMORYRESOURCE_HPP
#define SOLACE_MEMORYRESOURCE_HPP

#include "solace/mutableMemoryView.hpp"


namespace Solace {


/* Fixed-length raw memory buffer resource.
 * A resource type that represent raw system memory. It has ownership of the memory allocated
 * and deallocates the buffer when destroyed.
 *
 * @Note: Most methods are noexept.
 */
class MemoryResource {
public:

    using size_type = MutableMemoryView::size_type;


    /**
     * Memory disposer strategy.
     * This class encapsulate a strategy to free previously allocated memory.
     */
    class Disposer {
    public:
        virtual ~Disposer();

        virtual void dispose(MemoryView* view) const = 0;
    };

public:

    /// Non-default destructor release memory owned using disposer.
    ~MemoryResource();


    MemoryResource(MemoryResource const& rhs) = delete;
    MemoryResource& operator= (MemoryResource const& rhs) = delete;

    /** Construct an empty memory buffer */
    constexpr MemoryResource() noexcept = default;

    constexpr MemoryResource(MemoryResource&& rhs) noexcept
        : _data{std::move(rhs._data)}
        , _disposer{exchange(rhs._disposer, nullptr)}
    {
    }

    MemoryResource& operator= (MemoryResource&& rhs) noexcept {
        return swap(rhs);
    }

    /**
     * Construct a memory buffer from a memory view with a given disposer.
     * @param data A memory view this buffer owns.
     * @param disposer A disposer to dispose of the memory when this memory buffer is destroyed.
     */
    constexpr MemoryResource(MutableMemoryView data, Disposer* disposer = nullptr) noexcept
        : _data{std::move(data)}
        , _disposer{disposer}
    {}

    MemoryResource& swap(MemoryResource& rhs) noexcept {
        using std::swap;
        swap(_data, rhs._data);
        swap(_disposer, rhs._disposer);

        return *this;
    }

    constexpr MemoryView          view() const & noexcept   { return _data; }
    constexpr MutableMemoryView   view() & noexcept         { return _data; }

    constexpr bool empty() const noexcept {
        return _data.empty();
    }

    constexpr explicit operator bool() const noexcept {
        return (_data.dataAddress() != nullptr);
    }

    /**
     * Get the size of the memory buffer in bytes.
     * @return The size of the memory buffer in bytes.
     */
    constexpr size_type size() const noexcept { return _data.size(); }

private:

    MutableMemoryView   _data;
    Disposer const*     _disposer {nullptr};
};


inline void
swap(MemoryResource& a, MemoryResource& b) noexcept {
    a.swap(b);
}

}  // End of namespace Solace
#endif  // SOLACE_MEMORYRESOURCE_HPP
