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

#include "solace/libsolace_config.h"

#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <utility>	    // std::move, tuple, forward, pair, etc

namespace Solace {

typedef int8_t			int8;
typedef int16_t			int16;
typedef int32_t			int32;
typedef int64_t			int64;
typedef uint8_t         uint8;
typedef uint16_t		uint16;
typedef uint32_t		uint32;
typedef uint64_t 		uint64;


//! 32 bit floating point variable.
/** This is a typedef for float, it ensures portability of the engine. */
typedef float				float32;

//! 64 bit floating point variable.
/** This is a typedef for double, it ensures portability of the engine. */
typedef double				float64;

//! 8 bit unsigned variable - byte.
typedef uint8				byte;


// Static check of Integral types
// ...otherwise things might fail with compilers!
static_assert(1 == sizeof(bool), 	"size of bool is not 1 byte!");
static_assert(1 == sizeof(byte), 	"size of byte is not 1 byte!");
static_assert(1 == sizeof(uint8), 	"size of uint8 is not 1 byte!");
static_assert(1 == sizeof(int8), 	"size of int8 is not 1 byte!");
static_assert(2 == sizeof(int16), 	"size of int16 is not 2 bytes!");
static_assert(2 == sizeof(uint16), "size of uint16 is not 2 bytes!");
static_assert(4 == sizeof(int32), 	"size of int32 is not 4 bytes!");
static_assert(4 == sizeof(uint32), "size of uint32 is not 4 bytes!");
static_assert(8 == sizeof(int64), 	"size of uint64 is not 8 bytes!");
static_assert(8 == sizeof(uint64), "size of uint64 is not 8 bytes!");

static_assert(4 == sizeof(float32), "size of float32 is not 4 bytes!");
static_assert(8 == sizeof(float64), "size of float64 is not 8 bytes!");

// FIXME(abbyssoul): Is it really a blocker?
static_assert(sizeof(uint) == sizeof(uint32),
		"Size of uint is not equal to the size of uint32!");

template <typename T, ::std::size_t N>
::std::size_t arrayLength(const T (& SOLACE_UNUSED(t))[N]) { return N; }

}  // End of namespace Solace
#endif  // SOLACE_TYPES_HPP
