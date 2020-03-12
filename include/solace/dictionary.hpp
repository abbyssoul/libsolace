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
 * libSolace:
 *  @brief		Fixed size dictionary container
 *	@file		solace/dictionary.hpp
 ******************************************************************************/
#pragma once
#ifndef SOLACE_DICTIONARY_HPP
#define SOLACE_DICTIONARY_HPP

#include "solace/vector.hpp"
#include "solace/utils.hpp"


namespace Solace {

/**
 * Dictionary is a fixed size unordered map.
 *
 */
template<typename Key,
         typename T>
class Dictionary {
public:

    using value_type = T;
    using key_type = Key;

	using KeySet = Vector<key_type>;
	using ValueSet = Vector<value_type>;

	using KeyRef = typename KeySet::reference;
	using ValueRef = typename ValueSet::reference;

	using KeyConstRef = typename KeySet::const_reference;
	using ValueConstRef = typename ValueSet::const_reference;

	using KeysIterator   = typename KeySet::Iterator;
	using ValuesIterator = typename ValueSet::Iterator;

	using KeysConstIterator   = typename KeySet::const_iterator;
	using ValuesConstIterator = typename ValueSet::const_iterator;

	using size_type = typename ValueSet::size_type;

	struct Entry {
		key_type	key;
		value_type	value;
	};

	struct EntryConstRef {
		KeyConstRef		key;
		ValueConstRef	value;
    };

	struct EntryRef {
		KeyRef		key;
		ValueRef	value;
	};


	template<typename KI, typename VI>
	struct Iterator_base {

		constexpr Iterator_base(Iterator_base const& rhs) noexcept = default;
		constexpr Iterator_base(Iterator_base&& rhs) noexcept = default;

		constexpr Iterator_base(KI keyIt, VI valueIt) noexcept
			: _keyIt{mv(keyIt)}
			, _valueIt{mv(valueIt)}
		{
		}

		Iterator_base& operator= (Iterator_base const& rhs) noexcept = default;
		Iterator_base& operator= (Iterator_base&& rhs) noexcept {
			return swap(rhs);
		}

		constexpr bool operator!= (Iterator_base const& other) const noexcept {
			return (_keyIt != other._keyIt) || (_valueIt != other._valueIt);
		}

		constexpr bool operator== (Iterator_base const& other) const noexcept {
			return (_keyIt == other._keyIt) && (_valueIt == other._valueIt);
		}

		EntryConstRef operator* () const { return {*_keyIt, *_valueIt}; }

		Iterator_base& swap(Iterator_base& rhs) noexcept {
			std::swap(_keyIt, rhs._keyIt);
			std::swap(_valueIt, rhs._valueIt);

			return *this;
		}

		Iterator_base& operator++ () {
			++_keyIt;
			++_valueIt;

			return *this;
		}

	private:
		KI	_keyIt;
		VI _valueIt;
	};

	using Iterator = Iterator_base<KeysIterator, ValuesIterator>;
	using const_iterator = Iterator_base<KeysConstIterator, ValuesConstIterator>;

public:

    constexpr Dictionary() noexcept = default;

	Dictionary(Vector<Key>&& lookup, Vector<T>&& values) noexcept
		: _lookup{mv(lookup)}
		, _values{mv(values)}
    {}

    constexpr auto empty() const noexcept { return _values.empty(); }
    constexpr auto size() const noexcept { return _values.size(); }
    constexpr auto capacity() const noexcept { return _values.capacity(); }

	constexpr KeySet   const& keys()   const noexcept { return _lookup; }
	constexpr ValueSet const& values() const noexcept { return _values; }

    bool contains(Key const& key) const noexcept {
        return _lookup.contains(key);
    }

	Result<ValueRef, Error>
	put(Key key, T&& value) {
		auto maybeValue = _values.emplace_back(mv(value));
		if (!maybeValue) {
			return maybeValue.moveError();
		}

		auto maybeKey = _lookup.emplace_back(mv(key));
		if (!maybeKey) {
			_values.pop_back();
			return maybeKey.moveError();
		}

		return maybeValue;
	}

    template<typename... Args>
	Result<ValueRef, Error>
	put(Key key, Args&&...args) {
		auto maybeValue = _values.emplace_back(fwd<Args>(args)...);
		if (!maybeValue) {
			return maybeValue.moveError();
		}

		auto maybeKey = _lookup.emplace_back(mv(key));
		if (!maybeKey) {
			_values.pop_back();
			return maybeKey.moveError();
		}

		return maybeValue;
	}


	Optional<ValueRef> find(Key const& key) noexcept {
		auto valueIt = _values.begin();
		for (auto const& keyHash : _lookup) {
			// Maybe check for `valueIt != _values.end()` for sanity.
			if (keyHash == key) {
				return *valueIt;
			}
			++valueIt;
		}

		return none;
	}


	Optional<ValueConstRef> find(Key const& key) const noexcept {
		auto valueIt = _values.begin();
        for (auto const& keyHash : _lookup) {
			// Maybe check for `valueIt != _values.end()` for sanity.
            if (keyHash == key) {
				return *valueIt;
            }
			++valueIt;
        }

        return none;
    }

	Iterator begin() noexcept { return {_lookup.begin(), _values.begin()}; }
	Iterator end() noexcept { return {_lookup.end(), _values.end()}; }

	const_iterator begin() const noexcept { return {_lookup.begin(), _values.begin()}; }
	const_iterator end() const noexcept { return {_lookup.end(), _values.end()}; }

private:
	KeySet		_lookup;
	ValueSet	_values;
};



/// Create an empty zero sized dictionary
template<typename K, typename V>
[[nodiscard]]
constexpr Dictionary<K, V> makeDictionary() noexcept {
    return {};
}

template<typename K, typename V>
[[nodiscard]]
Result<Dictionary<K, V>, Error> makeDictionary(Vector<K>&& keys, Vector<V>&& values) noexcept {
	return {types::okTag, in_place, mv(keys), mv(values)};
}

/**
 * Create a new dictionary with a given memory resources.
 * Capacity of the resulting container is determined by the size of the resource.
 * @return A newly constructed empty dictionary.
 */
template<typename K, typename V>
[[nodiscard]]
Result<Dictionary<K, V>, Error> makeDictionary(MemoryResource&& keysMem, MemoryResource&& valuesMem) noexcept {
	using DictT = Dictionary<K, V>;

	return makeDictionary(makeVector<typename DictT::key_type>(mv(keysMem)),
						  makeVector<typename DictT::value_type>(mv(valuesMem)));
}

/**
 * Create a new Dictionary object with a given capacity.
 * @param size Desired dictionary capacity.
 * @return A new Dictionary instance.
 */
template<typename K, typename T>
[[nodiscard]]
Result<Dictionary<K, T>, Error> makeDictionary(typename Dictionary<K, T>::size_type size) {
    using DictT = Dictionary<K, T>;

	auto keys = makeVector<typename DictT::key_type>(size);
	if (!keys) {
		return keys.moveError();
	}

	auto values = makeVector<typename DictT::value_type>(size);
	if (!values) {
		return values.moveError();
	}

	return makeDictionary(keys.moveResult(), values.moveResult());
}

}  // End of namespace Solace
#endif  // SOLACE_DICTIONARY_HPP
