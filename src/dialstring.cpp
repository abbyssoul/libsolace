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
	auto result = DialString{kProtocolNone, data};
	bool isFailure = false;

    if (data.contains(':')) {
		data.split(":", [&result, &isFailure](StringView split, StringView::size_type i, StringView::size_type) {
			if (isFailure)
				return;

            if (i == 0) {
				auto parseResult = tryParseAtom(split.trim());
				if (!parseResult) {
					isFailure = true;
				} else {
					result.protocol = *parseResult;
				}
            } else if (i == 1) {
                result.address = split.trim();
            } else if (i == 2) {
                result.service = split.trim();
            }
        });
    }

	if (isFailure)
		return makeError(BasicError::InvalidInput, "tryParseDailString");

    return Ok(result);
}
