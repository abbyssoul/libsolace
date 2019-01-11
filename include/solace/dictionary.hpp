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
        : _lookup(std::move(lookup))
        , _values(std::move(values))
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

    void put(Key key, T&& value) {
        _values.emplace_back(std::move(value));
        _lookup.emplace_back(std::move(key));
    }

    template<typename... Args>
    void put(Key key, Args&&...args) {
        _values.emplace_back(std::forward<Args>(args)...);
        _lookup.emplace_back(std::move(key));
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
template<typename K, typename T>
[[nodiscard]]
constexpr Dictionary<K, T> makeDictionary() noexcept {
    return {};
}


/**
 * Create a new dictionary with a given memory resources.
 * Capacity of the resulting container is determined by the size of the resource.
 * @return A newly constructed empty dictionary.
 */
template<typename K, typename T>
[[nodiscard]]
Dictionary<K, T> makeDictionary(MemoryResource&& keysMem, MemoryResource&& valuesMem) noexcept {
    using DictT = Dictionary<K, T>;

    return {makeVector<typename DictT::key_type>(std::move(keysMem)),
            makeVector<typename DictT::value_type>(std::move(valuesMem))};
}

/**
 * Create a new Dictionary object with a given capacity.
 * @param size Desired dictionary capacity.
 * @return A new Dictionary instance.
 */
template<typename K, typename T>
[[nodiscard]]
Dictionary<K, T> makeDictionary(typename Dictionary<K, T>::size_type size) {
    using DictT = Dictionary<K, T>;

    return {    makeVector<typename DictT::key_type>(size),
                makeVector<typename DictT::value_type>(size)};
}


template <typename K, typename T,
          typename...Args>
[[nodiscard]]
Dictionary<K, T> makeDictionaryOf(Args&&...args) {
    using size_type = typename Dictionary<K, T>::size_type;
    // Should be relativily safe to cast: we don't expect > 65k arguments
    auto const arraySize = narrow_cast<size_type>(sizeof...(args));
    auto keysBuffer   = getSystemHeapMemoryManager().allocate(arraySize*sizeof(K));           // May throw
    auto valuesBuffer = getSystemHeapMemoryManager().allocate(arraySize*sizeof(T));           // May throw

    auto posKeys = arrayView<K>(keysBuffer.view());
    auto posValues = arrayView<T>(valuesBuffer.view());

    ArrayExceptionGuard<K> keysGuard(posKeys);
    ArrayExceptionGuard<T> valuesGuard(posValues);

    (keysGuard.emplace(std::move(args.key)), ...);
    (valuesGuard.emplace(std::move(args.value)), ...);

    keysGuard.release();
    valuesGuard.release();

    return {{std::move(keysBuffer),     arraySize},
            {std::move(valuesBuffer),   arraySize}};  // No except c-tor
}


}  // End of namespace Solace
#endif  // SOLACE_DICTIONARY_HPP
