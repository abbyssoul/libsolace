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
 * libSolace: ostream output support for libsolace types
 *	@file		solace/output_utils.hpp
 *	@brief		Util functions to out various objects into ostream
 ******************************************************************************/
#pragma once
#ifndef SOLACE_OUTPUT_UTILS_HPP
#define SOLACE_OUTPUT_UTILS_HPP

#include "solace/stringView.hpp"
#include "solace/string.hpp"
#include "solace/path.hpp"
#include "solace/uuid.hpp"
#include "solace/version.hpp"
#include "solace/dialstring.hpp"
#include "solace/optional.hpp"
#include "solace/hashing/messageDigest.hpp"

#include "solace/base16.hpp"

#include <ostream>

namespace Solace {

inline std::ostream& operator<< (std::ostream& ostr, StringLiteral const& str) {
	return ostr.write(str.data(), str.size());
}

inline std::ostream& operator<< (std::ostream& ostr, StringView const& str) {
    return ostr.write(str.data(), str.size());
}

inline std::ostream& operator<< (std::ostream& ostr, MemoryView view) {
    if (view.empty()) {
        return ostr << "<null>";
    }

    // We use custom output printing each byte as \0 bytest and \n are not printable otherwise.
    auto i = base16Encode_begin(view);
    auto const end = base16Encode_end(view);
    for (; i != end; ++i) {
        ostr << *i;
    }

    return ostr;
}


inline std::ostream& operator<< (std::ostream& ostr, String const& str) {
    return ostr << str.view();
}


inline std::ostream& operator<< (std::ostream& ostr, Error const& e) {
    auto const errorDomain = e.domain();
	auto const domain = findErrorDomain(errorDomain);

    if (domain) {
		ostr << (*domain).name();
    } else {
        constexpr auto N = sizeof(AtomValue);
        char buffer[sizeof(N) + 1];
        atomToString(errorDomain, buffer);
        ostr << buffer;
    }

    ostr << ':' << e.value() << ':';

    if (domain) {
		ostr << (*domain).message(e.value());
    }

    auto const tag = e.tag();
    if (!tag.empty()) {
        ostr << ':' << tag;
    }

    return ostr;
}

inline std::ostream&
operator<< (std::ostream& ostr, hashing::MessageDigest const& a) {
    ostr << '[';

    auto const dataView = a.view();
    for (auto i = base16Encode_begin(dataView),
         end = base16Encode_end(dataView);
         i != end; ++i) {
        auto const encodedView = (*i).view();
        ostr << "0x";
		ostr.write(static_cast<char const*>(encodedView.dataAddress()), encodedView.size());
    }

    ostr << ']';

    return ostr;
}


inline std::ostream& operator<< (std::ostream& ostr, Path const& v) {
    return ostr << v.toString();
}


inline std::ostream& operator<< (std::ostream& ostr, UUID const& v) {
    return ostr << v.toString();
}

inline std::ostream& operator<< (std::ostream& ostr, Version const& v) {
    return ostr << v.toString();
}


inline std::ostream& operator<< (std::ostream& ostr, DialString const& ds) {
	constexpr auto N = sizeof(AtomValue);
	char buff[sizeof(N) + 1];
	atomToString(ds.protocol, buff);

	ostr << buff << ':' << ds.address;
	if (!ds.service.empty()) {
		ostr << ':' << ds.service;
	}

	return ostr;
}



// TODO(abbyssoul): Should be in a separate file, if at all
template <typename T>
std::ostream& operator<< (std::ostream& ostr, const Optional<T>& anOptional) {
    return (anOptional.isNone())
            ? ostr.write("None", 4)
            : ostr << anOptional.get();
}

}  // namespace Solace
#endif  // SOLACE_OUTPUT_UTILS_HPP
