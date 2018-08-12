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
 *	@file		array.cpp
 *	@author		$LastChangedBy: soultaker $
 *	@date		$LastChangedDate$
 *	@brief		Helper functions for fixed size arrays
 *	ID:			$Id$
 ******************************************************************************/
#include "solace/array.hpp"

#include "solace/exception.hpp"



using namespace Solace;


const HeapArrayDisposer HeapArrayDisposer::instance = HeapArrayDisposer();




// Utility class that assists in constructing or destroying elements of an array, where the
// constructor or destructor could throw exceptions.  In case of an exception,
// ExceptionSafeArrayUtil's destructor will call destructors on all elements that have been
// constructed but not destroyed.  Remember that destructors that throw exceptions are required
// to use UnwindDetector to detect unwind and avoid exceptions in this case.  Therefore, no more
// than one exception will be thrown (and the program will not terminate).
class ExceptionSafeArrayUtil {
public:

    inline ~ExceptionSafeArrayUtil() noexcept(false) {
        if (_constructedElementCount > 0) destroyAll();
    }

    inline ExceptionSafeArrayUtil(void* ptr, size_t elementSize, size_t constructedElementCount,
                                void (*destoyer)(void*)) noexcept
        : _pos(reinterpret_cast<byte*>(ptr) + elementSize * constructedElementCount)
        , _elementSize(elementSize)
        , _constructedElementCount(constructedElementCount)
        , _destroyElement(destoyer)
    {}

    ExceptionSafeArrayUtil(ExceptionSafeArrayUtil const&) = delete;
    ExceptionSafeArrayUtil& operator= (ExceptionSafeArrayUtil const& ) = delete;

    // Construct the given number of elements.
    void construct(size_t count, void (*constructElement)(void*)) {
        while (count > 0) {
            constructElement(_pos);
            _pos += _elementSize;
            ++_constructedElementCount;
            --count;
        }
    }

    // Destroy all elements.  Call this immediately before ExceptionSafeArrayUtil goes out-of-scope
    // to ensure that one element throwing an exception does not prevent the others from being
    // destroyed.
    void destroyAll() {
        while (_constructedElementCount > 0) {
            _pos -= _elementSize;
            --_constructedElementCount;
            _destroyElement(_pos);
        }
    }

    // Prevent ExceptionSafeArrayUtil's destructor from destroying the constructed elements.
    // Call this after you've successfully finished constructing.
    void release() noexcept { _constructedElementCount = 0; }

private:

    byte*       _pos;
    size_t      _elementSize;
    size_t      _constructedElementCount;
    void        (*_destroyElement)(void*);
};



struct AutoDeleter {
    void* ptr;

    inline ~AutoDeleter() { operator delete(ptr); }
    constexpr AutoDeleter(void* pos) noexcept: ptr(pos) {}
    void* release() noexcept { void* result = ptr; ptr = nullptr; return result; }
};



void*
HeapArrayDisposer::allocateImpl(size_t elementSize, size_type elementCount, size_type capacity,
                                      void (*constructElement)(void*),
                                      void (*destroyElement)(void*)) {

    // NOTE: Allocate memory for the array storage
    AutoDeleter result(operator new(elementSize * capacity));

    if (constructElement == nullptr) {
        // Nothing to do.
    } else if (destroyElement == nullptr) {
        byte* pos = reinterpret_cast<byte*>(result.ptr);
        while (elementCount > 0) {
            constructElement(pos);
            pos += elementSize;
            --elementCount;
        }
    } else {
        ExceptionSafeArrayUtil guard(result.ptr, elementSize, 0, destroyElement);
        guard.construct(elementCount, constructElement);
        guard.release();
    }

    return result.release();
}


void
HeapArrayDisposer::disposeImpl(void* firstElement, size_t elementSize, size_type elementCount,
    void (*destroyElement)(void*)) const {
    // Note that capacity is ignored since operator delete() doesn't care about it.
    AutoDeleter deleter(firstElement);

    if (destroyElement != nullptr) {
        ExceptionSafeArrayUtil guard(firstElement, elementSize, elementCount, destroyElement);
        guard.destroyAll();
    }
}
