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
 * libSolace: A OOP view for a heterogenious collection of variable sized elements
 *	@file		solace/variableSpan.hpp
 ******************************************************************************/
#pragma once
#ifndef SOLACE_VARIABLESPAN_HPP
#define SOLACE_VARIABLESPAN_HPP

#include "solace/types.hpp"
#include "solace/memoryView.hpp"
#include "solace/byteReader.hpp"


namespace Solace {

enum class EncoderType {
	Natural,
	LittleEndian,
	BigEndian
};

template<typename DatumSizeType, EncoderType E>
struct TypedChunkReader;


template<typename DatumSizeType, EncoderType E>
struct TypedChunkReader {
	static MemoryView readChunk(MemoryView src, DatumSizeType& chunkSize) {
		ByteReader reader{src};
		reader.read(&chunkSize);

		return reader.viewRemaining();
	}
};


template<typename DatumSizeType>
struct TypedChunkReader<DatumSizeType, EncoderType::BigEndian> {
	static MemoryView readChunk(MemoryView src, DatumSizeType& chunkSize) {
		ByteReader reader{src};
		reader.readBE(&chunkSize);

		return reader.viewRemaining();
	}
};

template<typename DatumSizeType>
struct TypedChunkReader<DatumSizeType, EncoderType::LittleEndian> {
	static MemoryView readChunk(MemoryView src, DatumSizeType& chunkSize) {
		ByteReader reader{src};
		reader.readLE(&chunkSize);

		return reader.viewRemaining();
	}
};




template<typename T,
		 typename DatumSizeType = uint16,
		 EncoderType E = EncoderType::Natural>
struct VariableSpan {
	static_assert (std::is_constructible_v<T, MemoryView>, "Type must be contructable from MemoryView");

	using value_type = T;
	using size_type = uint16;
	using datum_size = DatumSizeType;

	using reference = T&;
	using const_reference = T const&;

	using pointer_type = T*;
	using const_pointer = T const*;

	struct Iterator {

		Iterator(size_type nElements, MemoryView dataView)
			: _nElements{nElements}
			, _data{dataView}
		{}

		constexpr Iterator(Iterator const& rhs) noexcept = default;
		constexpr Iterator(Iterator&& rhs) noexcept = default;

		Iterator& operator= (Iterator const& rhs) noexcept = default;
		Iterator& operator= (Iterator&& rhs) noexcept {
			return swap(rhs);
		}

		constexpr bool operator!= (Iterator const& other) const noexcept {
			return !(operator==(other));
		}

		constexpr bool operator== (Iterator const& other) const noexcept {
			return (_nElements == other._nElements);
//			return (_data.dataAddress() == other._data.dataAddress()) && (_nElements == other._nElements);
		}

		Iterator& operator++ () {
			if (0 == _nElements)
				return *this; // No where to go

			datum_size datumSize = 0;

			auto tail = TypedChunkReader<datum_size, E>::readChunk(_data, datumSize);
			_data = tail.slice(datumSize, tail.size());  // skip datumSize
			_nElements -= 1;

			return *this;
		}

		value_type operator-> () const {
			assertTrue(0 < _nElements, "Index out of bounds");

			datum_size datumSize = 0;

			auto tail = TypedChunkReader<datum_size, E>::readChunk(_data, datumSize);
			auto data = tail.slice(0, datumSize);

			return value_type{data};
		}

		value_type operator* () const {
			return operator ->();
		}

	private:
		size_type	_nElements;
		MemoryView	_data;
	};

	using const_iterator = Iterator const;

	constexpr VariableSpan() noexcept = default;

	VariableSpan(size_type nElements, MemoryView data) noexcept
		: _nElements{nElements}
		, _data{mv(data)}
	{}

	/**
	 * Check if this collection is empty.
	 * @return True is this is an empty collection.
	 */
	constexpr bool empty() const noexcept {
		return (_nElements == 0);
	}

	/**
	 * Get the number of elements in this collection.
	 * @return number of elements in this collection.
	 */
	constexpr size_type size() const noexcept {
		return _nElements;
	}

	Iterator begin() noexcept {
		return Iterator{_nElements, _data};
	}

	Iterator end() noexcept {
		return Iterator{0, _data};
	}

	const_iterator begin() const noexcept {
		return Iterator{_nElements, _data};
	}

	const_iterator end() const noexcept {
		return Iterator{0, _data};
	}


private:

	/// Number of elements in the span
	size_type	_nElements;

	/// Start of the data
	MemoryView	_data;

};



}  // namespace Solace

#endif  // SOLACE_VARIABLESPAN_HPP
