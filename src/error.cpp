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
 *	@brief		Implementation of an error type
 ******************************************************************************/
#include "solace/posixErrorDomain.hpp"
#include "solace/string.hpp"


using namespace Solace;


String
Error::toString() const {
    auto const domain = findErrorDomain(_domain);
	if (domain) {
		return (*domain)->message(_code);
	}

	// In case domain is not known:
	constexpr auto const N = sizeof(AtomValue);
	char buffer[N + 1] = {0};
	atomToString(_domain, buffer);

	auto maybeString = makeString(StringView{buffer}, StringView{": "}, _tag);
	return maybeString
			? maybeString.moveResult()
			: String{};
}
