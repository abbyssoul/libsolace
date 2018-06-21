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


using namespace Solace;



StringBuilder::StringBuilder(MemoryView&& buffer, const StringView& str):
    StringBuilder(std::move(buffer)) {
    _buffer.write(str.data(), str.length());
}

StringBuilder::StringBuilder(MemoryBuffer&& buffer, const StringView& str):
    StringBuilder(std::move(buffer)) {
    _buffer.write(str.data(), str.length());
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

StringView
StringBuilder::view() const noexcept {
    return StringView(_buffer.viewWritten().dataAs<const char>(), _buffer.position());
}


String
StringBuilder::toString() const {
    return (_buffer.position() == 0)
            ? String::Empty
            : String(_buffer.viewWritten());
}

bool StringBuilder::empty() const {
    return (_buffer.position() == 0);
}

StringBuilder::size_type
StringBuilder::length() const noexcept {
    return _buffer.position();
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

    MemoryView::value_type buffer[Char::max_bytes];
    auto b = wrapMemory(buffer, sizeof(buffer));


    for (size_type i = fromIndex; i + ch.getBytesCount() < _buffer.position(); ++i) {
        _buffer.read(i, b, ch.getBytesCount());

        if (ch.equals(b)) {
            return Optional<size_type>::of(i);
        }
    }

    return None();
}
