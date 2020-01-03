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
 *	@file		path.cpp
 *******************************************************************************/
#include "solace/path.hpp"

#include <algorithm>  // std::min/std::max

using namespace Solace;


const StringView Path::Delimiter("/");
const StringView SelfRef(".");
const StringView ParentRef("..");


Path makeRoot() {
	auto root = makePath(StringView{""});
	return root.isOk()
			? root.moveResult()
			: Path{};
}

const Path Path::Root = makeRoot();


Result<Path, Error>
Solace::makePath(StringView str) {
	auto maybeString = makeString(str);
	if (!maybeString) {
		return maybeString.moveError();
	}

	auto maybeArray = makeArrayOf<String>(maybeString.moveResult());
	if (!maybeArray) {
		return maybeArray.moveError();
	}

	return Ok(makePath(maybeArray.moveResult()));
}


Result<Path, Error>
Path::parse(StringView str, StringView delim) {
	Optional<Error> maybeError{};
    Vector<String> nonEmptyComponents;
    str.split(delim, [&](StringView c, StringView::size_type i, StringView::size_type count) {
		if (maybeError) return;

        if (nonEmptyComponents.capacity() == 0 && count != 0) {
			auto maybeComponents = makeVector<String>(count);
			if (maybeComponents) {
				nonEmptyComponents = maybeComponents.moveResult();
			} else {
				maybeError = maybeComponents.moveError();
			}
        }

        if (i + 1 == count && c.empty()) {
            return;
        }

		auto r = makeString(c);
		if (r) {
			nonEmptyComponents.emplace_back(r.moveResult());
		} else {
			maybeError = r.moveError();
		}
    });

	if (maybeError) {
		return maybeError.move();
	}

    if (nonEmptyComponents.empty()) {
		nonEmptyComponents.emplace_back(String{});
    }

    return Ok(Path(nonEmptyComponents.toArray()));
}


Path::Iterator&
Path::Iterator::operator++ () {
	_index += 1;

	return *this;
}


StringView
Path::Iterator::operator-> () const {
	return _path.get().getComponent(_index);
}

String::size_type
Path::length(StringView delim) const noexcept {
    auto const delimLen = delim.length();

    auto const nbComponents = _components.size();
    if (nbComponents == 0) {
        return 0;
    } else if (nbComponents == 1) {
        const auto& one = _components[0];
        return  (one.empty())
                ? delimLen          // Root - single empty item
                : one.length();
    }

    String::size_type len = 0;
    for (auto const& s : _components) {  // Accomulate:
        len += (delimLen + s.length());
    }

    return (len - delimLen);
}


int Path::compareTo(const Path& other) const {
    const size_type minLen = std::min(getComponentsCount(), other.getComponentsCount());

    // Find where this path diverges from other:
    for (size_type i = 0; i < minLen; ++i) {
        const auto& thisComponent = _components[i];
        const auto& otherComponent = other._components[i];

        auto const diff = thisComponent.compareTo(otherComponent);
        if (diff != 0) {
            return diff;
        }
    }


    return static_cast<int>(getComponentsCount()) - static_cast<int>(other.getComponentsCount());
}


bool Path::startsWith(const Path& other) const {
    if (empty())
        return other.empty();

    if (other.empty())
        return empty();

    if (other.length() > length())
        return false;

    auto const nbComponents = other.getComponentsCount();
    for (size_type i = 0; i < nbComponents; ++i) {
        const auto& otherComponent = other._components[i];
        const auto& thisComponent = _components[i];

        if (!thisComponent.equals(otherComponent)) {
            return thisComponent.startsWith(otherComponent) && (i + 1 == nbComponents);
        }
    }

    return true;
}

bool Path::endsWith(const Path& other) const {
    if (empty()) {
        return other.empty();
    }

    if (other.empty()) {
        return empty();
    }

    if (other.length() > length()) {
        return false;
    }

    auto const thisEnd = _components.size();
    auto const nbComponents = other.getComponentsCount();
    for (size_type i = 0; i < nbComponents; ++i) {
        const auto& thisComponent = _components[thisEnd - 1 - i];
        const auto& otherComponent = other._components[nbComponents - 1 - i];

        if (!thisComponent.equals(otherComponent)) {
            return thisComponent.endsWith(otherComponent) && (i + 1 == nbComponents);
        }
    }

    return true;
}

bool Path::contains(const Path& path) const {
    auto const nbOtherComponents = path.getComponentsCount();
    auto const nbThisComponents = getComponentsCount();

    if (nbOtherComponents > nbThisComponents) {
        // Longer path can not be contained in this shorter one
        return false;
    }

    for (size_type firstMatch = 0, nbComponents = nbThisComponents - nbOtherComponents + 1;
         firstMatch < nbComponents;
         ++firstMatch) {

        const auto& a = _components[firstMatch];
        if (a.equals(path._components[0])) {
            bool allMatched = true;
            for (size_type i = 1; i < nbOtherComponents; ++i) {
                const auto& b = _components[firstMatch + i];

                if (!b.equals(path._components[i])) {
                    allMatched = false;
                    break;
                }
            }

            if (allMatched)
                return true;
        }
    }

    return false;
}


bool
Path::isAbsolute() const noexcept {
    return (!empty() && _components[0].empty());
}


bool
Path::isRelative() const noexcept {
    return !isAbsolute();
}


Path
Path::normalize() const {
    // FIXME(abbyssoul): Dynamic memory re-allocation!!!
	auto maybeComponents = makeVector<String>(_components.size());   // Assumption: we don't make path any longer
	if (!maybeComponents) {
		return Path{};  // fixme, maybe return a view or an error
	}

	auto& components = maybeComponents.unwrap();
    for (auto const& c : _components) {
        if (c.equals(SelfRef)) {            // Skip '.' entries
            continue;
        } else if (c.equals(ParentRef) && components.size() > 0) {   // Skip '..' entries
            components.pop_back();
        } else {
			auto r = makeString(c);
			if (r) {
				components.emplace_back(r.moveResult());
			}
        }
    }

	return Path{components.toArray()};
}


Path
Path::getParent() const {
    auto const nbComponents = _components.size();
    if (nbComponents < 2) {
		auto maybeArray = makeArray<String>(_components);  // Copy components vector
		return maybeArray
				? Path{maybeArray.moveResult()}
				: Path{};
    }

    auto const nbBaseComponents = nbComponents - 1;
	auto maybeBasePath = makeVector<String>(nbBaseComponents);
	if (!maybeBasePath) {
		return Path{};
	}

	auto& basePath = maybeBasePath.unwrap();
    // TODO(abbyssoul): Should use array copy
    for (size_type i = 0; i < nbBaseComponents; ++i) {
		auto r = makeString(_components[i]);
		if (r) {
			basePath.emplace_back(r.moveResult());
		}
    }

	return Path{basePath.toArray()};
}


StringView
Path::getBasename() const {
    auto const nbComponents = _components.size();
    return (nbComponents == 1 && _components[0].empty())
            ? Delimiter
            : (nbComponents == 0
               ? String::Empty
               : _components[nbComponents - 1]).view();
}


StringView Path::getComponent(size_type index) const {
	return _components[index].view();
}


Path
Path::subpath(size_type from, size_type to) const noexcept {
    auto const nbComponent = _components.size();

    from = std::min(from, nbComponent);
    to = std::max(from, std::min(to, nbComponent));

	auto maybeComponenets = makeVector<String>(to - from);
	auto& components = maybeComponenets.unwrap();
    for (size_type i = from; i < to; ++i) {
		auto r = makeString(_components[i]);
		if (r) {
			components.emplace_back(r.moveResult());
		}
    }

    return {components.toArray()};
}


bool
Path::equals(Path const& rhv) const noexcept {
    return (&rhv == this) || rhv._components == _components;
}


String
Path::toString(StringView delim) const {
	if (isAbsolute() && _components.size() == 1) {
		auto delimString = makeString(delim);
		return (delimString)
				? delimString.moveResult()
				: String{};
	}


	auto maybePath = makeStringJoin(delim, _components.view());
	return (maybePath)
			? maybePath.moveResult()
			: String{};
}
