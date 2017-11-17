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
 * @file: framework/commanalineParser.cpp
 *
 *  Created by soultaker on 18/08/16.
*******************************************************************************/

#include "solace/parseUtils.hpp"


#include <fmt/format.h>

#include <cstring>
#include <cstdlib>

#include <type_traits>  // std::is_signed
#include <climits>



using namespace Solace;


auto conversionError(const char* fmt, const char* name, const char* value) {
    return Err<Error>(fmt::format(fmt, name, value));
}


template<typename T, bool isSigned = std::is_signed<T>::value>
struct Longest {
};

template<typename T>
struct Longest<T, true> {

    using type = int64;  // long long;

    static Result<type, Error> parse(const char* name, const char* value) {
        errno = 0;
        char* pEnd = nullptr;
        const auto result = strtoll(value, &pEnd, 0);

        if ((errno == ERANGE && (result == LLONG_MAX || result == LLONG_MIN)) || (errno != 0 && result == 0))
            return conversionError("Argument '{}' is outside of parsable int range: '{}'", name, value);

        if (!pEnd || pEnd == value)  // No conversion has been done
            return conversionError("Argument '{}' is not a valid value: '{}'", name, value);

        if (result > std::numeric_limits<T>::max() ||
            result < std::numeric_limits<T>::min())
            return conversionError("Argument '{}' is outside of bounds: '{}'", name, value);

        return Ok(result);
    }
};


template<typename T>
struct Longest<T, false> {

    using type = uint64;  // unsigned long long;

    static Result<type, Error> parse(const char* name, const char* value) {
        errno = 0;
        char* pEnd = nullptr;
        const auto result = strtoull(value, &pEnd, 0);

        if ((errno == ERANGE && (result == ULLONG_MAX)) || (errno != 0 && result == 0))
            return conversionError("Argument '{}' is outside of parsable uint range: '{}'", name, value);

        if (!pEnd || pEnd == value)  // No conversion has been done
            return conversionError("Argument '{}' is not a valid value: '{}'", name, value);

        if (result > std::numeric_limits<T>::max() ||
            result < std::numeric_limits<T>::min())
            return conversionError("Argument '{}' is outside of bounds: '{}'", name, value);

        return Ok(result);
    }
};



Result<int8, Error>
Solace::tryParseInt8(const char* value, const char* name) { return Longest<int8>::parse(name, value); }

Result<int16, Error>
Solace::tryParseInt16(const char* value, const char* name) { return Longest<int16>::parse(name, value); }

Result<int32, Error>
Solace::tryParseInt32(const char* value, const char* name) { return Longest<int32>::parse(name, value); }

Result<int64, Error>
Solace::tryParseInt64(const char* value, const char* name) { return Longest<int64>::parse(name, value); }

Result<uint8, Error>
Solace::tryParseUInt8(const char* value, const char* name) { return Longest<uint8>::parse(name, value); }

Result<uint16, Error>
Solace::tryParseUInt16(const char* value, const char* name) { return Longest<uint16>::parse(name, value); }

Result<uint32, Error>
Solace::tryParseUInt32(const char* value, const char* name) { return Longest<uint32>::parse(name, value); }

Result<uint64, Error>
Solace::tryParseUInt64(const char* value, const char* name) { return Longest<uint64>::parse(name, value); }
