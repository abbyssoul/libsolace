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
 *	@file		solace/hashing/messageDigest.cpp
 *	@brief		Implementation of MessageDigest class
 ******************************************************************************/
#include "solace/hashing/messageDigest.hpp"

#include "solace/base16.hpp"

#include <sstream>      // std::stringstream, std::stringbuf


using namespace Solace;
using namespace Solace::hashing;


void nibbleData(char* buffer, const byte* data, size_t len);


std::ostream&
operator<< (std::ostream& ostr, MessageDigest const& a) {
    ostr << '[';

    char buffer[3];
    WriteBuffer dest(wrapMemory(buffer));
    Base16Encoder encoder(dest);

    for (MessageDigest::size_type end = a.size(), i = 0; i < end; ++i) {
        encoder.encode(wrapMemory(&a[i], 1));

        ostr << "0x";
        ostr.write(buffer, 2);
        dest.rewind();
    }
    ostr << ']';

    return ostr;
}


String
MessageDigest::toString() const {
    std::vector<char> stringRepr;
    stringRepr.reserve(Base16Encoder::encodedSize(size()));
    WriteBuffer dest(wrapMemory(stringRepr.data(), stringRepr.size()));

    for (auto i = base16Encode_begin(_storage.view()),
         end = base16Encode_end(_storage.view());
         i != end; ++i) {
        dest.write((*i).view());
    }

    return String{stringRepr.data(), static_cast<String::size_type>(stringRepr.size())};
}
