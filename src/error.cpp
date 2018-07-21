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

using namespace Solace;


StringView Error::toString() const {
    // FIXME(abbyssoul): This is just a shitty engineering
    return StringView{_message.c_str(), static_cast<StringView::size_type>(_message.size())};
}


Error Solace::make_error(char const* msg) {
    return Error{msg};
}


Error Solace::make_errno() {

    // FIXME(abbyssoul): strerror can fail so we should set errno=0 before the call and check it after.
    return Error{strerror(errno)};
}
