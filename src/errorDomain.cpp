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
 *	@file		errorDomain.cpp
 *	@brief		Implementation of the Error Domain
 ******************************************************************************/
#include "solace/errorDomain.hpp"
#include "solace/posixErrorDomain.hpp"
#include "solace/dictionary.hpp"


using namespace Solace;

namespace  {

static const uint32 kNbErrorCategories = 128;

Dictionary<AtomValue, ErrorDomain*>
autoRegisterDomainMap() noexcept {
	static byte kKeysBuffer[kNbErrorCategories * sizeof(AtomValue)];
	static byte kValuesBuffer[kNbErrorCategories * sizeof(ErrorDomain*)];

	auto maybeDict = makeDictionary<AtomValue, ErrorDomain*>(
				wrapMemory(kKeysBuffer), wrapMemory(kValuesBuffer));

	return maybeDict.moveResult();
}

// Poor-man's singleton
static Dictionary<AtomValue, ErrorDomain*> kErrorDomainMap = autoRegisterDomainMap();

}  // namespace


uint32
Solace::registerErrorDomain(AtomValue categoryId, ErrorDomain& domain) noexcept {
    kErrorDomainMap.put(categoryId, &domain);

    return kErrorDomainMap.size();
}


Optional<ErrorDomain&>
Solace::findErrorDomain(AtomValue categoryId) noexcept {
	auto maybePointer = kErrorDomainMap.find(categoryId);
	if (!maybePointer)
		return none;

	ErrorDomain* p = *maybePointer;
	ErrorDomain& ref = *p;
	return Optional<ErrorDomain&>{std::ref(ref)};
}
