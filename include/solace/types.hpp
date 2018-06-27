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
 * libSolace: Primitive integral type definitions
 *	@file		solace/types.hpp
 *	@author		$LastChangedBy: $
 *	@date		$LastChangedDate: $
 *	@brief		Defines integral types used by library to ensure portability
 *	ID:			$Id: $
 ******************************************************************************/
#pragma once
#ifndef SOLACE_TYPES_HPP
#define SOLACE_TYPES_HPP

#include "solace/libsolace_config.hpp"

#include <cstddef>
#include <cstdint>

namespace Solace {

using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;


//! 32 bit floating point variable.
/** This is a typedef for float, it ensures portability of the engine. */
using float32 = float;

//! 64 bit floating point variable.
/** This is a typedef for double, it ensures portability of the engine. */
using float64 = double;

//! 8 bit unsigned variable - byte.
using byte = uint8;


// Static check of Integral types
// ...otherwise things might fail with compilers!
static_assert(1 == sizeof(bool), 	"size of bool is not 1 byte!");
static_assert(1 == sizeof(byte), 	"size of byte is not 1 byte!");
static_assert(1 == sizeof(uint8), 	"size of uint8 is not 1 byte!");
static_assert(1 == sizeof(int8), 	"size of int8 is not 1 byte!");
static_assert(2 == sizeof(int16), 	"size of int16 is not 2 bytes!");
static_assert(2 == sizeof(uint16),  "size of uint16 is not 2 bytes!");
static_assert(4 == sizeof(int32), 	"size of int32 is not 4 bytes!");
static_assert(4 == sizeof(uint32),  "size of uint32 is not 4 bytes!");
static_assert(8 == sizeof(int64), 	"size of uint64 is not 8 bytes!");
static_assert(8 == sizeof(uint64),  "size of uint64 is not 8 bytes!");

static_assert(4 == sizeof(float32), "size of float32 is not 4 bytes!");
static_assert(8 == sizeof(float64), "size of float64 is not 8 bytes!");


template <typename T, size_t N>
size_t arrayLength(T const (& SOLACE_UNUSED(t))[N]) { return N; }

}  // End of namespace Solace
#endif  // SOLACE_TYPES_HPP
