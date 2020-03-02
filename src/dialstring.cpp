/*
*  Copyright 2018 Ivan Ryabov
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
#include "solace/dialstring.hpp"
#include "solace/posixErrorDomain.hpp"

using namespace Solace;


Result<DialString, Error>
Solace::tryParseDailString(Solace::StringView data) noexcept {
	if (data.empty())
		return makeError(BasicError::InvalidInput, "tryParseDailString");

	auto result = Result<DialString, Error>{types::okTag, in_place, kProtocolNone, data};
	auto& ds = *result;
    if (data.contains(':')) {
		data.split(":", [&result, &ds](StringView split, StringView::size_type i, StringView::size_type) {
			if (!result) return;

            if (i == 0) {
				if (auto parseResult = tryParseAtom(split.trim())) {
					ds.protocol = *parseResult;
				} else {
					result = makeError(BasicError::InvalidInput, "tryParseDailString");
				}
            } else if (i == 1) {
				ds.address = split.trim();
            } else if (i == 2) {
				ds.service = split.trim();
            }
        });
    }

	return result;
}
