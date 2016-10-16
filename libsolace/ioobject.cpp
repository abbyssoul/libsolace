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
#include "solace/byteBuffer.hpp"

using namespace Solace;


IOObject::IOResult IOObject::read(ByteBuffer& destBuffer) {
    return read(destBuffer, destBuffer.remaining());
}

IOObject::IOResult IOObject::read(ByteBuffer& destBuffer, size_type bytesToRead) {
    auto destSlice = destBuffer.viewRemaining().slice(0, bytesToRead);
    auto result = read(destSlice);

    if (result) {
        destBuffer.advance(result.getResult());
    }

    return result;
}

IOObject::IOResult IOObject::write(ByteBuffer& srcBuffer) {
    return write(srcBuffer, srcBuffer.remaining());
}


IOObject::IOResult IOObject::write(ByteBuffer& srcBuffer, size_type bytesToWrite) {
    auto result = write(srcBuffer.viewRemaining().slice(0, bytesToWrite));

    if (result) {
        srcBuffer.advance(result.getResult());
    }

    return result;
}
