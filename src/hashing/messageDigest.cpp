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
#include "solace/vector.hpp"


using namespace Solace;
using namespace Solace::hashing;



MessageDigest::MessageDigest(MemoryView viewBytes)
	: _storage{makeArray<byte>(viewBytes.size(), viewBytes.dataAs<byte>()).moveResult()}
{ }


String
MessageDigest::toString() const {
    auto stringBuffer = makeVector<char>(Base16Encoder::encodedSize(size()));
	if (!stringBuffer) {
		return String{};
	}

	auto& buffer = stringBuffer.unwrap();
	ByteWriter dest{wrapMemory(buffer.data(), buffer.size())};

    auto const dataView = _storage.view().view();
    for (auto i = base16Encode_begin(dataView),
         end = base16Encode_end(dataView);
         i != end; ++i) {
        dest.write((*i).view());
    }

	auto result = makeString(buffer.data(), narrow_cast<String::size_type>(buffer.size()));
	return result
			? result.moveResult()
			: String{};
}
