/*
*  Copyright 2018 Ivan Ryabov
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
 * libSolace: Byte Writer
 *	@file		solace/ByteWriter.hpp
 *	@brief		Write-only adapter for a memory buffer/view.
 ******************************************************************************/
#pragma once
#ifndef SOLACE_BYTEWRITER_HPP
#define SOLACE_BYTEWRITER_HPP

#include "solace/mutableMemoryView.hpp"
#include "solace/memoryResource.hpp"

#include "solace/result.hpp"
#include "solace/error.hpp"


namespace Solace {


/**
 * Write-only adapter for a memory buffer/view.
 */
class ByteWriter {
public:
    using size_type = MemoryResource::size_type;

public:

    /** Construct an empty writer that has nowhere to write too */
    constexpr ByteWriter() noexcept = default;

    ByteWriter(ByteWriter const& other) = delete;
    ByteWriter& operator= (ByteWriter const&) = delete;

    /**
     * Construct the byte buffer by moving content from the other buffer
     * @param other Other buffer to take over from
     */
    constexpr ByteWriter(ByteWriter&& other) noexcept
        : _position{exchange(other._position, 0)}
        , _limit{exchange(other._limit, 0)}
        , _storage{std::move(other._storage)}
    {}

    constexpr ByteWriter(MemoryResource& buffer) noexcept
        : _limit{buffer.size()}
        , _storage{buffer.view(), nullptr}
    {}

    constexpr ByteWriter(MemoryResource&& buffer) noexcept
        : _limit{buffer.size()}
        , _storage{std::move(buffer)}
    {}

    /**
     * Construct the byte buffer from the memory view object
     * @param other Other buffer to copy data from
     */
    constexpr ByteWriter(MutableMemoryView memView) noexcept
        : _limit{memView.size()}
        , _storage{std::move(memView), nullptr}
    {}


    ByteWriter& swap(ByteWriter& rhs) noexcept {
        using std::swap;

        swap(_position, rhs._position);
        swap(_limit, rhs._limit);
        swap(_storage, rhs._storage);

        return *this;
    }

    ByteWriter& operator= (ByteWriter&& rhs) noexcept {
        return swap(rhs);
    }


    /**
     * Leave the limit unchanged and sets the position to zero.
     * It alwats works.
     * @return A reference to this for fluency.
     */
    constexpr ByteWriter& rewind() noexcept {
        _position = 0;

        return *this;
    }


    /** Returns this buffer's capacity.
     * A buffer's capacity is the number of elements it contains.
     * The capacity of a buffer is never negative and never changes.
     * @return Capacity of the buffer in bytes.
     */
    constexpr size_type capacity() const noexcept { return _storage.size(); }

    /**
     * Maximum number of bytes that can be written in the buffer.
     *
     * @note Buffer limit is always less of equal to buffer's capacity.
     * @return Number of bytes that can be read/written to the buffer
     */
    constexpr size_type limit() const noexcept { return _limit; }

    /** Limit number of bytes that can be read/written to this buffer.

     * @note Buffer limit is always less of equal to buffer's capacity.
     * @return A reference to this this buffer.
     *
     * @throws IllegalArgumentException if attempt is made to set limit to more then this buffer capacity.
     */
    Result<void, Error> limit(size_type newLimit) noexcept;

    /**
     * Get remaining number of bytes in the buffer (Up to the limit)
     * @return Remaining number of bytes in the buffer.
     */
    constexpr size_type remaining() const noexcept { return limit() - position(); }

    /**
     * Check if there are bytes left in the buffer (Up to the limit)
     * @return True if there are still some data before the limit is reached.
     */
    constexpr bool hasRemaining() const noexcept { return remaining() > 0; }

    /**
     * Get current position in the buffer.
     * It can be stored to later return to it using @see reset
     * @return Current position in the buffer
     */
    constexpr size_type position() const noexcept { return _position; }

    /**
     * Set current position to the given one.
     *
     * @return Reference to this buffer for fluent interface.
     * @note It is illigal to set position beyond the limit(), and exception will be raised in that case.
     */
    Result<void, Error> position(size_type newPosition) noexcept;

    /**
     * Increment current position by the given amount.
     * @param increment Amount to advance current position by.
     * @return Reference to this buffer for fluent interface.
     * @note It is illigal to advance position beyond the limit(), and exception will be raised in that case.
     */
    Result<void, Error> advance(size_type increment) noexcept;

    /**
     * Set the limit to the capacity and the position to zero.
     */
    constexpr ByteWriter& clear() noexcept {
        _position = 0;
        _limit = capacity();

        return *this;
    }

    /**
     * Set the limit to the current position and then sets the position to zero.
     */
    constexpr ByteWriter& flip() noexcept {
        _limit = _position;
        _position = 0;

        return *this;
    }


    MemoryView viewRemaining() const noexcept {
        return _storage.view().slice(position(), limit());
    }

    MutableMemoryView viewRemaining() noexcept {
        return _storage.view().slice(position(), limit());
    }

    MemoryView viewWritten() const noexcept {
        return _storage.view().slice(0, position());
    }

    MutableMemoryView viewWritten() noexcept {
        return _storage.view().slice(0, position());
    }


    Result<void, Error> write(char value)    noexcept { return write(&value, sizeof(value)); }
    Result<void, Error> write(int8 value)    noexcept { return write(&value, sizeof(value)); }
    Result<void, Error> write(uint8 value)   noexcept { return write(&value, sizeof(value)); }
    Result<void, Error> write(int16 value)   noexcept { return write(&value, sizeof(value)); }
    Result<void, Error> write(uint16 value)  noexcept { return write(&value, sizeof(value)); }
    Result<void, Error> write(int32 value)   noexcept { return write(&value, sizeof(value)); }
    Result<void, Error> write(uint32 value)  noexcept { return write(&value, sizeof(value)); }
    Result<void, Error> write(int64 value)   noexcept { return write(&value, sizeof(value)); }
    Result<void, Error> write(uint64 value)  noexcept { return write(&value, sizeof(value)); }
    Result<void, Error> write(float32 value) noexcept { return write(&value, sizeof(value)); }
    Result<void, Error> write(float64 value) noexcept { return write(&value, sizeof(value)); }

    /**
     * Write given data buffer into this writer.
     * @param data Raw byte data to write.
     * @return Result of write operation. An error is raised if data buffer exceed this writer's capacity.
     */
    Result<void, Error> write(MemoryView data) noexcept {
        return write(data.dataAddress(), data.size());
    }


    // Endianess aware write methods
    Result<void, Error> writeLE(int8 value)  noexcept { return write(&value, sizeof(value)); }
    Result<void, Error> writeLE(uint8 value) noexcept { return write(&value, sizeof(value)); }
    Result<void, Error> writeLE(int16 value) noexcept { return writeLE(static_cast<uint16>(value)); }
    Result<void, Error> writeLE(uint16 value) noexcept;
    Result<void, Error> writeLE(int32 value) noexcept { return writeLE(static_cast<uint32>(value)); }
    Result<void, Error> writeLE(uint32 value) noexcept;
    Result<void, Error> writeLE(int64 value) noexcept { return writeLE(static_cast<uint64>(value)); }
    Result<void, Error> writeLE(uint64 value) noexcept;

    Result<void, Error> writeBE(int8 value)  noexcept { return write(&value, sizeof(value)); }
    Result<void, Error> writeBE(uint8 value) noexcept { return write(&value, sizeof(value)); }
    Result<void, Error> writeBE(int16 value) noexcept { return writeBE(static_cast<uint16>(value)); }
    Result<void, Error> writeBE(uint16 value)noexcept;
    Result<void, Error> writeBE(int32 value) noexcept { return writeBE(static_cast<uint32>(value)); }
    Result<void, Error> writeBE(uint32 value) noexcept;
    Result<void, Error> writeBE(int64 value) noexcept { return writeBE(static_cast<uint64>(value)); }
    Result<void, Error> writeBE(uint64 value) noexcept;

protected:

    Result<void, Error> write(void const* bytes, size_type count) noexcept;

private:

    size_type           _position{};
    size_type           _limit{};

    MemoryResource        _storage;
};


inline void swap(ByteWriter& lhs, ByteWriter& rhs) noexcept {
    lhs.swap(rhs);
}

}  // End of namespace Solace
#endif  // SOLACE_BYTEWRITER_HPP
