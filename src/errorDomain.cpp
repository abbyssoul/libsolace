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
 *	@brief		Implementation of Error omain
 ******************************************************************************/
#include "solace/errorDomain.hpp"
#include "solace/posixErrorDomain.hpp"
#include "solace/dictionary.hpp"


using namespace Solace;


static const uint32 kNbErrorCategories = 128;


static Dictionary<AtomValue, ErrorDomain const*> kErrorDomainMap;


uint32 Solace::registerErrorDomain(AtomValue categoryId, ErrorDomain const& domain) noexcept {
    static byte kKeysBuffer[kNbErrorCategories * sizeof(AtomValue)];
    static byte kValuesBuffer[kNbErrorCategories * sizeof(ErrorDomain*)];

    if (kErrorDomainMap.capacity() == 0) {
		auto maybeDict = makeDictionary<AtomValue, ErrorDomain const*>(
					wrapMemory(kKeysBuffer), wrapMemory(kValuesBuffer));

		if (maybeDict) {
			kErrorDomainMap = maybeDict.moveResult();
		}
    }

    kErrorDomainMap.put(categoryId, &domain);

    return kErrorDomainMap.size();
}


Optional<ErrorDomain const*>
Solace::findErrorDomain(AtomValue categoryId) noexcept {
    return kErrorDomainMap.find(categoryId);
}
