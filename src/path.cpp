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


using namespace Solace;


const StringView Path::Delimiter("/");
const StringView SelfRef(".");
const StringView ParentRef("..");


const Path Path::Root = allocPath(StringLiteral{""});






template<typename T>
void joinComponents(Vector<String>& base, std::initializer_list<T> paths) {
    for (auto& component : paths) {
        base.emplace_back(std::move(component));
    }
}


template<typename T>
Vector<String> joinComponents(std::initializer_list<T> paths) {
    auto components = makeVector<String>(paths.size());

    joinComponents(components, paths);

    return components;
}


template<>
Vector<String> joinComponents(std::initializer_list<Path> paths) {
    size_t numberOfComponents = 0;
    // Count number of components
    for (const auto& p : paths) {
        numberOfComponents += p.getComponentsCount();
    }

    auto components = makeVector<String>(numberOfComponents);
    for (auto& path : paths) {
        for (auto& component : path) {
            components.emplace_back(std::move(component));
        }
    }

    return components;
}



Path Solace::allocPath(StringView str) {
    return allocPath(makeVector<String>({str}));
}



Path Solace::allocPath(Path const& base, Path const& rhs) {
    auto components = makeVector<String>(base.getComponentsCount() + rhs.getComponentsCount());

    for (auto const& c : base) {
        components.emplace_back(c);
    }
    for (const auto& c : rhs) {
        components.emplace_back(c);
    }

    return allocPath(std::move(components));
}

Path Solace::allocPath(Path const& base, std::initializer_list<Path> paths) {
    Path::size_type numberOfComponents = base.getComponentsCount();
    // Count number of components
    for (const auto& p : paths) {
        numberOfComponents += p.getComponentsCount();
    }

    // Now we can pr-eallocate storage for components
    auto components = makeVector<String>(numberOfComponents);

    for (auto& path : paths) {
        for (auto& component : path) {
            components.emplace_back(std::move(component));
        }
    }

    return allocPath(std::move(components));
}


Path Solace::allocPath(std::initializer_list<Path> paths) {
    return allocPath(joinComponents(paths));
}



Path Solace::allocPath(Path const& base, StringView rhs) {
    auto components = makeVector<String>(base.getComponentsCount() + 1);

    for (auto const& c : base) {
        components.emplace_back(c);
    }

    components.emplace_back(rhs);

    return allocPath(std::move(components));
}


Path Solace::allocPath(Path const& base, std::initializer_list<StringView> paths) {
    auto components = makeVector<String>(base.getComponentsCount() + paths.size());

    for (auto const& component : base) {
        components.emplace_back(component);
    }

    joinComponents(components, paths);


    return allocPath(std::move(components));
}


Path Solace::allocPath(std::initializer_list<StringView> paths) {
    return allocPath(joinComponents(paths));
}


Path Solace::allocPath(Path const& base, std::initializer_list<String> paths) {
    auto components = makeVector<String>(base.getComponentsCount() + paths.size());

    for (auto const& component : base) {
        components.emplace_back(component);
    }

    joinComponents(components, paths);

    return allocPath(std::move(components));
}

Path Solace::allocPath(std::initializer_list<String> paths) {
    return allocPath(makeVector<String>(paths));
}


Path Solace::allocPath(Path const& base, std::initializer_list<const char*> paths) {
    auto components = makeVector<String>(base.getComponentsCount() + paths.size());

    for (auto& component : base) {
        components.emplace_back(component);
    }

    joinComponents(components, paths);

    return allocPath(std::move(components));
}


Path Solace::allocPath(std::initializer_list<const char*> paths) {
    return allocPath(joinComponents(paths));
}


Result<Path, Error>
Path::parse(StringView str, StringView delim) {
    Vector<String> nonEmptyComponents;
    str.split(delim, [&](StringView c, StringView::size_type i, StringView::size_type count) {
        if (nonEmptyComponents.capacity() == 0 && count != 0) {
            nonEmptyComponents = makeVector<String>(count);
        }

        if (i + 1 == count && c.empty()) {
            return;
        }

        nonEmptyComponents.emplace_back(c);
    });

    if (nonEmptyComponents.empty()) {
        nonEmptyComponents.emplace_back(String::Empty);
    }

    return Ok(Path(std::move(nonEmptyComponents)));
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

        const auto diff = thisComponent.compareTo(otherComponent);
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

    const auto nbComponents = other.getComponentsCount();
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

    const auto thisEnd = _components.size();
    const auto nbComponents = other.getComponentsCount();
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
    const auto nbOtherComponents = path.getComponentsCount();
    const auto nbThisComponents = getComponentsCount();

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
    auto components = makeVector<String>(_components.size());   // Assumption: we don't make path any longer

    for (auto const& c : _components) {
        if (c.equals(SelfRef)) {            // Skip '.' entries
            continue;
        } else if (c.equals(ParentRef) && components.size() > 0) {   // Skip '..' entries
            components.pop_back();
        } else {
            components.emplace_back(c);
        }
    }

    return Path(std::move(components));
}


Path
Path::getParent() const {
    auto const nbComponents = _components.size();
    if (nbComponents < 2) {
        return {makeVector<String>(_components)};  // Copy components vector
    }

    auto const nbBaseComponents = nbComponents - 1;
    auto basePath = makeVector<String>(nbBaseComponents);
    // TODO(abbyssoul): Should use array copy
    for (size_type i = 0; i < nbBaseComponents; ++i) {
        basePath.emplace_back(_components[i]);
    }

    return Path(std::move(basePath));
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


Path::size_type
Path::getComponentsCount() const noexcept {
    return _components.size();
}

Solace::String const&
Path::getComponent(size_type index) const {
    return _components[index];
}


Path
Path::subpath(size_type beginIndex, size_type endIndex) const {
    const auto nbComponent = _components.size();
    if (beginIndex > nbComponent) {
        raise<IndexOutOfRangeException>(beginIndex, 0, nbComponent);
    }

    if (endIndex > nbComponent) {
        raise<IndexOutOfRangeException>(endIndex, 0, nbComponent);
    }

    if (beginIndex > endIndex) {
        raise<IndexOutOfRangeException>(beginIndex, 0, endIndex);
    }

    auto components = makeVector<String>(endIndex - beginIndex);
    for (size_type i = beginIndex; i < endIndex; ++i) {
        components.emplace_back(_components[i]);
    }

    return Path(std::move(components));
}


bool
Path::equals(Path const& rhv) const noexcept {
    return (&rhv == this) || rhv._components == _components;
}


String
Path::toString(StringView delim) const {
    return (isAbsolute() && _components.size() == 1)
            ? delim
            : String::join(delim, arrayView(_components.data(), _components.size()));
}
