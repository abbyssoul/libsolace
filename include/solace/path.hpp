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
 *	@brief		Immutable hierarchical Path class
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

	struct Iterator {

		Iterator(Path const& p, size_type position) noexcept
			: _index{position}
			, _path{p}
		{}

		constexpr Iterator(Iterator const& rhs) noexcept = default;
		constexpr Iterator(Iterator&& rhs) noexcept = default;

		Iterator& operator= (Iterator const& rhs) noexcept = default;
		Iterator& operator= (Iterator&& rhs) noexcept {
			return swap(rhs);
		}

		constexpr bool operator!= (Iterator const& other) const noexcept {
			return (_index != other._index);
		}

		constexpr bool operator== (Iterator const& other) const noexcept {
			return (_index == other._index);
		}

		Iterator& operator++ ();

		StringView operator-> () const;

		auto operator* () const {
			return operator ->();
		}


		Iterator& swap(Iterator& rhs) noexcept {
			std::swap(_index, rhs._index);
			std::swap(_path, rhs._path);

			return *this;
		}

		constexpr size_type getIndex() const noexcept { return _index; }

	private:
		size_type	_index;
		std::reference_wrapper<const Path> _path;
	};

	using const_iterator = Iterator;

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
	constexpr Path() noexcept = default;

    /** Construct an object by moving content from a given */
	constexpr Path(Path&& p) noexcept
		: _components{mv(p._components)}
    { }

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
    constexpr bool empty() const noexcept {
        return _components.empty();
    }

    /** Test if path is not empty.
     * @return True if this path is not an empty object.
     */
    explicit constexpr operator bool() const {
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
	bool endsWith(Path const& other) const;

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

	/** Get n'th component of this path
	 * @param index Index of a path segment
     * @return Number of path elements in this path
     */
	StringView getComponent(size_type index) const;

    const_iterator begin() const {
		return Iterator{*this, 0};
    }

    const_iterator end() const {
		return Iterator{*this, getComponentsCount()};
	}

    /** Returns sub path of this path
     * Slice of this path object
     * @return Sub path of this path
     */
    Path subpath(size_type beginIndex, size_type endIndex) const noexcept;


    /** @see Iterable::forEach */
    template<typename F>
	std::enable_if_t<isCallable<F, StringView>::value, Path const &>
    forEach(F&& f) const {
        for (auto const& i : _components) {
			f(i.view());
        }

        return *this;
    }

	template<typename F>
	std::enable_if_t<isCallable<F, Path::value_type&&>::value, Path &>
	forEach(F&& f) && {
		for (auto& i : _components) {
			f(mv(i));
		}

		return *this;
	}

    /** Get string representation of the path object using give delimiter */
    String toString(StringView delim) const;

    /**
     * Return string representation of this path
     * @return String representation of this path
     */
    String toString() const {
        return toString(Delimiter);
    }

protected:

	friend Path makePath(Array<String>&& array) noexcept;

    /** FIXME(abbyssoul): Only temporary here. to be removed
     * Move-Construct the path object from a collection of String components
     * @param array A collection of string components forming the path
     */
	constexpr Path(Array<String>&& array) noexcept
		: _components{mv(array)}
	{
        // No-op
    }


private:

	Array<String>  _components;
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


[[nodiscard]] inline
Path makePath(Array<String>&& array) noexcept {
	return {mv(array)};
}

[[nodiscard]] inline
Path makePath(Vector<String>&& vec) noexcept {
	return makePath(vec.toArray());
}


/**
 * Construct the path object from a single string component
 *
 * @note The string is is parsed into component, please use Path::parse
 */
[[nodiscard]]
Result<Path, Error> makePath(StringView str);

[[nodiscard]] inline
auto makePath(String const& str) {
    return makePath(str.view());
}

[[nodiscard]] inline
auto makePath(char const* str) {
    return makePath(StringView{str});
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
constexpr Path::size_type countPathComponents(Path&& path)      noexcept { return path.getComponentsCount(); }


template<typename T, typename...Args>
Path::size_type countPathComponents(T&& base, Args&&...args) {
	return (countPathComponents(fwd<T>(base)) + ... + countPathComponents(fwd<Args>(args)));
}


inline Result<void, Error> joinComponents(Vector<String>& base, StringView view) {
	auto r = makeString(view);
	if (!r)
		return r.moveError();

	base.emplace_back(r.moveResult());
	return Ok();
}

inline Result<void, Error> joinComponents(Vector<String>& base, String&& str) {
	base.emplace_back(mv(str));
	return Ok();
}

inline Result<void, Error> joinComponents(Vector<String>& base, String const& str) {
	auto maybeDup = makeString(str);
	if (!maybeDup)
		return maybeDup.moveError();

	base.emplace_back(maybeDup.moveResult());
	return Ok();
}

inline Result<void, Error> joinComponents(Vector<String>& base, Path&& path) {
	mv(path).forEach([&base](Path::value_type&& component) {
		base.emplace_back(mv(component));
    });

	return Ok();
}

inline Result<void, Error> joinComponents(Vector<String>& base, Path const& path) {
	for (auto component : path) {
		auto maybeDup = makeString(component);
		if (!maybeDup)
			return maybeDup.moveError();

		base.emplace_back(maybeDup.moveResult());
    }

	return Ok();
}


template <typename...Args>
Result<void, Error> joinComponents(Vector<String>& base, StringView view, Args&&...args) {
	auto r = joinComponents(base, view);
	if (!r)
		return r;

	return joinComponents(base, fwd<Args>(args)...);
}

template <typename...Args>
Result<void, Error> joinComponents(Vector<String>& base, String const& view, Args&&...args) {
	auto r = joinComponents(base, view);
	if (!r)
		return r;

	return joinComponents(base, fwd<Args>(args)...);
}


template <typename...Args>
Result<void, Error> joinComponents(Vector<String>& base, Path const& path, Args&&...args) {
	auto r = joinComponents(base, path);
	if (!r)
		return r;

	return joinComponents(base, fwd<Args>(args)...);
}


template <typename...Args>
Result<void, Error> joinComponents(Vector<String>& base, Path&& path, Args&&...args) {
	auto r = joinComponents(base, mv(path));
	if (!r)
		return r;

	return joinComponents(base, fwd<Args>(args)...);
}


}  // namespace details


template<typename...Args>
[[nodiscard]]
Result<Path, Error> makePath(Args&&...args) {
	auto maybeVector = makeVector<Path::value_type>(details::countPathComponents(fwd<Args>(args)...));
	if (!maybeVector) {
		return maybeVector.moveError();
	}

	auto maybeComponents = details::joinComponents(maybeVector.unwrap(), fwd<Args>(args)...);
	if (!maybeComponents) {
		return maybeComponents.moveError();
	}

	return Ok(makePath(maybeVector.moveResult()));
}

}  // namespace Solace
#endif  // SOLACE_PATH_HPP
