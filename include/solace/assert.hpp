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
 * libSolace: Assertion helpers
 *	@file		solace/assert.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_ASSERT_HPP
#define SOLACE_ASSERT_HPP

#include "solace/types.hpp"


namespace Solace {

/**
 * Throw an error of invalidState
 */
void raiseInvalidStateError();

/**
 * Throw an error if an object is in invalid state.
 * This usually indicate internale program logic error.
 *
 * @param message Error message to describe invalid state of an object.
 */
void raiseInvalidStateError(const char* message);



/**
 * An action taken when assertion have failed.
 * On system that support it - throws an exception or aborts exectution.
 * @param tag A tag to identify the assertion that has failed.
 */
//[[noreturn]]
void assertFail(const char* tag);

inline void assertTrue(bool condition, const char* tag)  {
    if (!condition) {
        assertFail(tag);
    }
}


/**
 * Check that condition is true.
 * @param condition Condition to check.
 */
inline void assertTrue(bool condition) {
    assertTrue(condition, "Assertion failed");
}




/**
 * Check that a pointer is NULL, throw an exception if it is not.
 * @param ptr A pointer to check.
 * @param message A message content of an excetpion thrown in case of assertion failure.
 */
inline const void* assertNull(const void* ptr, const char* message) {
    assertTrue(ptr == nullptr, message);

    return ptr;
}

/**
 * Check that a pointer is not NULL, throw an exception if it is.
 * @param ptr A pointer to check.
 * @param message A message content of an excetpion thrown in case of assertion failure.
 */
inline const void* assertNotNull(const void* ptr, const char* message) {
    assertTrue(ptr != nullptr, message);

    return ptr;
}


/**
 * Check that a pointer is NULL, throw an exception if it is not.
 * @param ptr A pointer to check for being nullptr.
 */
inline const void* assertNull(const void* ptr) {
    return assertNull(ptr, "Assertion violation: Null pointer value passed.");
}

/**
 * Check that a pointer is not NULL, throw an exception if it is.
 * @param ptr A pointer to check.
 */
inline const void* assertNotNull(const void* ptr)  {
    return assertNotNull(ptr, "Assertion violation: Null pointer value passed.");
}


template<typename T>
T* assertNotNull(T* value) {
    assertNotNull(static_cast<const void*>(value));

    return value;
}

template<typename T>
T* assertNull(T* value) {
    assertNull(static_cast<const void*>(value));

    return value;
}


/**
 * Assert that the give index is within the give range. Throw if it is not.
 *
 * @param index Index value to be asserted.
 * @param from Lower bound (inclusive)
 * @param to Upper value bound (exclusive)
 * @return Index value if the index is in range. Throws otherwise.
 */
uint16 assertIndexInRange(uint16 index, uint16 from, uint16 to);

/**
 * Assert that the give index is within the give range. Throw if it is not.
 *
 * @param index Index value to be asserted.
 * @param from Lower bound (inclusive)
 * @param to Upper value bound (exclusive)
 * @return Index value if the index is in range. Throws otherwise.
 */
uint32 assertIndexInRange(uint32 index, uint32 from, uint32 to);

/**
 * Assert that the give index is within the give range. Throw if it is not.
 *
 * @param index Index value to be asserted.
 * @param from Lower bound (inclusive)
 * @param to Upper value bound (exclusive)
 * @return Index value if the index is in range. Throws otherwise.
 */
uint64 assertIndexInRange(uint64 index, uint64 from, uint64 to);


/**
 * Assert that the give index is within the give range. Throw if it is not.
 * @param index Index value to be asserted.
 * @param from Lower bound (inclusive).
 * @param to Upper value bound (exclusive).
 * @param Message identifying location.
 *
 * @return Index value if the index is in range. Throws otherwise.
 */
uint16 assertIndexInRange(uint16 index, uint16 from, uint16 to, const char* message);


/**
 * Assert that the give index is within the give range. Throw if it is not.
 * @param index Index value to be asserted.
 * @param from Lower bound (inclusive).
 * @param to Upper value bound (exclusive).
 * @param Message identifying location.
 *
 * @return Index value if the index is in range. Throws otherwise.
 */
uint32 assertIndexInRange(uint32 index, uint32 from, uint32 to, const char* message);


/**
 * Assert that the give index is within the give range. Throw if it is not.
 * @param index Index value to be asserted.
 * @param from Lower bound (inclusive).
 * @param to Upper value bound (exclusive).
 * @param Message identifying location.
 *
 * @return Index value if the index is in range. Throws otherwise.
 */
uint64 assertIndexInRange(uint64 index, uint64 from, uint64 to, const char* message);

}  // End of namespace Solace
#endif  // SOLACE_ASSERT_HPP

