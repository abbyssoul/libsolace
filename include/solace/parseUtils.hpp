/*
*  Copyright 2017 Ivan Ryabov
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
 * libSolace: Command line parser
 *	@file		solace/parseUtils.hpp
 *	@brief		Various text parsing utils that has not found a better home yet.
 ******************************************************************************/
#pragma once
#ifndef SOLACE_PARSEUTILS_HPP
#define SOLACE_PARSEUTILS_HPP

#include "solace/types.hpp"
#include "solace/string.hpp"
#include "solace/result.hpp"
#include "solace/error.hpp"

namespace Solace {

Result<int8, Error> tryParseInt8(const char* value, const char* name);

Result<int16, Error> tryParseInt16(const char* value, const char* name);

Result<int32, Error> tryParseInt32(const char* value, const char* name);

Result<int64, Error> tryParseInt64(const char* value, const char* name);

Result<uint8, Error> tryParseUInt8(const char* value, const char* name);

Result<uint16, Error> tryParseUInt16(const char* value, const char* name);

Result<uint32, Error> tryParseUInt32(const char* value, const char* name);

Result<uint64, Error> tryParseUInt64(const char* value, const char* name);


template<typename T>
Result<T, Error> tryParse(const char* value, const char* name);


template<>
inline
Result<int8, Error> tryParse<int8>(const char* value, const char* name) { return tryParseInt8(value, name); }

template<>
inline
Result<int16, Error> tryParse<int16>(const char* value, const char* name) { return tryParseInt16(value, name); }

template<>
inline
Result<int32, Error> tryParse<int32>(const char* value, const char* name) { return tryParseInt32(value, name); }

template<>
inline
Result<int64, Error> tryParse<int64>(const char* value, const char* name) { return tryParseInt64(value, name); }


template<>
inline
Result<uint8, Error> tryParse<uint8>(const char* value, const char* name) { return tryParseUInt8(value, name); }

template<>
inline
Result<uint16, Error> tryParse<uint16>(const char* value, const char* name) { return tryParseUInt16(value, name); }

template<>
inline
Result<uint32, Error> tryParse<uint32>(const char* value, const char* name) { return tryParseUInt32(value, name); }

template<>
inline
Result<uint64, Error> tryParse<uint64>(const char* value, const char* name) { return tryParseUInt64(value, name); }


}  // End of namespace Solace
#endif  // SOLACE_PARSEUTILS_HPP
