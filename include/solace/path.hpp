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
 * libSolace: Hierarchical path object
 *	@file		solace/path.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	@brief		Immutable hierarchical Path class
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_PATH_HPP
#define SOLACE_PATH_HPP

#include "solace/string.hpp"
#include "solace/array.hpp"
#include "solace/result.hpp"
#include "solace/error.hpp"
#include "solace/vector.hpp"


namespace Solace {

/** Hierarchical path class - the kind used by File system, but not exactly
 * Path is an ordered sequence of strings/names that can be represented by a string,
 * e.g Formattable and Parsable object.
 *
 * Examples of Paths (first 3 with the same '/' as a delimiter):
 * 	* File system path string: /etc/config.json
 * 	* URL path component: /webapp/action/index.html
 * 	* Tree element path: <root>/node x/taget_node
 * 	* Web host name also a path: www.blah.example.com
 * 	* Java package name: org.java.AwesomePackages
 *
 * Note: that path object is designed to be immutable, e.g it cannot be changed once created,
 * It can be appended to / subtracted from - creating a new object (kind of like a linked list).
 * Being immutable means that any such addition or subtraction will produce a new object.
 *
 * Note: Path is an abstraction and is not designed to be compatible with
 * the underlying filesystem representation of a file path.
 * Wherever possible file objects can be created from a file system path,
 * but no direct compatibility is designed.
 * This also implies that functions line noramalize don't do file system travesal,
 * but operate on path string components only.
 */
class Path {
public:

    using value_type = String;
    using size_type = Array<value_type>::size_type;
    using const_iterator = Array<value_type>::const_iterator;

public:  // Static methods

	/**
	 * Default delimiter used form string representation of the path
	 * Note: might be different from platform delimiter
	 */
    static const StringView Delimiter;

    /**
     * Root path object
     */
    static const Path Root;

    /**
     * Parse a path object from a string.
     *
     * @param str A string to parse
     * @param delim A delimiter used to separate path components
     * @return Parsed path object
     *
     * TODO: Parse family of functions should return Result<Path, ParseError>
     */
    static Result<Path, Error>
    parse(StringView str, StringView delim = Delimiter);

public:  // Object construction

	/** Construct an empty path */
    Path() = default;

    /** Construct an object by moving content from a given */
    Path(Path&& p) noexcept: _components(std::move(p._components)) {
        // No-op
    }

public:  // Operation

    Path& swap(Path& rhs) noexcept {
        _components.swap(rhs._components);

        return (*this);
    }

    /**
     * Move assignement.
     * @param rhs An object to move content from.
     * @return A reference to this.
     */
    Path& operator= (Path&& rhs) noexcept {
        return swap(rhs);
    }

    /** Test if the path is empty.
     *
     * Path is empty only when it has no components.
     * @return True if the path object is empty.
     */
    bool empty() const noexcept {
        return _components.empty();
    }

    /** Test if path is not empty.
     * @return True if this path is not an empty object.
     */
    explicit operator bool() const {
      return !empty();
    }


    /** Tests this path for equality with the given object.
     * @param rhv A path to compare this one to.
     * @return True if this path is equal to the give
     */
    bool equals(Path const& rhv) const noexcept;


    /**
     * Test if the path is absolute.
     *
     * Asbolute path is the one that starts with the root.
     * @return True if the path object represents absolute path
     */
    bool isAbsolute() const noexcept;

    /**
     * Test if the path is relative.
     *
     * Relative path is the one that doe not starts with the root.
     * @return True if the path object represents relative path
     */
    bool isRelative() const noexcept;

    /**
     * Get the lenght of the string representation of this path.
     *
     * @note: To get the number of components, please @see getComponentsCount
     * @return Size of the string representation of the path in characters.
     */
    String::size_type length(StringView delim = Delimiter) const noexcept;

    //--------------------------------------------------------------------------
	// --- Relational collection operations
	//--------------------------------------------------------------------------
    /** Compares two paths lexicographically.
	 * The comparison is based on the Unicode value
     * of each character in the strings that make up path parts.
     * The result is zero if two paths are equal:
	 * exactly when the equals(other) would return true.
	 *
	 * @return The value 0 if the argument string is equal to this string;
	 * @return a value less than 0 if this string is lexicographically less than the string argument;
	 * @return and a value greater than 0 if this string is lexicographically greater than the string argument.
	 */
    int compareTo(const Path& other) const;

    /** Tests if this path starts with the given path.
     * @param other
     * @return True, if this path object starts with the given
     */
    bool startsWith(const Path& other) const;

    /** Tests if this path string representation starts with the string given.
     * @param str A string to test
     * @return True, if this path object starts with the given string
     */
    bool startsWith(StringView str) const {
        // FIXME: Wasteful allocation of string representation
        return toString().startsWith(str);
    }

    /** Tests if this path ends with the given path.
     * @param other
     * @return True, if this path object ends with the given
     */
    bool endsWith(const Path& other) const;

    /** Tests if this path ends with the given string.
     * @param other
     * @return True, if this path object ends with the given
     */
    bool endsWith(StringView other) const {
        // FIXME: Wasteful allocation of string representation
        return toString().endsWith(other);
    }

	/** Determine if the path contains a given subpath.
	 *
	 * @param path The subpath to search for.
	 * @return <b>true</b> if the path contains the subpath, <b>false</b> otherwise.
	 */
    bool contains(const Path& path) const;

    /** Determine if the path string representation contains a given substring.
     *
     * @param str The substring to search for.
     * @return <b>true</b> if the path contains the substring, <b>false</b> otherwise.
     */
    bool contains(StringView str) const {
        // FIXME: Wasteful allocation of string representation
        return toString().contains(str);
    }
    /** Determine if the path string representation contains a given substring.
     *
     * @param str The substring to search for.
     * @return <b>true</b> if the path contains the substring, <b>false</b> otherwise.
     */
    bool contains(String const& str) const {
        return contains(str.view());
    }

    /** Returns a path that is this path with redundant name elements eliminated.
     * Self reference (usually '.') and parent reference (usually '..') are
     * considered redundunt elements and in most cases can be removed.
     *
     * @note The mothod does not consult file system and operates on the string only.
     * As such it can result in files that don't exist.
     *
     * @return path that is this path with redundant name elements eliminated.
     */
    Path normalize() const;

    // ---- decomposition ----
    /** Get parent path or null path if this is the root
     * @return Parent of this path or null
     */
    Path getParent() const;

    /**
     * Returns the name of the object this path leads to
     * @return The last element of the name sequence
     */
    StringView getBasename() const;

    /** Get number of components this path includes
     * @brief getComponentsCount
     * @return Number of path elements in this path
     */
    constexpr size_type getComponentsCount() const noexcept {
        return _components.size();
    }

    /** Get number of components this path includes
     * @brief getComponentsCount
     * @return Number of path elements in this path
     */
    const String& getComponent(size_type index) const;

    const_iterator begin() const {
        return _components.begin();
    }

    const_iterator end() const {
        return _components.end();
    }

    /** Returns sub path of this path
     * Splice of this path object
     * @return Sub path of this path
     */
    Path subpath(size_type beginIndex, size_type endIndex) const;


    /** @see Iterable::forEach */
    template<typename F>
    std::enable_if_t<isCallable<F, const value_type&>::value, const Path& >
    forEach(F&& f) const {
        for (auto const& i : _components) {
            f(i);
        }

        return *this;
    }

    template<typename F>
    std::enable_if_t<isCallable<F, value_type&&>::value, Path& >
    forEach(F&& f) && {
        for (auto& i : _components) {
            f(std::move(i));
        }

        return *this;
    }


    /** Get string representation of the path object using give delimiter */
    String toString(StringView delim) const;

    /** @see IFormattable::toString() */
    String toString() const {
        return toString(Delimiter);
    }

protected:
    friend Path makePath(Array<String>&& array);

    /** FIXME(abbyssoul): Only temporary here. to be removed
     * Move-Construct the path object from a collection of String components
     * @param array A collection of string components forming the path
     */
    Path(Array<String>&& array): _components(std::move(array)) {
        // No-op
    }


private:

    Array<String>  _components{};
};


inline
bool operator== (Path const& lhs, Path const& rhv) noexcept {
    return lhs.equals(rhv);
}

inline
bool operator!= (Path const& lhs, Path const& rhv) noexcept {
    return !lhs.equals(rhv);
}

inline
void swap(Path& lhs, Path& rhs) noexcept {
    lhs.swap(rhs);
}

/**
 * Construct the path object from a single string component
 *
 * @note The string is is parsed into component, please use Path::parse
 */
Path makePath(StringView str);

inline
Path makePath(String const& str) {
    return makePath(str.view());
}

inline
Path makePath(char const* str) {
    return makePath(StringView{str});
}

inline
Path makePath(Array<String>&& array) {
    return {std::move(array)};
}

inline
Path makePath(Vector<String>&& vec) {
    return makePath(vec.toArray());
}


namespace details {

constexpr Path::size_type countPathComponents()                 noexcept { return 0; }
constexpr Path::size_type countPathComponents(StringView)       noexcept { return 1; }
constexpr Path::size_type countPathComponents(StringLiteral)    noexcept { return 1; }
constexpr Path::size_type countPathComponents(String const&)    noexcept { return 1; }
constexpr Path::size_type countPathComponents(String&)          noexcept { return 1; }
constexpr Path::size_type countPathComponents(char const*)      noexcept { return 1; }
constexpr Path::size_type countPathComponents(Path const& path) noexcept { return path.getComponentsCount(); }
constexpr Path::size_type countPathComponents(Path& path)       noexcept { return path.getComponentsCount(); }

template<typename T, typename...Args>
Path::size_type countPathComponents(T&& base, Args&&...args) {
    return (countPathComponents(base) + countPathComponents(std::forward<Args>(args)...));
}


inline void joinComponents(Vector<String>& base, StringView view) {
    base.emplace_back(makeString(view));
}

inline void joinComponents(Vector<String>& base, String&& str) {
    base.emplace_back(std::move(str));
}

inline void joinComponents(Vector<String>& base, String const& str) {
    base.emplace_back(makeString(str));
}

inline void joinComponents(Vector<String>& base, Path&& path) {
    std::move(path).forEach([&base](Path::value_type&& component) {
        base.emplace_back(std::move(component));
    });
}

inline void joinComponents(Vector<String>& base, Path const& path) {
    for (auto const& component : path) {
        base.emplace_back(makeString(component));
    }
}


template <typename...Args>
void joinComponents(Vector<String>& base, StringView view, Args&&...args) {
    joinComponents(base, view);
    joinComponents(base, std::forward<Args>(args)...);
}

template <typename...Args>
void joinComponents(Vector<String>& base, String const& view, Args&&...args) {
    joinComponents(base, view);
    joinComponents(base, std::forward<Args>(args)...);
}


template <typename...Args>
void joinComponents(Vector<String>& base, Path const& path, Args&&...args) {
    joinComponents(base, path);
    joinComponents(base, std::forward<Args>(args)...);
}

}  // namespace details

/*
template<typename...Args>
Path makePath(Path const& base, Args&&...args) {
    auto components = makeVector<String>(details::countComponentes(base, std::forward<Args>(args)...));
    details::joinComponents(components, base, std::forward<Args>(args)...);

    return makePath(std::move(components));
}
*/

template<typename...Args>
Path makePath(Args&&...args) {
    auto components = makeVector<Path::value_type>(details::countPathComponents(std::forward<Args>(args)...));
    details::joinComponents(components, std::forward<Args>(args)...);

    return makePath(std::move(components));
}


/**
 * Join paths objects into a single path
 */
/*
Path makePath(Path const& base, Path const& rhs);
Path makePath(Path const& base, std::initializer_list<Path> paths);
Path makePath(std::initializer_list<Path> components);

Path makePath(Path const& base, StringView rhs);
Path makePath(Path const& base, std::initializer_list<StringView> paths);
Path makePath(std::initializer_list<StringView> components);

inline
Path makePath(Path const& base, String const& rhs) { return makePath(base, rhs.view()); }
Path makePath(Path const& base, std::initializer_list<String> paths);
Path makePath(std::initializer_list<String> components);

inline
Path makePath(Path const& base, char const* rhs) { return makePath(base, StringView(rhs)); }
Path makePath(Path const& base, std::initializer_list<const char*> paths);
Path makePath(std::initializer_list<const char*> components);
*/
}  // namespace Solace
#endif  // SOLACE_PATH_HPP
