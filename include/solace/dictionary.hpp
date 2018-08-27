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
#include "solace/hashing/fixedSizeDigest.hpp"
#include "solace/utils.hpp"


namespace Solace {

/**
 * Dictionary is a fixed size unordered hash-map.
 *
 */
template<typename T,
         typename Key/* = std::result_of<hash(std::declval<T>())>::type*/>
class Dictionary {
public:

    using value_type = T;
    using key_type = Key;

    using size_type = typename Vector<T>::size_type;

    struct Entry {
        key_type    key;
        value_type  value;
    };

public:

    Dictionary() = default;

    Dictionary(Vector<key_type>&& lookup, Vector<T>&& values)
        : _lookup(std::move(lookup))
        , _values(std::move(values))
    {}

    constexpr bool empty() const noexcept { return _values.empty(); }
    constexpr size_type size() const noexcept { return _values.size(); }
    constexpr size_type capacity() const noexcept { return _values.capacity(); }

    Vector<T>& values() noexcept { return _values; }
    Vector<T> const& values() const noexcept { return _values; }

    bool contains(key_type const& key) const noexcept {
        return _lookup.contains(key);
    }

    void put(key_type key, T const& value) {
        _values.push_back(value);
        _lookup.puch_back(key);
    }

    void put(key_type key, T&& value) {
        _values.emplace_back(std::move(value));
        _lookup.emplace_back(std::move(key));
    }

    template<typename... Args>
    void put(key_type key, Args&&...args) {
        _values.emplace_back(std::forward<Args>(args)...);
        _lookup.emplace_back(std::move(key));
    }


    Optional<T> find(key_type const& key) const noexcept {
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


/**
 * Dictionary factory function
 */
template<typename T, typename K>
Dictionary<T, K> makeDictionary(typename Dictionary<T, K>::size_type size) {
    using DictT = Dictionary<T, K>;

    return {    makeVector<typename DictT::key_type>(size),
                makeVector<typename DictT::value_type>(size)};
}

template<typename T, typename K>
Dictionary<T,K> makeDictionary(std::initializer_list<typename Dictionary<T, K>::Entry> entries) {
//    using DictT = Dictionary<T, K>;

    auto result = makeDictionary<T,K>(entries.size());

    for (auto& i : entries) {
        result.put(std::move(i.key), std::move(i.value));
    }

    return result;
}


}  // End of namespace Solace
#endif  // SOLACE_DICTIONARY_HPP
