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

    using size_type = typename Vector<value_type>::size_type;

    struct Entry {
        key_type    key;
        value_type  value;
    };

public:

    constexpr Dictionary() noexcept = default;

    Dictionary(Vector<Key>&& lookup, Vector<T>&& values)
		: _lookup{mv(lookup)}
		, _values{mv(values)}
    {}

    constexpr auto empty() const noexcept { return _values.empty(); }
    constexpr auto size() const noexcept { return _values.size(); }
    constexpr auto capacity() const noexcept { return _values.capacity(); }

    constexpr Vector<Key> const&    keys() const noexcept   { return _lookup; }
    constexpr Vector<T> const&      values() const noexcept { return _values; }

    bool contains(Key const& key) const noexcept {
        return _lookup.contains(key);
    }
/*
    void put(Key key, T const& value) {
        _values.push_back(value);
        _lookup.puch_back(key);
    }
*/

	Result<T&, Error>
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
	Result<T&, Error>
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


    Optional<T> find(Key const& key) const noexcept {
        size_type i = 0;
        for (auto const& keyHash : _lookup) {
            if (keyHash == key) {
                return _values[i];
            }

            i += 1;
        }

        return none;
    }


private:
    Vector<key_type>        _lookup;
    Vector<value_type>      _values;
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


template <typename K, typename T,
          typename...Args>
[[nodiscard]]
Result<Dictionary<K, T>, Error> makeDictionaryOf(Args&&...args) {
    using size_type = typename Dictionary<K, T>::size_type;
    // Should be relativily safe to cast: we don't expect > 65k arguments
    auto const arraySize = narrow_cast<size_type>(sizeof...(args));
	auto keysBuffer = getSystemHeapMemoryManager().allocate(arraySize*sizeof(K));
	if (!keysBuffer) {
		return keysBuffer.moveError();
	}

	auto valuesBuffer = getSystemHeapMemoryManager().allocate(arraySize*sizeof(T));
	if (!valuesBuffer) {
		return valuesBuffer.moveError();
	}

	auto posKeys = arrayView<K>(keysBuffer.unwrap().view());
	auto posValues = arrayView<T>(valuesBuffer.unwrap().view());

	ArrayExceptionGuard<K> keysGuard{posKeys};
	ArrayExceptionGuard<T> valuesGuard{posValues};

	(keysGuard.emplace(mv(args.key)), ...);
	(valuesGuard.emplace(mv(args.value)), ...);

    keysGuard.release();
    valuesGuard.release();

	return makeDictionary(Vector<K>{keysBuffer.moveResult(), arraySize},
						  Vector<T>{valuesBuffer.moveResult(), arraySize});  // No except c-tor
}


}  // End of namespace Solace
#endif  // SOLACE_DICTIONARY_HPP
