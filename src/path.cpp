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


const Path Path::Root(StringView{""});




template<typename T>
void joinComponents(std::vector<String>& base, std::initializer_list<T> paths) {
    for (auto& component : paths) {
        base.emplace_back(std::move(component));
    }
}


template<typename T>
std::vector<String> joinComponents(std::initializer_list<T> paths) {
    std::vector<String> components;
    components.reserve(paths.size());

    joinComponents(components, paths);

    return components;
}


template<>
std::vector<String> joinComponents(std::initializer_list<Path> paths) {
    size_t numberOfComponents = 0;
    // Count number of components
    for (const auto& p : paths) {
        numberOfComponents += p.getComponentsCount();
    }

    std::vector<String> components;
    components.reserve(numberOfComponents);

    for (auto& path : paths) {
        for (auto& component : path) {
            components.emplace_back(std::move(component));
        }
    }

    return components;
}




bool Path::isAbsolute() const noexcept {
    return (!empty() && _components.front().empty());
}

bool Path::isRelative() const noexcept {
    return !isAbsolute();
}


Path
Path::join(Path const& base, std::initializer_list<Path> paths) {
    size_type numberOfComponents = base.getComponentsCount();
    // Count number of components
    for (const auto& p : paths) {
        numberOfComponents += p.getComponentsCount();
    }

    // Now we can pr-eallocate storage for components
    std::vector<String> components;
    components.reserve(numberOfComponents);

    for (auto& path : paths) {
        for (auto& component : path) {
            components.emplace_back(std::move(component));
        }
    }

    return Path(std::move(components));
}


Path
Path::join(Path const& base, std::initializer_list<String> paths) {
    std::vector<String> components;
    components.reserve(base.getComponentsCount() + paths.size());

    for (auto& component : base) {
        components.push_back(component);
    }

    joinComponents(components, paths);

    return Path(std::move(components));
}


Path
Path::join(Path const& base, std::initializer_list<StringView> paths) {
    std::vector<String> components;
    components.reserve(base.getComponentsCount() + paths.size());

    for (auto& component : base) {
        components.push_back(component);
    }

    joinComponents(components, paths);


    return Path(std::move(components));
}


Path
Path::join(Path const& base, std::initializer_list<const char*> paths) {
    std::vector<String> components;
    components.reserve(base.getComponentsCount() + paths.size());

    for (auto& component : base) {
        components.push_back(component);
    }

    joinComponents(components, paths);

    return Path(std::move(components));
}


Result<Path, Error>
Path::parse(StringView str, StringView delim) {
    std::vector<String> nonEmptyComponents;
    str.split(delim, [&](StringView c, StringView::size_type i, StringView::size_type count) {
        if (i + 1 == count && c.empty())
            return;

        nonEmptyComponents.emplace_back(c);
    });

    return nonEmptyComponents.empty()
            ? Ok(Root)
            : Ok(Path(std::move(nonEmptyComponents)));
}


Path::Path(std::initializer_list<Path> paths) : _components(joinComponents(paths)) {
}


Path::Path(std::initializer_list<StringView> paths) : _components(joinComponents(paths)) {
    // No-op
}

Path::Path(std::initializer_list<const char*> paths) : _components(joinComponents(paths)) {
    // No-op
}


String::size_type
Path::length(const StringView& delim) const noexcept {
    const auto delimLen = delim.length();

    const auto nbComponents = _components.size();
    if (nbComponents == 0) {
        return 0;
    } else if (_components.size() == 1) {
        const auto& one = _components.front();
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
    if (empty())
        return other.empty();

    if (other.empty())
        return empty();

    if (other.length() > length())
        return false;

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
        if (a.equals(path._components.front())) {
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
    // FIXME(abbyssoul): Dynamic memory re-allocation!!!
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

    return Path(std::move(components));
}

Path Path::getParent() const {
    const auto nbComponents = _components.size();
    if (nbComponents < 2) {
        return *this;
    }

    auto const nbBaseComponents = _components.size() - 1;
    std::vector<String> basePath;
    basePath.reserve(nbBaseComponents);
    // TODO(abbyssoul): Should use array copy
    for (size_type i = 0; i < nbBaseComponents; ++i) {
        basePath.push_back(_components[i]);
    }

    return Path(std::move(basePath));
}

StringView
Path::getBasename() const {
    return (isAbsolute() && _components.size() == 1)
            ? Delimiter
            : last().view();
}


Path::size_type Path::getComponentsCount() const noexcept {
    return _components.size();
}

const Solace::String& Path::getComponent(size_type index) const {
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

    std::vector<String> components;
    components.reserve(endIndex - beginIndex);
    for (size_type i = beginIndex; i < endIndex; ++i) {
        components.push_back(_components[i]);
    }

    return Path(std::move(components));
}

Path Path::join(const Path& rhs) const {
    std::vector<String> components;
    components.reserve(getComponentsCount() + rhs.getComponentsCount());

    for (const auto& c : _components) {
        components.emplace_back(c);
    }
    for (const auto& c : rhs._components) {
        components.emplace_back(c);
    }

    return Path{std::move(components)};
}

Path Path::join(StringView rhs) const {
    std::vector<String> components;
    components.reserve(getComponentsCount() + 1);

    for (const auto& c : _components) {
        components.emplace_back(c);
    }
    components.emplace_back(rhs);

    return Path{std::move(components)};
}

Path Path::join(std::initializer_list<StringView> rhs) const {
    return Path::join(*this, rhs);
}



bool Path::equals(Path const& rhv) const noexcept {
    return (&rhv == this) || rhv._components == _components;
}


const String& Path::first() const {
    return empty()
            ? String::Empty
            : _components.front();
}


const String& Path::last() const {
    return empty()
            ? String::Empty
            : _components.back();
}


String
Path::toString(StringView delim) const {
    return (isAbsolute() && _components.size() == 1)
            ? delim
            : String::join(delim, arrayView(_components.data(), _components.size()));
}
