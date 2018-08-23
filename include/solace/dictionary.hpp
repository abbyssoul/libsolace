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

#include "solace/assert.hpp"
#include "solace/arrayView.hpp"
#include "solace/mutableMemoryView.hpp"
#include "solace/memoryManager.hpp"  // TODO(abbyssoul): Allocate memory via memory manager

#include "solace/traits/callable.hpp"


namespace Solace {

//FixedSizeDigest

/**
 * Dictionary is a fixed size unordered hash-map.
 *
 */
template<class T>
class Dictionary {
public:

    using size_type = typename Vector<T>::size_type;
    using HashType = uint32; //std::result_of<hash(std::declval<T>)>::type;

public:

    constexpr bool empty() const noexcept { return _values.empty(); }
    constexpr size_type size() const noexcept { return _values.size(); }

    Vector<T>& values() noexcept { return _values; }
    Vector<T> const& values() const noexcept { return _values; }

    void put(HashType key, T const& value);
    void put(HashType key, T&& value);

    Optional<T> find(HashType key) const noexcept {
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
    Vector<HashType>    _lookup;
    Vector<T>           _values;
};


/**
 * Dictionary factory function
 */
template<typename T>
Dictionary<T> makeDictionary(typename Dictionary<T>::size_type size);

template<typename T>
Dictionary<T> makeDictionary(std::initializer_list<typename Dictionary<T>::Entry> entries);

}  // End of namespace Solace
#endif  // SOLACE_DICTIONARY_HPP
