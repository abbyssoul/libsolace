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
 *  @file   systemErrorDomain.cpp
 *  @brief  Globals for System Error Domain
 ******************************************************************************/
#include "solace/errorDomain.hpp"
#include "solace/posixErrorDomain.hpp"
#include "solace/string.hpp"


#include <cstring>
#include <cerrno>


using namespace Solace;


const AtomValue Solace::kSystemCatergory = atom("posix");

namespace /*anonymous*/ {

struct SystemErrorDomain final
        : public ErrorDomain
{
    StringView name() const noexcept override {
        return StringLiteral{"PosixSystemError"};
    }

    String message(int code) const noexcept override {
		auto errorMessage = makeString(std::strerror(code));

		return (errorMessage)
				? errorMessage.moveResult()
				: makeString(StringLiteral{"PosixSystemError: Failed to get error string"});
    }
};


static SystemErrorDomain kSystemErrorDomain;
static const auto rego = registerErrorDomain(kSystemCatergory, kSystemErrorDomain);

}  // namespace

Error
Solace::makeErrno() noexcept {
    return makeSystemError(errno);
}


Error
Solace::makeErrno(StringLiteral tag) noexcept {
	return makeSystemError(errno, mv(tag));
}
