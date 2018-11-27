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
#include "solace/memoryResource.hpp"
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


    /**
     * Construct a string giving it memory buffer to manage with it's content being interpreted as a string.
     * @param buffer A buffer to take ownership of.
     * @param stringLen Lenght of the string in writtein into the buffer.
     * @note The buffer passed must be big enought to hold the string of the given size.
     */
    constexpr String(MemoryResource&& buffer, size_type stringLen) noexcept
        : _buffer(std::move(buffer))
        , _size(stringLen)
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
	bool contains(StringView str) const noexcept
	{	return indexOf(str).isSome(); }

    bool contains(String const& str) const noexcept
    {	return contains(str.view()); }

    bool contains(value_type str) const noexcept
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
	Optional<size_type> indexOf(StringView str, size_type fromIndex = 0) const noexcept;

	Optional<size_type> indexOf(String const& str, size_type fromIndex = 0) const noexcept {
		return indexOf(str.view(), fromIndex);
	}

	Optional<size_type> indexOf(value_type ch, size_type fromIndex = 0) const noexcept;

	Optional<size_type> lastIndexOf(StringView str, size_type fromIndex = 0) const noexcept;

    Optional<size_type> lastIndexOf(String const& str, size_type fromIndex = 0) const noexcept {
        return lastIndexOf(str.view(), fromIndex);
    }

    Optional<size_type> lastIndexOf(value_type ch, size_type fromIndex = 0) const noexcept;


    /**
     * Splits this string around matches of the given delimeter.
     * @param delim A delimeter string to split this string around.
     * @param f A callable object to be called for each split substring.
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
	StringView substring(size_type from, size_type len) const noexcept {
		return view().substring(from, len);
	}

    /**
     * Returns a new string that is a substring of this string
     * starting from 'from' of the given length.
     *
     *  @param from [in] Index of first character of the substring.
     *  @return Substring of this string starting from the given index.
     */
    StringView substring(size_type from) const noexcept {
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
    bool startsWith(StringView prefix) const noexcept;

    /**
     * Tests if this string starts with the given prefix.
     * @param prefix The prefix to check.
     * @return True if this string indeed starts with the given prefix, false otherwise.
	 */
	bool startsWith(String const& prefix) const noexcept {
		return startsWith(prefix.view());
	}

	/**
	 * Tests if this string starts with the specified prefix.
     * @param prefix The prefix to check.
     * @return True if this string indeed starts with the given prefix, false otherwise.
     */
    bool startsWith(value_type prefix) const noexcept;

	/**
	 * Tests if this string ends with the specified suffix.
     * @param suffix The suffix to check.
     * @return True if this string indeed ends with the given suffix, false otherwise.
     */
    bool endsWith(StringView suffix) const noexcept;

	/**
	 * Tests if this string ends with the specified suffix.
	 * @param suffix The suffix to check.
	 * @return True if this string indeed ends with the given suffix, false otherwise.
	 */
	bool endsWith(String const& suffix) const noexcept {
		return endsWith(suffix.view());
	}

	/**
	 * Tests if this string ends with the specified suffix.
     * @param suffix The suffix to check.
     * @return True if this string indeed ends with the given suffix, false otherwise.
     */
    bool endsWith(value_type suffix) const noexcept;

	/** Returns a hash code for this string.
	 * The hash code for a String object is computed as
	 * s[0]*31^(n-1) + s[1]*31^(n-2) + ... + s[n-1]
	 * using int arithmetic, where s[i] is the ith character of the string,
	 * n is the length of the string, and ^ indicates exponentiation.
	 * (The hash value of the empty string is zero.)
	 *
	 * @return A hash code value for this object.
	 */
	uint64 hashCode() const noexcept;

	/**
	 * Identity operation
	 */
	String const& toString() const noexcept { return *this; }

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
	StringView operator() (size_type from, size_type to) const noexcept
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
    MemoryResource                _buffer;
    size_type                   _size{0};
};


inline
bool operator< (String const& lhs, String const& rhs) noexcept {
	return lhs.compareTo(rhs) < 0;
}


inline
bool operator== (String const& lhv, String const& rhv) noexcept {
    return lhv.equals(rhv);
}

inline
bool operator== (String const& lhv, StringView rhv) noexcept {
    return lhv.equals(rhv);
}

inline
bool operator== (StringView lhv, String const& rhv) noexcept {
    return rhv.equals(lhv);
}

inline
bool operator== (String const& lhv, const char* rhv) noexcept {
    return lhv.equals(rhv);
}

inline
bool operator== (const char* lhv, String const& rhv) noexcept {
    return rhv.equals(lhv);
}


inline
bool operator!= (String const& lhv, String const& rhv) noexcept {
    return !lhv.equals(rhv);
}

inline
bool operator!= (String const& lhv, StringView rhv) noexcept {
    return !lhv.equals(rhv);
}

inline
bool operator!= (StringView lhv, String const& rhv) noexcept {
    return !rhv.equals(lhv);
}

inline
bool operator!= (String const& lhv, const char* rhv) noexcept {
    return !lhv.equals(rhv);
}

inline
bool operator!= (const char* lhv, String const& rhv) noexcept {
    return !rhv.equals(lhv);
}


inline void swap(String& lhs, String& rhs) noexcept {
    lhs.swap(rhs);
}



/**
 * Construct a new string from a StringView
 * @param view A string view to copy data from.
 * @return A new string object that owns the memory where the data is kept.
 */
[[nodiscard]] String makeString(StringView view);


/**
 * Construct a new string from a StringLiteral. Resulting string does not own memory buffer.
 * @param view A string literal that represents the string.
 * @return A new string object that doesn not owns the memory where the data is kept.
 */
[[nodiscard]]
String makeString(StringLiteral literal) noexcept;


//!< Construct a string from a raw null-terminated (C-style) string.
[[nodiscard]] inline String makeString(const char* data) {
    return makeString(StringView(data));
}

//!< Construct a string from a raw byte buffer of a given size
[[nodiscard]] inline String makeString(const char* data, String::size_type dataLength)  {
    return makeString(StringView(data, dataLength));
}

//!< Construct the string from std::string - STD compatibility method
// TODO(one day): String makeString(std::string const& buffer);
// TODO(one day): String makeString(std::string&& buffer);

//!< Copy string content from another string.
[[nodiscard]] inline String makeString(String const& s) {
    return makeString(s.view());
}

template <>
inline String* ctor(String& location, String const& s) {
  return new (_::PlacementNew(), &location) String(makeString(s));
}

inline constexpr
StringView::size_type totalSize() noexcept {
    return 0;
}

inline constexpr
StringView::size_type totalSize(StringView::value_type SOLACE_UNUSED(arg)) noexcept {
    return 1;
}

inline constexpr
StringView::size_type totalSize(StringView arg) noexcept {
    return arg.size();
}

inline constexpr
StringView::size_type totalSize(StringLiteral arg) noexcept {
    return arg.size();
}

inline
StringView::size_type totalSize(const char* arg) noexcept {
    return totalSize(StringView{arg});
}

template<size_t N>
StringView::size_type totalSize(char const (&)[N]) noexcept {
    return N;
}


inline constexpr
StringView::size_type totalSize(String const& arg) noexcept {
    return arg.size();
}

template<typename T, typename K, typename...Args>
StringView::size_type totalSize(T&& a, K&& b, Args&&... args) noexcept {
    return totalSize(a) + totalSize(std::forward<K>(b), std::forward<Args>(args)...);
}

inline
auto writeArg(ByteWriter& dest, StringView::value_type arg) noexcept {
    return dest.write(arg);
}

inline
auto writeArg(ByteWriter& dest, StringView arg) noexcept {
    return dest.write(arg.view());
}

inline
auto writeArg(ByteWriter& dest, String const& arg) {
    return dest.write(arg.view().view());
}

inline
bool writeAllArgs(ByteWriter&) { return true; }

template<typename T, typename...Args>
bool writeAllArgs(ByteWriter& dest, T&& arg, Args&&...args) {
    writeArg(dest, arg);
    // FIXME: Check return value!

    return writeAllArgs(dest, std::forward<Args>(args)...);
}

/**
 * Concatenates the specified string to the end of this string.
 * @param str A string to append to this string.
 * @return A string that is a result of concatenation of this and a given string.
 */
template<typename... StringViews>
String makeString(StringView lhs, StringView rhs, StringViews&&... args) {
    auto const totalStrLen = totalSize(lhs, rhs, std::forward<StringViews>(args)...);
    auto buffer = getSystemHeapMemoryManager().allocate(totalStrLen * sizeof(StringView::value_type));    // May throw
    auto writer = ByteWriter(buffer.view());

    // Copy string view content into a new buffer
    // FIXME: WriteAllArgs returns Result<> thus makeString should return -> Result<String, Error>
    /*auto r = */writeAllArgs(writer, lhs, rhs, std::forward<StringViews>(args)...);

    return { std::move(buffer), totalStrLen };
}

template<typename... StringViews>
String makeString(StringView::value_type lhs, StringView rhs, StringViews&&... args) {
    auto const totalStrLen = totalSize(lhs, rhs, std::forward<StringViews>(args)...);
    auto buffer = getSystemHeapMemoryManager().allocate(totalStrLen * sizeof(StringView::value_type));    // May throw
    auto writer = ByteWriter(buffer.view());

    // Copy string view content into a new buffer
    // FIXME: WriteAllArgs returns Result<> thus makeString should return -> Result<String, Error>
    /*auto r = */writeAllArgs(writer, lhs, rhs, std::forward<StringViews>(args)...);

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

template<typename... StringViews>
String makeString(StringView::value_type lhs, String const& rhs, StringViews&&... args) {
    return makeString(lhs, rhs.view(), std::forward<StringViews>(args)...);
}

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
    return String{};
}

inline
String makeStringJoin(StringView::value_type SOLACE_UNUSED(by)) {
    return String{};
}

inline
String makeStringJoin(StringView SOLACE_UNUSED(by), StringView str) {
    return makeString(str);
}

inline
String makeStringJoin(StringView::value_type SOLACE_UNUSED(by), StringView str) {
    return makeString(str);
}

inline
String makeStringJoin(StringView SOLACE_UNUSED(by), String const& str) {
    return makeString(str);
}

inline
String makeStringJoin(StringView::value_type SOLACE_UNUSED(by), String const& str) {
    return makeString(str);
}


inline
auto writeJointArgs(ByteWriter& dest, StringView SOLACE_UNUSED(by), StringView arg) {
    return dest.write(arg.view());
}

inline
auto writeJointArgs(ByteWriter& dest, StringView SOLACE_UNUSED(by), String const& arg) {
    return dest.write(arg.view().view());
}

inline
auto writeJointArgs(ByteWriter& dest, StringView::value_type SOLACE_UNUSED(by), StringView arg) {
    return dest.write(arg.view());
}

inline
auto writeJointArgs(ByteWriter& dest, StringView::value_type SOLACE_UNUSED(by), String const& arg) {
    return dest.write(arg.view().view());
}


template<typename...Args>
Result<void, Error> writeJointArgs(ByteWriter& dest, StringView by, StringView arg, Args&&...args) {
    auto r = dest.write(arg.view())
            .then([&dest, &by]() {
                return dest.write(by.view());
            });

    return r
            ? writeJointArgs(dest, by, std::forward<Args>(args)...)
            : std::move(r);
}

template<typename...Args>
Result<void, Error> writeJointArgs(ByteWriter& dest, StringView by, String const& arg, Args&&...args) {
    auto r = dest.write(arg.view().view())
            .then([&dest, &by]() {
                dest.write(by.view());
            });

    return r
            ? writeJointArgs(dest, by, std::forward<Args>(args)...)
            : std::move(r);
}

template<typename...Args>
Result<void, Error> writeJointArgs(ByteWriter& dest, StringView::value_type by, StringView arg, Args&&...args) {
    auto r = dest.write(arg.view())
            .then([&dest, &by]() {
                return dest.write(by);
            });

    return r
            ? writeJointArgs(dest, by, std::forward<Args>(args)...)
            : std::move(r);
}

template<typename...Args>
Result<void, Error> writeJointArgs(ByteWriter& dest, StringView::value_type by, String const& arg, Args&&...args) {
    auto r = dest.write(arg.view().view())
            .then([&dest, &by]() {
                dest.write(by);
            });

    return r
            ? writeJointArgs(dest, by, std::forward<Args>(args)...)
            : std::move(r);
}



template<typename...Args>
String makeStringJoin(StringView by, Args&&... args) {
    auto const len = totalSize(std::forward<Args>(args)...);
    auto const totalStrLen = narrow_cast<StringView::size_type>(totalSize(by) * (sizeof...(args) - 1) + len);
    auto buffer = getSystemHeapMemoryManager().allocate(totalStrLen * sizeof(StringView::value_type));    // May throw
    auto writer = ByteWriter(buffer.view());

    // Copy string view content into a new buffer
    // FIXME: writeJointArgs returns Result<> thus makeString should return -> Result<String, Error>
    writeJointArgs(writer, by, std::forward<Args>(args)...);

    return {std::move(buffer), totalStrLen};
}

template<typename...Args>
String makeStringJoin(StringView::value_type by, Args&&... args) {
    auto const len = totalSize(std::forward<Args>(args)...);
    auto const totalStrLen = narrow_cast<StringView::size_type>(totalSize(by) * (sizeof...(args) - 1) + len);
    auto buffer = getSystemHeapMemoryManager().allocate(totalStrLen * sizeof(StringView::value_type));    // May throw
    auto writer = ByteWriter(buffer.view());

    // Copy string view content into a new buffer
    // FIXME: writeJointArgs returns Result<> thus makeString should return -> Result<String, Error>
    writeJointArgs(writer, by, std::forward<Args>(args)...);

    return {std::move(buffer), totalStrLen};
}


template<typename...Args>
String makeStringJoin(String const& by, Args&&... args) {
    return makeStringJoin(by.view(), std::forward<Args>(args)...);
}

/**
 * Return string result of joining an array of strings.
 * @param by - A string joining values
 * @param list - An array of strings to join with the give separator
 *
 * @return The resulting string
 */
String makeStringJoin(StringView by, ArrayView<const String> list);



}  // namespace Solace
#endif  // SOLACE_STRING_HPP
