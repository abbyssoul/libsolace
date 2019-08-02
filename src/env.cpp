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
 *	@brief		Implementation of Env class
 ******************************************************************************/
#include "solace/env.hpp"
#include "solace/posixErrorDomain.hpp"


#include <cstdlib>
#include <cstring>

#include <unistd.h>


using namespace Solace;


#ifdef __APPLE__
# include <crt_externs.h>
# define environ (*_NSGetEnviron())
int clearenv(void) {
    environ[0] = nullptr;
    return 0;
}
#elif defined SOLACE_PLATFORM_BSD
extern char **environ;
int clearenv(void) {
    environ[0] = nullptr;
    return 0;
}
#else
extern char** environ;
#endif





Env::Iterator&
Env::Iterator::operator++ () {
    assertIndexInRange(_index, 0, _size, "iterator");

    ++_index;

    return *this;
}


Env::Var
Env::Iterator::operator-> () const {
    assertIndexInRange(_index, 0, _size, "iterator");

    Env::Var var;
    StringView{environ[_index]}
        .split(StringLiteral{"="}, [&var](StringView v, StringView::size_type i, StringView::size_type) {
            if (i == 0) {
               var.name = v;
            } else if (i == 1) {
               var.value = v;
            }

            ++i;
        });

    return var;
}


Env::Env() noexcept
{
}


Optional<StringView>
Env::get(StringView name) const noexcept {
    for (auto e : *this) {
        if (e.name.equals(name))
            return Optional<StringView>{e.value};
    }

    return none;

//    auto value =
//    #ifdef SOLACE_PLATFORM_LINUX
//        secure_getenv(name.data());
//    #else
//        getenv(name.data());
//    #endif

//    return (value != nullptr)
//            ? Optional<StringView>{value}
//            : none;
}


Result<void, Error>
Env::set(StringView name, StringView value, bool replace) noexcept {
    return (setenv(name.data(), value.empty() ? "" : value.data(), replace) == 0)
            ? none
            : Result<void, Error>{makeErrno("setenv")};
}


Result<void, Error> Env::unset(StringView name) noexcept {
    return (unsetenv(name.empty() ? "" : name.data()) == 0)
            ? none
            : Result<void, Error>{makeErrno("unsetenv")};
}


// cppcheck-suppress unusedFunction
Result<void, Error>
Env::clear() noexcept {
    return (clearenv())
        ? none
        : Result<void, Error>{makeErrno("clearenv")};
}


Env::size_type
Env::size() const noexcept {
    size_type environSize = 0;

    // NOTE: Unbounded loop OMG!
    while (environ && environ[environSize]) {
        ++environSize;
    }

    return environSize;
}
