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
 * libSolace: Byte Reader
 *	@file		solace/byteReader.hpp
 *	@brief		Read-only adapter for a memory buffer/view.
 ******************************************************************************/
#pragma once
#ifndef SOLACE_BYTEREADER_HPP
#define SOLACE_BYTEREADER_HPP

#include "solace/memoryView.hpp"
#include "solace/mutableMemoryView.hpp"     // read destination
#include "solace/memoryResource.hpp"

#include "solace/result.hpp"
#include "solace/error.hpp"


namespace Solace {

/**
 * A byte buffer with stream access semantic.
 * This is an adapter on top of Memory view (@see ImmutableMemoryView) to enable easy reading of data.
 *
 */
class ByteReader {
public:
    using size_type = MemoryResource::size_type;

public:

    /** Construct an empty buffer of size zero */
    constexpr ByteReader() noexcept = default;


    ByteReader(ByteReader const& other) = delete;
    ByteReader& operator= (ByteReader const&) = delete;


    /**
     * Construct the byte buffer by moving content from the other buffer
     * @param other Other buffer to take over from
     */
	/*gcc7.2 bug: constexpr*/ ByteReader(ByteReader&& other) noexcept
        : _position{exchange(other._position, 0)}
        , _limit{exchange(other._limit, 0)}
		, _storage{mv(other._storage)}
    {}


    /**
     * Construct the read buffer from an existing memory buffer object.
     * Note: The caller of this method is responsible for the life-time of the memory buffer.
     * Use move constructor if you want to delegate life-time management to the instance of the reader.
     * @param other Other buffer to copy data from
     */
	/*gcc7.2 bug: constexpr*/ ByteReader(MemoryResource& buffer) noexcept
        : _limit{buffer.size()}
        , _storage{buffer.view(), nullptr}
    {
    }

    /**
     * Construct the byte buffer from the memory buffer object
     * @param other Other buffer to copy data from
     */
	/*gcc7.2 bug: constexpr*/ ByteReader(MemoryResource&& buffer) noexcept
        : _limit{buffer.size()}
		, _storage{mv(buffer)}
    {
    }

    /**
     * Construct the byte buffer from the memory view object
     * @param other Other buffer to copy data from
     */
    ByteReader(MemoryView view) noexcept
        : _limit{view.size()}
		, _storage{mutable_cast(view)}
    {
    }


    ByteReader& swap(ByteReader& rhs) noexcept {
        using std::swap;

        swap(_position, rhs._position);
        swap(_limit, rhs._limit);
        swap(_storage, rhs._storage);

        return *this;
    }


    ByteReader& operator= (ByteReader&& rhs) noexcept {
        return swap(rhs);
    }


    /**
     * Leave the limit unchanged and sets the position to zero.
     * @return A reference to this for fluency.
     */
    constexpr ByteReader& rewind() noexcept {
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
     * Number of bytes available to read from this buffer.
     *
     * @note Buffer limit is always less of equal to buffer's capacity.
     * @return Number of bytes that can be read/written to the buffer
     */
    constexpr size_type limit() const noexcept { return _limit; }

	/** Set the limit to the number of bytes that can be read from the buffer.
	 *
	 * @param newLimit Maximum number of bypes that can be written.
	 * @note Buffer limit is always less of equal to buffer's capacity.
	 * @return Result void or an error if operation failed.
     *
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
     * Set position back to the previously saved mark
     * @return Reference to this buffer.
     */
    Result<void, Error>
    reset(size_type savedMark) noexcept {
        return position(savedMark);
    }

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
     * Read a single byte from the buffer
     * @return One byte read from the buffer
     */
    Result<byte, Error> get() noexcept;

    /**
     * Get a single byte from the buffer in the given position
     * @return One byte read from the buffer
     * @note This operation does not advance current position
     */
    Result<byte, Error> get(size_type position) const noexcept;

    /**
     * Read data form this buffer and store it into a given destantion.
     * @param dest Buffer to store read data into.
     * @return Nothing if successfull or an error.
     */
    Result<void, Error>
    read(MutableMemoryView dest) noexcept {
        return read(dest, dest.size());
    }

    Result<void, Error>
    read(MutableMemoryView dest, size_type bytesToRead) noexcept;

    Result<void, Error>
    read(size_type offset, MutableMemoryView dest, size_type bytesToRead) const noexcept;

    Result<void, Error>
    read(size_type offset, MutableMemoryView dest) const noexcept {
        return read(offset, dest, dest.size());
    }

    MemoryView viewRemaining() const noexcept {
        return _storage.view().slice(position(), limit());
    }

    Result<void, Error>  read(char*      dest) noexcept { return read(dest, sizeof(char));    }
    Result<void, Error>  read(int8*      dest) noexcept { return read(dest, sizeof(int8));    }
    Result<void, Error>  read(uint8*     dest) noexcept { return read(dest, sizeof(uint8));   }
    Result<void, Error>  read(int16*     dest) noexcept { return read(dest, sizeof(int16));   }
    Result<void, Error>  read(uint16*    dest) noexcept { return read(dest, sizeof(uint16));  }
    Result<void, Error>  read(int32*     dest) noexcept { return read(dest, sizeof(int32));   }
    Result<void, Error>  read(uint32*    dest) noexcept { return read(dest, sizeof(uint32));  }
    Result<void, Error>  read(int64*     dest) noexcept { return read(dest, sizeof(int64));   }
    Result<void, Error>  read(uint64*    dest) noexcept { return read(dest, sizeof(uint64));  }
    Result<void, Error>  read(float32*   dest) noexcept { return read(dest, sizeof(float32)); }
    Result<void, Error>  read(float64*   dest) noexcept { return read(dest, sizeof(float64)); }

    // Endianess aware read/write
    Result<void, Error>  readLE(int8& value)  noexcept { return read(&value, sizeof(int8)); }
    Result<void, Error>  readLE(uint8& value) noexcept { return read(&value, sizeof(uint8)); }
    Result<void, Error>  readLE(int16& value) noexcept { return readLE(reinterpret_cast<uint16&>(value)); }
    Result<void, Error>  readLE(uint16& value)noexcept;
    Result<void, Error>  readLE(int32& value) noexcept { return readLE(reinterpret_cast<uint32&>(value)); }
    Result<void, Error>  readLE(uint32& value)noexcept;
    Result<void, Error>  readLE(int64& value) noexcept { return readLE(reinterpret_cast<uint64&>(value)); }
    Result<void, Error>  readLE(uint64& value)noexcept;

    Result<void, Error>  readBE(int8& value)  noexcept { return read(&value, sizeof(int8)); }
    Result<void, Error>  readBE(uint8& value) noexcept { return read(&value, sizeof(uint8)); }
    Result<void, Error>  readBE(int16& value) noexcept { return readBE(reinterpret_cast<uint16&>(value)); }
    Result<void, Error>  readBE(uint16& value)noexcept;
    Result<void, Error>  readBE(int32& value) noexcept { return readBE(reinterpret_cast<uint32&>(value)); }
    Result<void, Error>  readBE(uint32& value)noexcept;
    Result<void, Error>  readBE(int64& value) noexcept { return readBE(reinterpret_cast<uint64&>(value)); }
    Result<void, Error>  readBE(uint64& value)noexcept;

protected:
    Result<void, Error>  read(void* dest, size_type count) noexcept;

protected:

    size_type           _position{};
    size_type           _limit{};

    MemoryResource      _storage;

};


inline
void swap(ByteReader& lhs, ByteReader& rhs) noexcept {
    lhs.swap(rhs);
}

}  // End of namespace Solace
#endif  // SOLACE_BYTEREADER_HPP
