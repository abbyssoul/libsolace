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
 * libSolace: String primitive
 *	@file		solace/string.hpp
 *	@author		$LastChangedBy: $
 *	@date		$LastChangedDate: $
 *	@brief		Immutable Unicode String
 *	ID:			$Id: $
 ******************************************************************************/
#pragma once
#ifndef SOLACE_STRING_HPP
#define SOLACE_STRING_HPP


#include "solace/char.hpp"
#include "solace/iformattable.hpp"
#include "solace/byteBuffer.hpp"
#include "solace/array.hpp"


#include <string>   // std::string
#include <ostream>


namespace Solace {

/** Immutable Unicode String
 * Solace::String is a proper immutable unicode string that brings the comfort yet
 * it can be easily converted to and from std::string and/or C-strings
 */
class String:   public IFormattable,
                public IComparable<String>
        // TODO(abbyssoul): public Iterable<Char>
{
public:
    typedef uint32  size_type;
    typedef Char    value_type;

public:

	//!< Default constructor constructs an null(empty) string
	String() noexcept;

	//!< Copy string from an other one
	String(const String& s) noexcept;

	//!< Move-construct an object
	String(String&& s) noexcept;

    //!< Construct a string from a raw null-terminated (C-style) string
	String(const char* data);

    //!< Construct a string from a raw byte buffer of a given size
    String(const char* data, size_type length);

    //!< Construct a string from a nicely managed buffer
    String(const Buffer& buffer);

    //!< Construct a string from a byte buffer
    String(ByteBuffer& buffer);

    //!< Construct the string from std::string - STD compatibility method
	String(const std::string& buffer) : _str(buffer) {}

	virtual ~String() = default;

public:  // Additional to base object operations

    String& swap(String& rhs) noexcept;

    String& operator= (String&& rhs) noexcept {
		return swap(rhs);
	}

    String& operator= (const String& rhs) noexcept {
        String(rhs).swap(*this);

        return *this;
    }

public:  // Basic collection operations:

    /** Test if this collection is empty
     * @returns True, if string is empty
     */
    bool empty() const noexcept;

	/** Length of this string.
	 * The length is the number of Unicode code units in the string.
	 *
	 * @return Number of code points this string contains.
	 */
	size_type length() const noexcept;

	/** Size of string buffer in bytes.
	 *
	 * @return Size of buffer in bytes.
	 * @note Always (size() >= length())
	 *
	 */
	size_type size() const noexcept;

    //!< True if values are equal
    bool equals(const String& v) const noexcept override;

    using IComparable::operator!=;
    using IComparable::operator==;

    //!< True if values are equal
    bool equals(const char* v) const;

    bool operator== (const char* rhv) const {
        return equals(rhv);
    }

    bool operator!= (const char* rhv) const {
        return !equals(rhv);
    }

    /** Compares two strings lexicographically.
	 * The comparison is based on the Unicode value
	 * of each character in the strings.
	 * The result is zero if the strings are equal:
	 * exactly when the equals(other) would return true.
	 *
	 * @return The value 0 if the argument string is equal to this string;
	 * @return a value less than 0 if this string is lexicographically less than the string argument;
	 * @return and a value greater than 0 if this string is lexicographically greater than the string argument.
	 */
	int compareTo(const String& other) const;

	int compareTo(const char* other) const;

	/**
	 * Returns the char value at the specified index.
	 * An index ranges from 0 to length() - 1.
	 *
	 * @param index The index.
	 * @param @return The character at the specified index, as a Char.
	 * @throw std::out_of_range If <i>index</I> is out of range.
	 */
	value_type charAt(size_type index) const;

	/** Determine if the string contains a given substring.
	 *
	 * @param str The substring to search for.
	 * @return <b>true</b> if the string contains at least one occurrence of
	 * the substring, <b>false</b> otherwise.
	 */
	bool contains(const String& str) const
    {	return indexOf(str).isSome(); }

	bool contains(const value_type& str) const
    {	return indexOf(str).isSome(); }

	bool contains(const char* str) const
    {	return indexOf(str).isSome(); }

	/** Index of the first occurrence of the given character.
	 *
	 * Returns the index within this string of the first occurrence of the
	 * give character.
	 * If a character given occurs in the String, then the index
	 * (in Unicode code units) of the first occurrence is returned, such:
	 * this.charAt(k) == ch
	 *
     * @return Optional index of the first occurrence of the given character.
	 */
    Optional<size_type> indexOf(const String& str, size_type fromIndex = 0) const;

    Optional<size_type> indexOf(const value_type& ch, size_type fromIndex = 0) const;

    Optional<size_type> indexOf(const char* str, size_type fromIndex = 0) const;

    Optional<size_type> lastIndexOf(const String& str, size_type fromIndex = 0) const;

    Optional<size_type> lastIndexOf(const value_type& ch, size_type fromIndex = 0) const;

    Optional<size_type> lastIndexOf(const char* str, size_type fromIndex = 0) const;

	/** Returns a string that is a result of concatenation of this and a given string
	 * Concatenates the specified string to the end of this string.
	 */
	String concat(const String& str) const;

	String concat(const char* str) const;

	/**
	 * Returns a new string with all occurrences of 'what'
	 * replaced with 'with'.
	 */
	String replace(const value_type& what, const value_type& with) const;

	/**
	 * Returns a new string with all occurrences of substring
	 * replaced with given one.
	 */
	String replace(const String& what, const String& by) const;

	/** Splits this string around matches of expr
	 *
	 */
	Array<String> split(const String& expr) const;

	/**
	 * Returns a new string that is a substring of this string
	 * starting from 'from' of the given length.
	 *
	 *  @param from [in] Index of first character of the substring.
	 *  @param len [in] Length of the substring.
     *  @return Substring of this string starting from the given index.
	 */
    String substring(size_type from, size_type len) const;

    /**
     * Returns a new string that is a substring of this string
     * starting from 'from' of the given length.
     *
     *  @param from [in] Index of first character of the substring.
     *  @return Substring of this string starting from the given index.
     */
    String substring(size_type from) const;

	/**
	 * Returns a copy of the string,
	 * with leading and trailing whitespace omitted.
	 */
	String trim() const;

	/**
	 * Returns a new string
	 * that has all characters of this string converted to lower case.
	 * Doesn't handle locale dependent special casing.
	 */
	String toLowerCase() const;

	/**
	 * Returns a new string
	 * that has all characters of this string converted to upper case.
	 * Doesn't handle locale dependent special casing.
	 */
	String toUpperCase() const;

	/**
	 * Tests if this string starts with the specified prefix.
	 * @params prefix - the prefix.
	 */
	bool startsWith(const String& prefix) const;

	/**
	 * Tests if this string starts with the specified prefix.
	 * @params prefix - the prefix.
	 */
	bool startsWith(const value_type& prefix) const;

	/**
	 * Tests if this string ends with the specified suffix.
	 * @params suffix - the suffix.
	 */
	bool endsWith(const String& suffix) const;

	/**
	 * Tests if this string ends with the specified suffix.
	 * @params suffix - the suffix.
	 */
	bool endsWith(const value_type& suffix) const;

	/** Returns a hash code for this string.
	 * The hash code for a String object is computed as
	 * s[0]*31^(n-1) + s[1]*31^(n-2) + ... + s[n-1]
	 * using int arithmetic, where s[i] is the ith character of the string,
	 * n is the length of the string, and ^ indicates exponentiation.
	 * (The hash value of the empty string is zero.)
	 *
	 * @return A hash code value for this object.
	 */
    int64 hashCode() const;

	/**
	 * Identity operation
	 */
	String toString() const override { return *this; }

	/**
     * Get bytes of the encoded representation of the string.
	 */
    Buffer getBytes() const;

    /** Array index operator. Obtain a copy of the character at the given
	 * offset in the string.
	 *
	 * @param index The index.
	 * @return The character at the specified index, as a Char.
	 * @throw OutOfBoundsException If <i>index</I> is out of range.
	 */
	value_type operator[] (size_type index) const {
		return charAt(index);
	}

	/** Substring operator. Extract a substring from the string.
	 *
     * @param from [in] The start index of the substring.
     * @param to [in] The end index of the substring
	 * @return The substring as a new string. If fromIndex and toIndex are
	 * equal, an empty string is returned.
     *
     * @see String::substring
	 */
    String operator() (size_type from, size_type to) const
	{	return substring(from, to);}

	const char* c_str() const;

    // FIXME: Must be conditional to std support mode only
    std::string to_str() const {
        return _str;
    }

public:

	/** The <b>null</b> string. */
	static const String Null;

	/** The <b>empty</b> string. */
	static const String Empty;

	/**
	 * Return jointed string from this given initializer_list
	 */
	static String join(const String& by, std::initializer_list<String> list);

	/** Return jointed string from the given collection */
	static String join(const String& by, const Array<String>& list);

protected:
public:

    /* FIXME(abbyssoul): should be implemented after migration from std::string
    Iterator findFirst(const value_type& ch) const {
        return findFirst(ch, begin());
    }
    Iterator findFirst(const String& ch) const {
        return findFirst(ch, begin());
    }

    Iterator findFirst(const value_type& ch, Iterator from) const;
    Iterator findFirstNot(const value_type& ch, Iterator from) const;
    Iterator findLast(const value_type& ch, Iterator from) const;
    Iterator findLastNot(const value_type& ch, Iterator from) const;

    Iterator findFirst(const String& ch, Iterator from) const;
    Iterator findFirstNot(const String& ch, Iterator from) const;
    Iterator findLast(const String& ch, Iterator from) const;
    Iterator findLastNot(const String& ch, Iterator from) const;

    String substring(Iterator from) const;
    String substring(Iterator from, Iterator to) const;
*/
private:

    // FIXME(abbyssoul): Come up a better implementation
    std::string _str;
};

inline bool operator< (const String& lhs, const String& rhs) {
	return lhs.compareTo(rhs) < 0;
}


// FIXME: std dependence, used for Unit Testing only
inline std::ostream& operator<<(std::ostream& ostr, const String& str) {
    return ostr << str.c_str();
}

inline std::ostream& operator<<(std::ostream& ostr, const IFormattable& form) {
    return ostr << form.toString();
}

}  // namespace Solace
#endif  // SOLACE_STRING_HPP
