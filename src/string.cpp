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
 *	@file		solace/string.cpp
 *	@brief		Implementation of fixed size String.
 ******************************************************************************/
#include "solace/string.hpp"


using namespace Solace;


const String String::Empty{};


Result<String, Error>
Solace::makeString(StringView str) {
	auto buffer = getSystemHeapMemoryManager().allocate(str.size() * sizeof(StringView::value_type));    // May throw

    // Copy string view content into a new buffer
	auto writeResult = buffer.view().write(str.view());
	if (!writeResult)
		return writeResult.getError();

	return Ok(String{mv(buffer), str.size()});
}


String
Solace::makeString(StringLiteral literal) noexcept {
    auto view = literal.view();
	auto buffer = MemoryResource{wrapMemory(const_cast<MemoryView::value_type*>(view.dataAddress()), view.size()),
								 nullptr};

	return { mv(buffer), literal.size() };
}


Result<String, Error>
Solace::makeStringReplace(StringView str, String::value_type what, String::value_type with) {
    auto const totalStrLen = str.size();
    auto buffer = getSystemHeapMemoryManager().allocate(totalStrLen * sizeof(StringView::value_type));    // May throw
    auto bufferView = buffer.view();

	auto writeResult = buffer.view().write(str.view());
	if (!writeResult)
		return writeResult.getError();

    for (StringView::size_type to = 0; to < totalStrLen; ++to) {
        auto offsetValue = bufferView.dataAs<StringView::value_type>(to);
        if (*offsetValue == what) {
            *offsetValue = with;
        }
    }

	return Ok(String{ mv(buffer), totalStrLen });
}


Result<String, Error> Solace::makeStringReplace(StringView str, StringView what, StringView by) {
    auto const srcStrLen = str.size();
    auto const delimLength = what.size();
    StringView::size_type delimCount = 0;

    for (StringView::size_type i = 0; i < srcStrLen && i + delimLength <= srcStrLen; ++i) {
        if (what.equals(str.substring(i, i + delimLength))) {
            delimCount += 1;
            i += delimLength - 1;
        }
    }

    auto const byLen = by.size();

    // Note:  srcStrLen >= delimLength*delimCount. Thus this should not overflow.
    auto const newStrLen = narrow_cast<StringView::size_type>(srcStrLen + byLen * delimCount - delimLength*delimCount);

    auto buffer = getSystemHeapMemoryManager().allocate(newStrLen * sizeof(StringView::value_type));    // May throw
    auto writer = ByteWriter(buffer.view());

    StringView::size_type from = 0;
    for (StringView::size_type to = 0; to < srcStrLen && to + delimLength <= srcStrLen; ++to) {
        if (what.equals(str.substring(to, to + delimLength))) {
            auto const tokLen = narrow_cast<StringView::size_type>(to - from);

            writer.write(str.substring(from, from + tokLen).view());
            writer.write(by.view());

            to += delimLength - 1;
            from = to + 1;
        }
    }

	auto result = writer.write(str.substring(from).view());
	if (!result) {
		return result.moveError();
	}

	return Ok(String{ mv(buffer), newStrLen });
}


bool String::equals(StringView v) const noexcept {
    return view().equals(v);
}

int String::compareTo(StringView other) const noexcept {
    return view().compareTo(other);
}

String::value_type
String::charAt(size_type index) const {
    return view().charAt(index);
}

Optional<String::size_type>
String::indexOf(value_type ch, size_type fromIndex) const noexcept {
    return view().indexOf(ch, fromIndex);
}


Optional<String::size_type>
String::indexOf(StringView str, size_type fromIndex) const noexcept {
    return view().indexOf(str, fromIndex);
}

Optional<String::size_type>
String::lastIndexOf(value_type ch, size_type fromIndex) const noexcept {
    return view().lastIndexOf(ch, fromIndex);
}

Optional<String::size_type>
String::lastIndexOf(StringView str, size_type fromIndex) const noexcept {
    return view().lastIndexOf(str, fromIndex);
}


bool String::startsWith(StringView prefix) const noexcept {
    return view().startsWith(prefix);
}

bool String::startsWith(value_type prefix) const noexcept {
    return view().startsWith(prefix);
}

bool String::endsWith(StringView suffix) const noexcept {
    return view().endsWith(suffix);
}

bool String::endsWith(value_type suffix) const noexcept {
    return view().endsWith(suffix);
}


uint64
String::hashCode() const noexcept {
    return view().hashCode();
}


StringView
String::trim() const noexcept {
    return view().trim();
}


/** Return jointed string from the given collection */
Result<String, Error> Solace::makeStringJoin(StringView by, ArrayView<const String> list) {
    auto totalStrLen = narrow_cast<StringView::size_type>(by.size() * (list.size() - 1));
    for (auto const& i : list) {
        totalStrLen += i.size();
    }

    auto buffer = getSystemHeapMemoryManager().allocate(totalStrLen * sizeof(StringView::value_type));    // May throw
    auto writer = ByteWriter(buffer.view());

    // Copy string view content into a new buffer
    auto count = list.size();
    for (auto const& i : list) {
        auto const iView = i.view();
		auto r = writer.write(iView.view());
		if (!r)
			return r.moveError();

        count -= 1;

        if (count > 0) {
			r = writer.write(by.view());
			if (!r)
				return r.moveError();
		}
    }

	return Ok(String{ mv(buffer), totalStrLen });
}



/** Return String representation of boolen value **/
/*
String String::valueOf(bool value) {
    return (value)
            ? makeString(TRUE_STRING)
            : makeString(FALSE_STRING);
}


String String::valueOf(StringView value) {
    return makeString(value);
}

String String::valueOf(int32 val) {
    return String{ std::to_string(val) };
}

String String::valueOf(int64 val) {
    return String{ std::to_string(val) };
}

String String::valueOf(uint32 val) {
    return String{ std::to_string(val) };
}

String String::valueOf(uint64 val) {
    return String{ std::to_string(val) };
}

String String::valueOf(float32 val) {
    return String{ std::to_string(val) };
}

String String::valueOf(float64 val) {
    return String{ std::to_string(val) };
}
*/
