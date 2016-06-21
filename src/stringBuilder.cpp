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
 * libSolace
 *	@file		stringBuilder.cpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	@brief		Implementation of StringBuilder
 *	ID:			$Id$
 ******************************************************************************/
#include "solace/stringBuilder.hpp"
#include "solace/exception.hpp"


using Solace::StringBuilder;
using Solace::String;
using Solace::IllegalArgumentException;
using Solace::Optional;
using Solace::ByteBuffer;


StringBuilder::StringBuilder(const ByteBuffer& buffer): _buffer(buffer) {
}

StringBuilder::StringBuilder(const ByteBuffer& buffer, const char* cstr): _buffer(buffer) {
    if (!cstr) {
        raise<IllegalArgumentException>("cstr");
    }

    _buffer.write(cstr, std::char_traits<char>::length(cstr));
}


StringBuilder::StringBuilder(const ByteBuffer& buffer, const String& str): _buffer(buffer) {
	_buffer.write(str.c_str(), str.length());
}


StringBuilder::StringBuilder(const ByteBuffer& buffer, std::initializer_list<String> content): _buffer(buffer) {
    for (const auto& str : content) {
        _buffer.write(str.c_str(), str.length());
    }
}


StringBuilder& StringBuilder::append(char c) {
	_buffer << c;

	return *this;
}

StringBuilder& StringBuilder::append(const Char& c) {
	_buffer.write(c.c_str(), c.getBytesCount());

    return *this;
}

StringBuilder& StringBuilder::append(const char* cstr) {
	_buffer.write(cstr, std::char_traits<char>::length(cstr));

	return *this;
}

StringBuilder& StringBuilder::append(const IFormattable& f) {
	return append(f.toString());
}

StringBuilder& StringBuilder::append(const String& str) {
	_buffer.write(str.c_str(), str.length());

    return *this;
}

String StringBuilder::toString() const {
    return String(_buffer.viewWritten());
}

bool StringBuilder::empty() const {
	return _buffer.empty();
}


String StringBuilder::substring(size_type from, size_type to) const {
    // TODO(abbyssoul): Check for index out of range

    auto data = _buffer.viewWritten();
    return String(std::string(
            reinterpret_cast<const char*>(data.dataAddress() + from),
            to - from));
}

Optional<StringBuilder::size_type> StringBuilder::indexOf(const Char& ch, size_type fromIndex) const {
    // TODO(abbyssoul): Check for index out of range

    Char c;
    for (size_type i = fromIndex; i + ch.getBytesCount() < _buffer.position(); ++i) {
        _buffer.read(i, const_cast<byte*>(c.data()), ch.getBytesCount());
        if (c.equals(ch)) {
            return Optional<size_type>::of(i);
        }
    }

    return Optional<size_type>::none();
}
