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
 * libSolace: Compile time config
 *	@file		solace/libsolace_config.h
 *	@brief		Platform and compiler specific configuraion.
 ******************************************************************************/
#pragma once
#ifndef SOLACE_COMPILER_CONFIG_HPP
#define SOLACE_COMPILER_CONFIG_HPP


// Platform and Compiler specific information goes in this header file.
// The defines to control which platform is included are:
//
// _WIN32      :  Microsoft Windows
// WIN32       :  Microsoft Windows
// __MINGW32__ :  Minimalist GNU for Windows
// __CYGWIN__  :  Cygwin
// __APPLE__   :  Macintosh OS X (10.2.3 or higher required)
// __sgi       :  Silicon Graphics Irix
// __sun       :  Sun Solaris
// <none>      :  Linux
//
// Add others as needed.

/*-----------------------------------------------------------------------
 * Define SOLACE_DEBUG macro to indicate debug-mode
 * -----------------------------------------------------------------------*/
#if (defined(_DEBUG) || defined(DEBUG)) && !defined(SOLACE_DEBUG)
#   define SOLACE_DEBUG
#endif

#if defined(SOLACE_MEMORY_DEBUG)
#	define GLIBCXX_FORCE_NEW
#endif  // SOLACE_DEBUG_MEMORY

/*-----------------------------------------------------------------------
 * Next switch will define our platform identifying macro SOLACE_PLATFORM_*
 *-----------------------------------------------------------------------*/
#if defined(_WIN32) || defined(WIN32) || defined(WIN64) || defined(__MINGW32__) || defined(__CYGWIN__)
#	define SOLACE_PLATFORM_WIN
#elif defined(__linux__)
    #define SOLACE_PLATFORM_LINUX
#elif defined(__APPLE__)
    #define SOLACE_PLATFORM_APPLE

	#include <AvailabilityMacros.h>
	#include <TargetConditionals.h>

	#if TARGET_OS_IPHONE && TARGET_IPHONE_SIMULATOR
        #define SOLACE_PLATFORM_IPHONE
    #elif TARGET_OS_IPHONE
        #define SOLACE_PLATFORM_IPHONE
    #else
        #define SOLACE_PLATFORM_OSX
    #endif
#elif  defined(__FreeBSD__) || defined(__OpenBSD__)
#	define SOLACE_PLATFORM_BSD
#else
//    #warning Compiling on Unknown platform!
    #pragma message("Compiling on Unknown platform!")
#endif

#if defined(unix) || defined(SOLACE_PLATFORM_BSD) || defined(SOLACE_PLATFORM_LINUX) || defined(SOLACE_PLATFORM_OSX)
#	define SOLACE_PLATFORM_POSIX
#endif

/*---------------------------------
 * Platform specific settings
 *---------------------------------*/

/**
  @def STR(expression)

  Creates a string from the expression.

  <CODE>STR(this becomes a string)<PRE> evaluates the same as <CODE>"this becomes a string"</CODE>
 */
#define STR(x) #x


#define SOLACE_UNUSED(identifier)

//! Exception specification, for compilers that supports it
#define SOLACE_THROWS(x) throw((x))

//! No throwing specification, for compilers that supports it
#define SOLACE_NO_THROW noexcept



#if __clang__

#define SOLACE_DEPRECATED(reason) \
    __attribute__((deprecated(reason)))
#define SOLACE_UNAVAILABLE(reason) \
    __attribute__((unavailable(reason)))
#define SOLACE_NO_SANITIZE(feature) \
    __attribute__((no_sanitize(feature)))

#elif __GNUC__

#define SOLACE_DEPRECATED(reason) \
    __attribute__((deprecated))
#define SOLACE_UNAVAILABLE(reason)
#define SOLACE_NO_SANITIZE(feature)
#else
#define SOLACE_DEPRECATED(reason)
// TODO(msvc): MSVC prefers a prefix __declspec(deprecated).
#define SOLACE_UNAVAILABLE(reason)
#define SOLACE_NO_SANITIZE(feature)
#endif


#endif  // SOLACE_COMPILER_CONFIG_HPP
