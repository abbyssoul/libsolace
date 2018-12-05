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
 * libSolace: Process runtime environment variables
 *	@file		solace/env.hpp
 *	@author		$LastChangedBy: $
 *	@date		$LastChangedDate: $
 *	@brief		Runtime environment variables
 *	ID:			$Id: $
 ******************************************************************************/
#pragma once
#ifndef SOLACE_PROCESS_ENV
#define SOLACE_PROCESS_ENV


#include "solace/stringView.hpp"
#include "solace/error.hpp"
#include "solace/result.hpp"


namespace Solace {

/**
 * This class incapsulates access to the running process runtime environment variables.
 */
class Env {
public:

    using size_type = uint16;

    /**
     * Struct representing environment variable.
     */
    struct Var {
        StringView name;
        StringView value;
    };

    /**
     * Environment variables iterator
     */
    struct Iterator {

        constexpr Iterator(size_type size, size_type position) noexcept
            : _index(position)
            , _size(size)
        {}

        constexpr Iterator(Iterator const& rhs) noexcept = default;

        constexpr Iterator(Iterator&& rhs) noexcept
            : _index(rhs._index)
            , _size(rhs._size)
        {}

        Iterator& operator= (Iterator&& rhs) noexcept {
            return swap(rhs);
        }

        constexpr bool operator!= (const Iterator& other) const noexcept {
            return (_index != other._index);
        }

        constexpr bool operator== (const Iterator& other) const noexcept {
            return (_index == other._index);
        }

        Iterator& operator++ ();

        Var operator* () const {
            return operator ->();
        }

        Var operator-> () const;

        Iterator& swap(Iterator& rhs) noexcept {
            std::swap(_index, rhs._index);
            std::swap(_size, rhs._size);

            return *this;
        }

        constexpr size_type getIndex() const noexcept { return _index; }

    private:
        size_type _index;
        size_type _size;
    };

    using const_iterator = const Iterator;

public:

    /**
     * Get a value of the environment variable if one exists.
     *
     * @param name Name of the variable to get value of.
     * @return Value of the variable or None if no variable is set.
     */
    Optional<StringView> get(StringView name) const noexcept;

    /**
     * Set a value of the environment variable.
     *
     * @param name Name of the variable to set value of.
     * @param value Value of the variable to set to.
     * @param replace If true and variable already set, the value will be replaced. If false old value is preserved.
     */
    Result<void, Error> set(StringView name, StringView value, bool replace = true) noexcept;

    /**
     * Un-Set a value of the given environment variable.
     *
     * @param name Name of the variable to get value of.
     * @return Result of the unset operation.
     */
    Result<void, Error> unset(StringView name) noexcept;

    /**
     * Clear current environment.
     *
     * @return Result of the unset operation.
     */
    Result<void, Error> clear() noexcept;

    /**
     * Check if there are no environment variables.
     *
     * @return True is this is an empty collection.
     */
    bool empty() const noexcept {
        return (size() == 0);
    }


    //-----------------------------------------------------------------------------------
    // Iterable trait
    //-----------------------------------------------------------------------------------

    /**
     * Get the number of elements in this array
     * @return The size of this finite collection
     */
    size_type size() const noexcept;

    const_iterator begin() const noexcept {
        return Iterator(size(), 0);
    }

    const_iterator end() const noexcept {
        auto const pos = size();
        return Iterator(pos, pos);
    }


    template<typename F>
    std::enable_if_t<isCallable<F, const Var>::value, const Env& >
    forEach(F&& f) const {
        for (auto x : *this) {
            f(x);
        }

        return *this;
    }

    template<typename F>
    std::enable_if_t<isCallable<F, const StringView, const StringView>::value, const Env& >
    forEach(F&& f) const {
        for (auto x : *this) {
            f(x.name, x.value);
        }

        return *this;
    }


    template<typename F>
    std::enable_if_t<isCallable<F, size_type, const Var>::value, const Env& >
    forEach(F&& f) const {
        for (auto i = begin(), ends = end(); i != ends; ++i) {
            f(i.getIndex(), *i);
        }

        return *this;
    }
};



}  // End of namespace Solace
#endif  // SOLACE_PROCESS_ENV
