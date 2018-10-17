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
 * libSolace: Fixed size immutable string
 *	@file		solace/string.hpp
 *	@brief		Fixed size immutable string that owns its memory.
 ******************************************************************************/
#pragma once
#ifndef SOLACE_STRING_HPP
#define SOLACE_STRING_HPP


#include "solace/stringView.hpp"
#include "solace/memoryBuffer.hpp"
#include "solace/memoryManager.hpp"
#include "solace/arrayView.hpp"
#include "solace/byteWriter.hpp"


namespace Solace {

/** Immutable String object
 * Solace::String is a proper immutable unicode string that brings the comfort yet
 * it can be easily converted to and from std::string and/or C-strings
 */
class String {
public:

    /// String size_type is intentionally small to disallow long strings.
    using size_type = StringView::size_type;
    using value_type = StringView::value_type;

    using char_type = Char;

public:

    ~String() = default;

    //!< Default constructor constructs an empty string.
    constexpr String() noexcept = default;

    //!< Copy-construct is deleted
    String(String const&) = delete;

    //!< Move-construct a string.
    constexpr String(String&& rhs) noexcept
        : _buffer(std::move(rhs._buffer))
        , _size(exchange(rhs._size, 0))
    {
    }

    String(MemoryBuffer&& buffer, size_type bufferLen)
        : _buffer(std::move(buffer))
        , _size(bufferLen)
    {}

public:  // Additional to base object operations

    String& operator= (String const&) = delete;

    String& swap(String& rhs) noexcept;

    String& operator= (String&& rhs) noexcept {
		return swap(rhs);
	}

public:  // Basic collection operations:

    /** Test if this collection is empty
     * @returns True, if string is empty
     */
    constexpr bool empty() const noexcept {
        return _size == 0;
    }

	/** Length of this string.
	 * The length is the number of Unicode code units in the string.
	 *
	 * @return Number of code points this string contains.
	 */
	constexpr size_type length() const noexcept {
		return _size;
	}

	/** Size of string buffer in bytes.
	 *
	 * @return Size of buffer in bytes.
	 * @note Always (size() >= length())
	 */
	constexpr size_type size() const noexcept {
		return _size;
	}

    //!< True if values are equal
    bool equals(String const& v) const noexcept {
        return equals(v.view());
    }

    //!< True if values are equal
    bool equals(StringView v) const noexcept;

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
	int compareTo(String const& other) const noexcept {
		return compareTo(other.view());
	}

	int compareTo(StringView other) const noexcept;

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
	bool contains(StringView str) const
	{	return indexOf(str).isSome(); }

    bool contains(String const& str) const
    {	return contains(str.view()); }

    bool contains(value_type str) const
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
	Optional<size_type> indexOf(StringView str, size_type fromIndex = 0) const;

	Optional<size_type> indexOf(String const& str, size_type fromIndex = 0) const {
		return indexOf(str.view(), fromIndex);
	}

    Optional<size_type> indexOf(value_type ch, size_type fromIndex = 0) const;

    Optional<size_type> lastIndexOf(StringView str, size_type fromIndex = 0) const;

    Optional<size_type> lastIndexOf(String const& str, size_type fromIndex = 0) const {
        return lastIndexOf(str.view(), fromIndex);
    }

    Optional<size_type> lastIndexOf(value_type ch, size_type fromIndex = 0) const;

	/** Splits this string around matches of expr
	 *
	 */
    template<typename Callable>
    void split(StringView delim, Callable&& f) const {
        view().split(delim, std::forward<Callable>(f));
    }

    template<typename Callable>
    void split(value_type delim, Callable&& f) const {
        view().split(delim, std::forward<Callable>(f));
    }

	/**
	 * Returns a new string that is a substring of this string
	 * starting from 'from' of the given length.
	 *
	 *  @param from [in] Index of first character of the substring.
	 *  @param len [in] Length of the substring.
     *  @return Substring of this string starting from the given index.
	 */
	StringView substring(size_type from, size_type len) const {
		return view().substring(from, len);
	}

    /**
     * Returns a new string that is a substring of this string
     * starting from 'from' of the given length.
     *
     *  @param from [in] Index of first character of the substring.
     *  @return Substring of this string starting from the given index.
     */
    StringView substring(size_type from) const {
        return view().substring(from);
    }

	/**
	 * Returns a copy of the string,
	 * with leading and trailing whitespace omitted.
	 */
	StringView trim() const noexcept;


    /**
     * Tests if this string starts with the given prefix.
     * @param prefix The prefix to check.
     * @return True if this string indeed starts with the given prefix, false otherwise.
     */
    bool startsWith(StringView prefix) const;

    /**
     * Tests if this string starts with the given prefix.
     * @param prefix The prefix to check.
     * @return True if this string indeed starts with the given prefix, false otherwise.
	 */
	bool startsWith(String const& prefix) const {
		return startsWith(prefix.view());
	}

	/**
	 * Tests if this string starts with the specified prefix.
     * @param prefix The prefix to check.
     * @return True if this string indeed starts with the given prefix, false otherwise.
     */
    bool startsWith(value_type prefix) const;

	/**
	 * Tests if this string ends with the specified suffix.
     * @param suffix The suffix to check.
     * @return True if this string indeed ends with the given suffix, false otherwise.
     */
    bool endsWith(StringView suffix) const;

	/**
	 * Tests if this string ends with the specified suffix.
	 * @param suffix The suffix to check.
	 * @return True if this string indeed ends with the given suffix, false otherwise.
	 */
	bool endsWith(String const& suffix) const {
		return endsWith(suffix.view());
	}

	/**
	 * Tests if this string ends with the specified suffix.
     * @param suffix The suffix to check.
     * @return True if this string indeed ends with the given suffix, false otherwise.
     */
    bool endsWith(value_type suffix) const;

	/** Returns a hash code for this string.
	 * The hash code for a String object is computed as
	 * s[0]*31^(n-1) + s[1]*31^(n-2) + ... + s[n-1]
	 * using int arithmetic, where s[i] is the ith character of the string,
	 * n is the length of the string, and ^ indicates exponentiation.
	 * (The hash value of the empty string is zero.)
	 *
	 * @return A hash code value for this object.
	 */
    uint64 hashCode() const;

	/**
	 * Identity operation
	 */
	String const& toString() const { return *this; }

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
	StringView operator() (size_type from, size_type to) const
	{	return substring(from, to); }


    /**
     * Get raw bytes of the string.
     * @return Immutable Memory View into the string data.
     */
    /*constexpr*/ StringView view() const noexcept {
        return {_buffer.view().dataAs<char>(), size()};
    }

public:

	/** The <b>empty</b> string. */
    static String const  Empty;

    /**
     * Return String representation of value.
     * @param val Value to convert into the string
     *
     * @return The string representation of the given value
     **/
//    static String valueOf(bool val);
//    static String valueOf(String const& val);
//    static String valueOf(StringView val);
//    static String valueOf(int16 val);
//    static String valueOf(int32 val);
//    static String valueOf(int64 val);
//    static String valueOf(uint16 val);
//    static String valueOf(uint32 val);
//    static String valueOf(uint64 val);
//    static String valueOf(float32 val);
//    static String valueOf(float64 val);


    /** @see Iterable::forEach */
    template<typename F>
    std::enable_if_t<isCallable<F, const value_type&>::value || isCallable<F, value_type>::value,
    const String& >
    forEach(F&& f) const {
        auto v = view();
        for (const auto& x : v) {
            f(x);
        }

        return *this;
    }

private:
    MemoryBuffer                _buffer;
    size_type                   _size{0};
};


inline
bool operator< (String const& lhs, String const& rhs) {
	return lhs.compareTo(rhs) < 0;
}


inline
bool operator== (String const& lhv, String const& rhv) {
    return lhv.equals(rhv);
}

inline
bool operator== (String const& lhv, StringView rhv) {
    return lhv.equals(rhv);
}

inline
bool operator== (StringView lhv, String const& rhv) {
    return rhv.equals(lhv);
}

inline
bool operator== (String const& lhv, const char* rhv) {
    return lhv.equals(rhv);
}

inline
bool operator== (const char* lhv, String const& rhv) {
    return rhv.equals(lhv);
}


inline
bool operator!= (String const& lhv, String const& rhv) {
    return !lhv.equals(rhv);
}

inline
bool operator!= (String const& lhv, StringView rhv) {
    return !lhv.equals(rhv);
}

inline
bool operator!= (StringView lhv, String const& rhv) {
    return !rhv.equals(lhv);
}

inline
bool operator!= (String const& lhv, const char* rhv) {
    return !lhv.equals(rhv);
}

inline
bool operator!= (const char* lhv, String const& rhv) {
    return !rhv.equals(lhv);
}



inline void swap(String& lhs, String& rhs) noexcept {
    lhs.swap(rhs);
}



//!< Construct a string from a StringView
String makeString(StringView view);

//!< Construct a string from a raw null-terminated (C-style) string.
inline String makeString(const char* data) {
    return makeString(StringView(data));
}

//!< Construct a string from a raw byte buffer of a given size
inline String makeString(const char* data, String::size_type dataLength)  {
    return makeString(StringView(data, dataLength));
}

//!< Construct the string from std::string - STD compatibility method
// TODO(one day): String makeString(std::string const& buffer);
// TODO(one day): String makeString(std::string&& buffer);

//!< Copy string content from another string.
inline String makeString(String const& s) {
    return makeString(s.view());
}

template <>
inline void ctor(String& location, String const& s) {
  new (_::PlacementNew(), &location) String(makeString(s));
}



/**
 * Concatenates the specified string to the end of this string.
 * @param str A string to append to this string.
 * @return A string that is a result of concatenation of this and a given string.
 */
//String makeString(StringView lhs, StringView rhs);



inline
StringView::size_type totalSize(StringView arg) {
    return arg.size();
}

inline
StringView::size_type totalSize(String const& arg) {
    return arg.size();
}

template<typename...Args>
StringView::size_type totalSize(StringView by, Args&&... args) {
    return totalSize(by) + totalSize(std::forward<Args>(args)...);
}

template<typename...Args>
StringView::size_type totalSize(String const& by, Args&&... args) {
    return totalSize(by) + totalSize(std::forward<Args>(args)...);
}


inline
bool writeArg(ByteWriter& dest, StringView arg) {
    dest.write(arg.view());

    return true;
}

inline
bool writeArg(ByteWriter& dest, String const& arg) {
    dest.write(arg.view().view());

    return true;
}

inline
bool writeAllArgs(ByteWriter&) { return true; }

template<typename T, typename...Args>
bool writeAllArgs(ByteWriter& dest, T&& arg, Args&&...args) {
    writeArg(dest, arg);
    // FIXME: Check return value!

    return writeAllArgs(dest, std::forward<Args>(args)...);
}

template<typename... StringViews>
String makeString(StringView lhs, StringView rhs, StringViews&&... args) {
    auto const totalStrLen = totalSize(lhs, rhs, std::forward<StringViews>(args)...);
    auto buffer = getSystemHeapMemoryManager().create(totalStrLen * sizeof(StringView::value_type));    // May throw
    auto writer = ByteWriter(buffer.view());

    // Copy string view content into a new buffer
    auto r = writeAllArgs(writer, lhs, rhs, std::forward<StringViews>(args)...);

    return { std::move(buffer), totalStrLen };
}

template<typename... StringViews>
String makeString(String const& lhs, StringView rhs, StringViews&&... args) {
    return makeString(lhs.view(), rhs, std::forward<StringViews>(args)...);
}
template<typename... StringViews>
String makeString(StringView lhs, String const& rhs, StringViews&&... args) {
    return makeString(lhs, rhs.view(), std::forward<StringViews>(args)...);
}

template<typename... StringViews>
String makeString(String const& lhs, String const& rhs, StringViews&&... args) {
    return makeString(lhs.view(), rhs.view(), std::forward<StringViews>(args)...);
}

/*
inline String makeString(StringView lhs, String const& rhs) {
    return makeString(lhs, rhs.view());
}

inline String makeString(String const& lhs, StringView rhs) {
    return makeString(lhs.view(), rhs);
}

inline String makeString(String const& lhs, const char* str) {
    return makeString(lhs, StringView(str));
}

inline String makeString(String const& lhs, String const& rhs) {
    return makeString(lhs.view(), rhs.view());
}
*/

/**
 * Returns a new string with all occurrences of 'what' replaced with 'with'.
 * @param what A character to be replaced in the original string.
 * @param with A replacement character that will replace all occurrences of the given one in the source string.
 * @return A new string with all occurrences of 'what' replaced with 'with'.
 */
String makeStringReplace(StringView str, String::value_type what, String::value_type with);

/**
 * Returns a new string with all occurrences of substring 'what' replaced with given one.
 * @param what A sub-string to be replaced in the original string.
 * @param with A replacement string that will replace all occurrences of the given one in the source string.
 * @return A new string with all occurrences of 'what' replaced with 'with'.
 */
String makeStringReplace(StringView str, StringView what, StringView with);

inline String makeStringReplace(String const& str, String::value_type what, String::value_type with) {
    return makeStringReplace(str.view(), what, with);
}
inline String makeStringReplace(String const& str, StringView what, StringView with) {
    return makeStringReplace(str.view(), what, with);
}

inline String makeStringReplace(String const& str, String const& what, String const& with) {
    return makeStringReplace(str.view(), what.view(), with.view());
}

inline String makeStringReplace(String const& str, String const& what, StringView with) {
    return makeStringReplace(str.view(), what.view(), with);
}

inline String makeStringReplace(String const& str, StringView what, String const& with) {
    return makeStringReplace(str.view(), what, with.view());
}


inline
String makeStringJoin(StringView SOLACE_UNUSED(by)) {
    return makeString(String::Empty);
}

inline
String makeStringJoin(StringView SOLACE_UNUSED(by), StringView str) {
    return makeString(str);
}

inline
String makeStringJoin(StringView SOLACE_UNUSED(by), String const& str) {
    return makeString(str);
}


inline
bool writeJointArgs(ByteWriter& dest, StringView SOLACE_UNUSED(by), StringView arg) {
    dest.write(arg.view());

    return true;
}

inline
bool writeJointArgs(ByteWriter& dest, StringView SOLACE_UNUSED(by), String const& arg) {
    dest.write(arg.view().view());

    return true;
}


template<typename...Args>
bool writeJointArgs(ByteWriter& dest, StringView by, StringView arg, Args&&...args) {
    dest.write(arg.view());
    dest.write(by.view());

    return writeJointArgs(dest, by, std::forward<Args>(args)...);
}

template<typename...Args>
bool writeJointArgs(ByteWriter& dest, StringView by, String const& arg, Args&&...args) {
    dest.write(arg.view().view());
    dest.write(by.view());

    return writeJointArgs(dest, by, std::forward<Args>(args)...);
}


template<typename...Args>
String makeStringJoin(StringView by, Args&&... args) {
    auto const len = totalSize(std::forward<Args>(args)...);
    auto const totalStrLen = narrow_cast<StringView::size_type>(by.size() * (sizeof...(args) - 1) + len);
    auto buffer = getSystemHeapMemoryManager().create(totalStrLen * sizeof(StringView::value_type));    // May throw
    auto writer = ByteWriter(buffer.view());

    // Copy string view content into a new buffer
    writeJointArgs(writer, by, std::forward<Args>(args)...);

    return {std::move(buffer), totalStrLen};
}

template<typename...Args>
String makeStringJoin(String const& by, Args&&... args) {
    return makeStringJoin(by.view(), std::forward<Args>(args)...);
}
/*
inline String makeStringJoin(StringView by, String const& lhs, StringView rhs) {
    return makeStringJoin(by, lhs.view(), rhs);
}

inline String makeStringJoin(StringView by, StringView lhs, String const& rhs) {
    return makeStringJoin(by, lhs, rhs.view());
}

inline String makeStringJoin(StringView by, String const& lhs, String const& rhs) {
    return makeStringJoin(by, lhs.view(), rhs.view());
}
*/

/**
 * Return jointed string from this given initializer_list
 * @param by - A string joining values
 * @param list - An array of strings to join with the give separator
 *
 * @return The resulting string
 */
String makeStringJoin(StringView by, ArrayView<const String> list);



}  // namespace Solace
#endif  // SOLACE_STRING_HPP
