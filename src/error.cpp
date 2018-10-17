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
 *	@file		error.cpp
 *	@author		$LastChangedBy: $
 *	@date		$LastChangedDate: $
 *	@brief		Implementation of error type
 *	ID:			$Id: $
 ******************************************************************************/
#include "solace/error.hpp"

#include <cstring>
#include <cerrno>

using namespace Solace;


const AtomValue Solace::kDefaultCatergory = atom("default");
const AtomValue Solace::kSystemCatergory = atom("posix");


Error
Solace::makeErrno(int errCode, StringLiteral tag) {
    return Error{kSystemCatergory, errCode, std::move(tag)};
}

Error
Solace::makeErrno(int errCode) {
    return Error{kSystemCatergory, errCode};
}


Error
Solace::makeErrno(StringLiteral tag) {
    return makeErrno(errno, std::move(tag));
}

Error
Solace::makeErrno() {
    return makeErrno(errno);
}


StringView
Error::toString() const {
    auto const domain = getErrorDomain(_domain);
    return domain->getMessage(_code);
}

