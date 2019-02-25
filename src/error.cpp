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
#include "solace/posixErrorDomain.hpp"
#include "solace/string.hpp"


using namespace Solace;


String
Error::toString() const {
    auto const domain = getErrorDomain(_domain);

    if (!domain) {
        constexpr auto N = sizeof(AtomValue);
        char buffer[sizeof(N) + 1];
        detail::unwrap<std::uintmax_t>(static_cast<std::uintmax_t>(_domain), buffer);

        return makeString(StringView{buffer}, StringView{": "}, _tag);
    }

    return (*domain)->getMessage(_code);
}
