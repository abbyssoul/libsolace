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
 *	@file		ioobject.cpp
 *	@author		$LastChangedBy: soultaker $
 *	@date		$LastChangedDate$
 *	@brief		Implementation of IOObject
 *	ID:			$Id$
 ******************************************************************************/
#include "solace/ioobject.hpp"

using namespace Solace;


IOObject::IOResult
IOObject::read(ByteWriter& destBuffer, size_type bytesToRead) {
    auto destSlice = destBuffer.viewRemaining().slice(0, bytesToRead);

    return read(destSlice)
            .then([&destBuffer](auto bytesRead) {
                destBuffer.advance(bytesRead);

                return bytesRead;
            });
}

IOObject::IOResult
IOObject::write(ByteReader& srcBuffer, size_type bytesToWrite) {
    return write(srcBuffer.viewRemaining().slice(0, bytesToWrite))
            .then([&srcBuffer](auto bytesRead) {
                srcBuffer.advance(bytesRead);

                return bytesRead;
            });
}
