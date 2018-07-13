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
 * libSolace: MemoryView
 *	@file		solace/memoryBuffer.hpp
 *	@brief		Definition of a MemoryBuffer object
 ******************************************************************************/
#pragma once
#ifndef SOLACE_MEMORYBUFFER_HPP
#define SOLACE_MEMORYBUFFER_HPP

#include "solace/memoryView.hpp"


namespace Solace {


/**
 * Memory disposer strategy.
 * This class encapsulate a strategy to free previously allocated memory.
 */
class MemoryViewDisposer {
public:
    virtual ~MemoryViewDisposer();

    virtual void dispose(ImmutableMemoryView* view) const = 0;
};


/* Fixed-length raw memory buffer.
 * Note: Unlike MemoryView - MemoryBuffer owns the memory and deallocates is when destroyed.
 */
class MemoryBuffer {
public:
    using size_type = MemoryView::size_type;

public:

    ~MemoryBuffer();

    /** Construct an empty memory buffer */
    MemoryBuffer() noexcept = default;

    /**
     * Construct a memory buffer from a memory view with a given disposer.
     * @param data A memory view this buffer owns.
     * @param disposer A disposer to dispose of the memory when this memory buffer is destroyed.
     */
    MemoryBuffer(MemoryView data, MemoryViewDisposer* disposer = nullptr) noexcept :
        _data(std::move(data)),
        _disposer(disposer)
    {}

    MemoryBuffer(MemoryBuffer&& rhs) noexcept :
        _data(std::move(rhs._data)),
        _disposer(std::exchange(rhs._disposer, nullptr))
    {
    }

    MemoryBuffer& operator= (MemoryBuffer&& rhs) {
        return swap(rhs);
    }

    MemoryBuffer(MemoryBuffer const& rhs) = delete;
    MemoryBuffer& operator= (MemoryBuffer const& rhs) = delete;

    MemoryBuffer& swap(MemoryBuffer& rhs) noexcept {
        _data.swap(rhs._data);
        std::swap(_disposer, rhs._disposer);

        return *this;
    }

    MemoryView&         view() noexcept         { return _data; }
    MemoryView const&   view() const noexcept   { return _data; }

    bool empty() const noexcept {
        return _data.empty();
    }

    explicit operator bool() const noexcept {
        return (_data.dataAddress() != nullptr);
    }

    /**
     * @return The size of this finite collection
     */
    size_type size() const noexcept { return _data.size(); }

private:

    MemoryView                  _data;
    MemoryViewDisposer const*   _disposer {nullptr};
};

}  // End of namespace Solace
#endif  // SOLACE_MEMORYBUFFER_HPP
