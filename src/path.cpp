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
 *
 *  Created by soultaker on 8/03/16
******************************************************************************/
#include "solace/path.hpp"
#include "solace/exception.hpp"


using Solace::Path;
using Solace::String;
using Solace::Array;

const String Path::Delimiter("/");
const String SelfRef(".");
const String ParentRef("..");


const Path Path::Root("");



bool Path::isAbsolute() const noexcept {
    return (!empty() && _components.first().empty());
}

bool Path::isRelative() const noexcept {
    return !isAbsolute();
}


Path Path::join(std::initializer_list<Path> paths) {
    size_type numberOfComponents = 0;
    for (const auto& p : paths) {
        numberOfComponents += p.getComponentsCount();
    }

    Array<String> components(numberOfComponents);
    size_type i = 0;
    for (const auto& path : paths) {
        for (const auto& component : path) {
            components[i++] = component;
        }
    }

    return Path(components);
}

Path Path::join(std::initializer_list<String> paths) {
    Array<String> components(paths);

    return Path(components);
}

Path Path::parse(const String& str, const String& delim) {
    const auto components = str.split(delim);

    return {components};
}


String::size_type Path::length(const String& delim) const noexcept {
    const auto delimLen = delim.length();

    const auto nbComponents = _components.size();
    if (nbComponents == 0) {
        return 0;
    } else if (_components.size() == 1) {
        const auto& one = _components.first();
        if (one.empty()) {  // Absolute
            return delimLen;
        } else {
            return one.length();
        }
    }

    String::size_type len = 0;
    for (const auto& s : _components) {
        len += (delimLen + s.length());
    }

    return len - delimLen;
}


int Path::compareTo(const Path& other) const {
    const size_type minLen = std::min(getComponentsCount(), other.getComponentsCount());
    int difference = getComponentsCount() - other.getComponentsCount();

    // Find where this path diverges from other:
    for (size_type i = 0; i < minLen; ++i) {
        const auto& thisComponent = _components[i];
        const auto& otherComponent = other._components[i];

        difference += thisComponent.compareTo(otherComponent);
    }

    return difference;
}


bool Path::startsWith(const Path& other) const {
    if (empty())
        return other.empty();

    if (other.empty())
        return empty();

    if (other.length() > length())
        return false;

    for (size_type i = 0, end = other.getComponentsCount(); i < end; ++i) {
        const auto& otherComponent = other._components[i];
        const auto& thisComponent = _components[i];

        if (!thisComponent.equals(otherComponent)) {
            return thisComponent.startsWith(otherComponent) && (i + 1 == end);
        }
    }

    return true;
}

bool Path::endsWith(const Path& other) const {
    if (empty())
        return other.empty();

    if (other.empty())
        return empty();

    if (other.length() > length())
        return false;

    const auto thisEnd = _components.size();
    for (size_type i = 0, end = other.getComponentsCount(); i < end; ++i) {
        const auto& thisComponent = _components[thisEnd - 1 - i];
        const auto& otherComponent = other._components[end - 1 - i];

        if (!thisComponent.equals(otherComponent)) {
            return thisComponent.endsWith(otherComponent) && (i + 1 == end);
        }
    }

    return true;
}

bool Path::contains(const Path& path) const {
    const auto nbOtherComponents = path.getComponentsCount();
    const auto nbThisComponents = getComponentsCount();

    if (nbOtherComponents > nbThisComponents) {
        // Longer path can not be contained in this shorter one
        return false;
    }

    for (size_type firstMatch = 0, end = nbThisComponents - nbOtherComponents + 1;
         firstMatch < end;
         ++firstMatch) {

        const auto& a = _components[firstMatch];
        if (a.equals(path._components.first())) {
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

Path Path::normalize() const {
    // FIXME: Dynamic memory re-allocation!!!
    std::vector<String> components;

    for (const auto& c : _components) {
        if (c.equals(SelfRef)) {
            continue;
        } else if (c.equals(ParentRef)) {
            components.pop_back();
        } else {
            components.push_back(c);
        }
    }

    return Path(components);
}

Path Path::getParent() const {
    const auto nbComponents = _components.size();
    if (nbComponents < 2) {
        return *this;
    }

    Array<String> basePath(_components.size() - 1);
    // TODO(abbyssoul): Should use array copy
    for (size_type i = 0; i < basePath.size(); ++i) {
        basePath[i] = _components[i];
    }

    return Path(basePath);
}

Path::size_type Path::getComponentsCount() const noexcept {
    return _components.size();
}

String Path::getComponent(size_type index) const {
    return _components[index];
}


Path Path::subpath(size_type beginIndex, size_type endIndex) const {

    const auto nbComponent = _components.size();
    if (beginIndex > nbComponent)
        raise<IndexOutOfRangeException>(beginIndex, 0, nbComponent);

    if (endIndex > nbComponent)
        raise<IndexOutOfRangeException>(endIndex, 0, nbComponent);

    if (beginIndex > endIndex)
        raise<IndexOutOfRangeException>(beginIndex, 0, endIndex);

    const auto newSize = endIndex - beginIndex;
    Array<String> components(newSize);
    for (size_type i = 0; i < newSize; ++i) {
        components[i] = _components[beginIndex + i];
    }

    return Path(components);
}

Path Path::join(const Path& rhs) const {
    return Path::join({*this, rhs});
}

Path Path::join(const String& rhs) const {
    Array<String> components(getComponentsCount() + 1);

    size_type i = 0;
    for (const auto& c : _components) {
        components[i++] = c;
    }
    components[i++] = rhs;

    return Path(components);
}


bool Path::equals(const Path& rhv) const noexcept {
    return (&rhv == this) || rhv._components.equals(_components);
}


String Path::first() const {
    return empty()
            ? String::Empty
            : _components.first();
}


String Path::last() const {
    return empty()
            ? String::Empty
            : _components.last();
}

const Path& Path::forEach(const std::function<void(const String&)> &f) const {
    _components.forEach(f);

    return *this;
}

String Path::toString(const String& delim) const {
    return (isAbsolute() && _components.size() == 1)
            ? delim
            : String::join(delim, _components);
}
