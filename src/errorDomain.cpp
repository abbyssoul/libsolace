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
 *	@brief		Implementation of Error domain
 ******************************************************************************/
#include "solace/errorDomain.hpp"
#include "solace/posixErrorDomain.hpp"
#include "solace/asyncErrorDomain.hpp"

#include "solace/dictionary.hpp"


using namespace Solace;


const AtomValue Solace::kDefaultCatergory = atom("defaul");
const AtomValue Solace::kSystemCatergory = atom("posix");
const AtomValue Solace::kAsyncErrorCatergory = atom("async");


static Dictionary<AtomValue, ErrorDomain*> kErrorDomainMap;


uint32 Solace::registerErrorDomain(AtomValue categoryId, ErrorDomain& domain) noexcept {
    kErrorDomainMap.put(categoryId, &domain);

    return kErrorDomainMap.size();
}


Optional<ErrorDomain*>
Solace::getErrorDomain(AtomValue categoryId) noexcept {
    return kErrorDomainMap.find(categoryId);
}
