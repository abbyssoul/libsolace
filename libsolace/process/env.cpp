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
 * libSolace
 *	@file		process/env.cpp
 *	@author		$LastChangedBy: soultaker $
 *	@date		$LastChangedDate$
 *	@brief		Implementation of env class
 *	ID:			$Id$
 ******************************************************************************/
#include "solace/process/env.hpp"
#include "solace/exception.hpp"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

using namespace Solace;
using namespace Solace::Process;


Env::Iterator::Iterator(size_t size, size_t position) :
    _index(position), _size(size)
{
}

Env::Iterator& Env::Iterator::operator++ () {
    if (_index < _size) {
        ++_index;
    } else {
        // Actually we are better off raising an exception:
        raise<IndexOutOfRangeException>("iterator",
                                        static_cast<size_t>(_index),
                                        static_cast<size_t>(0),
                                        static_cast<size_t>(_size));
        _index = _size;
    }

    return *this;
}


Env::Var Env::Iterator::operator-> () const {
    if (!(_index < _size)) {
        raise<IndexOutOfRangeException>("iterator",
                                        static_cast<size_t>(_index),
                                        static_cast<size_t>(0),
                                        static_cast<size_t>(_size));
    }

    const auto env = environ[_index];
    auto c = strchr(env, '=');

    if (!c) {
        raise<IndexOutOfRangeException>("iterator: Env",
                                        static_cast<size_t>(_index),
                                        static_cast<size_t>(0),
                                        static_cast<size_t>(_size));
    }

    const String::size_type varNameSize = std::distance(env, c);

    return { {env,  varNameSize}, c };
}


Env::Env() {

}


Optional<String> Env::get(const String& name) const {
    auto value = secure_getenv(name.c_str());

    return (value)
            ? Optional<String>::of(value)
            : None();
}


void Env::set(const String& name, const String& value, bool replace) {
    auto result = setenv(name.c_str(), value.c_str(), replace);

    // TODO(abbyssoul): should be ErrnoError
    if (result) {
        raise<Exception>("setenv has failed!.");
    }
}


void Env::unset(const String& name) {
    auto result = unsetenv(name.c_str());
    if (result) {
        raise<Exception>("unsetenv has failed. Possibly invalid value of the name has been give.");
    }
}


// cppcheck-suppress unusedFunction
void Env::clear() {
    if (clearenv()) {
        raise<Exception>("clearenv has failed. No idea how that even possible.");
    }
}


Env::size_type Env::size() const noexcept {
    size_type environSize = 0;

    // TODO(abbyssoul): Unbounded loop OMG!
    while (environ && environ[environSize]) {
        ++environSize;
    }

    return environSize;
}


String Env::operator[] (const String& name) const {
    return get(name).get();
}


Env::const_iterator Env::begin() const {
    return Iterator(size(), 0);
}

Env::const_iterator Env::end() const {
    const auto pos = size();
    return Iterator(pos, pos);
}

const Env& Env::forEach(const std::function<void(const String&)> &f) const {

    for (size_t i = 0; environ && environ[i]; ++i) {
        f(environ[i]);
    }

    return *this;
}
