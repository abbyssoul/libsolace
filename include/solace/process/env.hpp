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
 *	@file		solace/process/env.hpp
 *	@author		$LastChangedBy: $
 *	@date		$LastChangedDate: $
 *	@brief		Runtime environment variables
 *	ID:			$Id: $
 ******************************************************************************/
#pragma once
#ifndef SOLACE_PROCESS_ENV
#define SOLACE_PROCESS_ENV

#include "solace/string.hpp"
#include "solace/iterable.hpp"


namespace Solace { namespace Process {

/**
 * This class incapsulates access to runtime environment variable for the process.
 */
class Env : public Iterable<Env, String> {
public:

    typedef size_t size_type;

    struct Var {
        String name;
        String value;

        Var(const String& inName, const String& inValue): name(inName), value(inValue)
        {}

        Var(Var&& other): name(std::move(other.name)), value(std::move(other.value))
        {}

        Var& swap(Var& other) noexcept {
            std::swap(name, other.name);
            std::swap(value, other.value);

            return *this;
        }

        Var& operator= (Var&& rhs) noexcept {
            return swap(rhs);
        }
    };

    /**
     * Environment variables iterator
     */
    class Iterator {
    public:
        bool operator!= (const Iterator& other) const {
            return (_index != other._index);
        }

        bool operator== (const Iterator& other) const {
            return (_index == other._index);
        }

        Iterator& operator++ ();

        Var operator* () const {
            return this->operator ->();
        }

        Var operator-> () const;

        Iterator& swap(Iterator& rhs) noexcept {
            std::swap(_index, rhs._index);
            std::swap(_size, rhs._size);

            return *this;
        }

        Iterator(size_type size, size_type position);

        Iterator(const Iterator& rhs):
            _index(rhs._index),
            _size(rhs._size)
        {}

        Iterator(Iterator&& rhs):
            _index(rhs._index),
            _size(rhs._size)
        {}

        Iterator& operator= (Iterator&& rhs) noexcept {
            return swap(rhs);
        }

    private:
        size_type _index;
        size_type _size;
    };

    typedef const Iterator const_iterator;

public:

    /**
     * Construct an environment access class
     */
    Env();

    /**
     * Get a value of the environment variable if one exists.
     *
     * @param name Name of the variable to get value of.
     * @return Value of the variable or None if no variable is set.
     */
    Optional<String> get(const String& name) const;

    /**
     * Set a value of the environment variable.
     *
     * @param name Name of the variable to set value of.
     * @param value Value of the variable to set to.
     * @param replace If true and variable already set - the value will be replace. If false old value is preserved.
     *
     * FIXME(abbyssoul): This operation can fail according to specs. Use Result<>
     */
    void set(const String& name, const String& value, bool replace = true);

    /**
     * Set a value of the environment variable.
     *
     * @param name Name of the variable to get value of.
     * @return Value of the variable or None if no variable is set.
     *
     * FIXME(abbyssoul): This operation can fail according to specs. Use Result<>
     */
    void unset(const String& name);

    /**
     * Clear current environment.
     *
     * FIXME(abbyssoul): This operation can fail according to specs. Use Result<>
     */
    void clear();

    //-----------------------------------------------------------------------------------
    // Iterable interface goes next
    //-----------------------------------------------------------------------------------

    /**
     * Check if this collection is empty.
     * @return True is this is an empty collection.
     */
    bool empty() const noexcept {
        return (size() == 0);
    }

    /**
     * Get the number of elements in this array
     * @return The size of this finite collection
     */
    size_type size() const noexcept;

    String operator[] (const String& name) const;

    const_iterator begin() const;

    const_iterator end() const;

    const Env& forEach(const std::function<void(const String&)> &f) const override;

};

}  // End of namespace Process
}  // End of namespace Solace
#endif  // SOLACE_PROCESS_ENV
