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
 * libSolace: String view
 *	@file		solace/stringView.hpp
 *	@brief		Static Immutable Unicode String Literal
 ******************************************************************************/
#pragma once
#ifndef SOLACE_STRINGLITERAL_HPP
#define SOLACE_STRINGLITERAL_HPP

#include "solace/char.hpp"
#include "solace/optional.hpp"


namespace Solace {

/**
 * String View.
 *
 */
class StringView {
public:

    /// String size_type is intentionally small to discourage long strings.
    using size_type = uint16;

    using value_type = char;

    using const_iterator = value_type const*;

public:

    /** Construct an empty string view. Nothing to see here.
     *  By convention for empty view: data() is equal to nullptr and size() is equal to 0.
     */
    constexpr StringView() noexcept = default;

    //!< Copy-construct a string.
//    constexpr StringView(StringView const& other) noexcept = default;

    /**
     *  Constructs a view of the first count characters of the character array starting with the element pointed by s.
     *  s can contain null characters. The behavior is undefined if [s, s+count) is not a valid range.
     *  After construction, data() is equal to s, and size() is equal to count.
     *
     * @param s A pointer to a character array or a C string to initialize the view with.
     * @param count Number of characters to include in the view.
     */
    StringView(char const* s, size_type count) /*noexcept*/
        : _size{count}
        , _data{(s == nullptr)
                 ? (assertTrue(count == 0), s)
                 : assertNotNull(s)
                }
    {}


    // NOTE: This blows if a StringView is constructed over a fixed size c-array buffer
//    template<size_t N>
//    constexpr StringView(char const (&str)[N]) :
//            StringView(&str[0], N - 1)
//    {
//    }

    /**
     * Constructs a view of the null-terminated string pointed to by s, not including the terminating null character.
     * The length of the view is determined as if by strlen(s).
     * Note: the behavior is undefined if [s, s + strlen(s)) is not a valid range.
     * After construction, data() is equal to s, and size() is equal to Traits::length(s).
     * @param s A pointer to a character array or a C string to initialize the view with.
     */
    StringView(char const* s) noexcept;

	StringView(MemoryView data) noexcept
		: _size{narrow_cast<size_type>(data.size())}
		, _data{data.dataAs<char>()}
	{}

    StringView& swap(StringView& rhs) noexcept {
        using std::swap;

        swap(_size, rhs._size);
        swap(_data, rhs._data);

        return *this;
    }

    constexpr bool empty() const noexcept {
        return (_size == 0);
    }

    constexpr size_type length() const noexcept {
        return _size;
    }

    constexpr size_type size() const noexcept {
        return _size;
    }

    constexpr const char* data() const noexcept {
        return _data;
    }

    //!< True if values are equal
    bool equals(StringView x) const noexcept;

    int compareTo(StringView x) const noexcept;

    int compareTo(const char* x) const noexcept {
        return compareTo(StringView{x});
    }

    /**
     * Tests if the string starts with the specified prefix.
     *
     * @param prefix The prefix to check.
     * @return True if the string indeed starts with the given prefix, false otherwise.
     */
    constexpr bool startsWith(char prefix) const noexcept {
        return (_data == nullptr)
                ? (prefix == 0)
                : (_data[0] == prefix);
    }

    /**
     * Tests if the string starts with the specified prefix.
     *
     * @param prefix The prefix to check.
     * @return True if the string indeed starts with the given prefix, false otherwise.
     */
    bool startsWith(StringView prefix) const noexcept;

    /**
     * Tests if the string ends with the specified suffix.
     *
     * @param suffix The suffix to check.
     * @return True if this string indeed ends with the given suffix, false otherwise.
     */
    constexpr bool endsWith(char suffix) const noexcept {
        return (_data == nullptr)
                ? (suffix == 0)
                : (_data[size() - 1] == suffix);
    }

    /**
     * Tests if this string ends with the specified suffix.
     * @param suffix The suffix to check.
     * @return True if this string indeed ends with the given suffix, false otherwise.
     */
    bool endsWith(StringView suffix) const noexcept;

    /** Index of the first occurrence of the given sub sequance.
     *
     * Returns the index within this string of the first occurrence of the
     * give substring.
     *
     * @return Optional index of the first occurrence of the given substring.
     */
    Optional<size_type> indexOf(StringView str, size_type fromIndex = 0) const noexcept;

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
    Optional<size_type> indexOf(value_type ch, size_type fromIndex = 0) const noexcept;

    /** Index of the last occurrence of the given sub sequance.
     *
     * Returns the last index within this string of the of the give substring.
     *
     * @return Optional index of the last occurrence of the given substring.
     */
    Optional<size_type> lastIndexOf(StringView str, size_type fromIndex = 0) const noexcept;

    /** Index of the last occurrence of the given character.
     *
     * Returns the index within this string of the last occurrence of the give character.
     *
     * @return Optional index of the last occurrence of the given character.
     */
    Optional<size_type> lastIndexOf(value_type ch, size_type fromIndex = 0) const noexcept;

    /** Determine if the string contains a given substring.
     *
     * @param str The substring to search for.
     * @return <b>true</b> if the string contains at least one occurrence of
     * the substring, <b>false</b> otherwise.
     */
    bool contains(StringView str) const noexcept
    {	return indexOf(str).isSome(); }

    /** Determine if the string contains a given character.
     *
     * @param ch The character to search for.
     * @return <b>true</b> if the string contains at least one occurrence of
     * the character, <b>false</b> otherwise.
     */
    bool contains(value_type c) const noexcept
    {	return indexOf(c).isSome(); }

    /**
     * Returns a new string that is a substring of this string
     * starting from 'from' of the given length.
     *
     *  @param from [in] Index of first character of the substring.
     *  @param to [in] End index.
     *  @return Substring of this string starting from the given index.
     */
    StringView substring(size_type from, size_type to) const noexcept;

    /**
     * Returns a new string that is a substring of this string
     * starting from 'from' of the given length.
     *
     *  @param from [in] Index of first character of the substring.
     *  @return Substring of this string starting from the given index.
     */
    StringView substring(size_type from) const noexcept {
        return substring(from, size());
    }

    /**
     * Returns a sub-string with leading and trailing whitespace omitted.
     */
    StringView trim() const noexcept;

    /**
     * Returns a sub-string with leading and trailing occurance of the given character skipped.
     */
    StringView trim(value_type delim) const noexcept;

    /**
     * Get character at the index.
     * @param index Index of the character in the sequence.
     * @return Character at the index.
     * FIXME: Should be Optional<> and noexcept
     */
    value_type charAt(size_type index) const {
        index = assertIndexInRange(index, 0, _size);

        return _data[index];
    }

    /** Array index operator. Obtain a copy of the character at the given
	 * offset in the string.
	 *
	 * @param index The index.
	 * @return The character at the specified index, as a Char.
	 * @throw OutOfBoundsException If <i>index</I> is out of range.
	 * FIXME: Should be Optional<> and noexcept
	 */
    value_type operator[] (size_type index) const {
        return _data[index];
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
    StringView operator() (size_type from, size_type to) const noexcept {
        return substring(from, to);
    }

	constexpr explicit operator bool() const noexcept {
	  return !empty();
	}

    /** Splits the string around matches of expr
     * @param delim A delimeter to split the string by.
     * @return A total number of splits.
     */
    template<typename Callable>
    std::enable_if_t<isCallable<Callable, StringView>::value, size_type>
    split(StringView delim, Callable&& f) const {
        auto const delimLength = delim.size();
        auto const thisSize = size();

        if (thisSize < delimLength) {
            f(*this);

            return 1;
        }

        if (delimLength == 0) {
            for (size_type to = 0; to < thisSize; ++to) {
                f(substring(to, to + 1));
            }

            return thisSize;
        }

        size_type const toInc = (delimLength == 0) ? 0 : delimLength - 1;
        size_type from = 0, count = 1;

        for (size_type to = 0; to + delimLength <= thisSize; ++to) {
            if (delim.equals(substring(to, to + toInc + 1))) {
                count += 1;
                f(substring(from, to));

                to += toInc;
                from = to + 1;
            }
        }

        f(substring(from));

        return count;
    }

    template<typename Callable>
    std::enable_if_t<isCallable<Callable, StringView, StringView::size_type, StringView::size_type>::value, size_type>
    split(StringView delim, Callable&& f) const {
        auto const delimLength = delim.size();
        auto const thisSize = size();
        size_type delimCount = 0;

        for (size_type i = 0; i < thisSize && i + delimLength <= thisSize; ++i) {  // count_if, but with custom stride
            if (delim.equals(substring(i, i + delimLength))) {
                delimCount += 1;
                i += delimLength - 1;
            }
        }

        size_type to = 0, from = 0;
        for (size_type i = 0; to < thisSize && to + delimLength <= thisSize; ++to) {
            if (delim.equals(substring(to, to + delimLength))) {
                f(substring(from, to), i, delimCount + 1);
                i += 1;

                to += delimLength - 1;
                from = to + 1;
            }
        }

        f(substring(from), delimCount, delimCount + 1);

        return delimCount + 1;
    }

    /** Splits the string around matches of expr
     * @param delim A delimeter to split the string by.
     * @return A list of substrings.
     */
    template<typename Callable>
    size_type split(value_type delim, Callable&& f) const {
        auto const thisSize = size();
        size_type from = 0, count = 1;

        for (size_type to = 0; to < thisSize; ++to) {
            if (_data[to] == delim) {
                f(substring(from, to));
                count += 1;
                from = to + 1;
            }
        }

        f(substring(from));

        return count;
    }


    /** Returns a hash code for this string.
     *
     * @return A hash code value for the string.
     */
    uint64 hashCode() const noexcept;

    const_iterator begin() const noexcept {
        return empty()
                ? nullptr
                : _data;
    }

    const_iterator end() const noexcept { return begin() + size(); }

    MemoryView view() const noexcept {
        return wrapMemory(_data, _size);
    }

protected:

    constexpr StringView(size_type count, char const* s) noexcept
        : _size{count}
        , _data{s}
    {}

private:
    size_type   _size = 0;
	char const* _data = nullptr;
};


static_assert(sizeof(StringView) <= 2*sizeof(void*),
              "StringView must be no more then 2 pointers in size");

static_assert(std::is_trivially_destructible<StringView>::value,
              "StringView is not trivially copyable");
static_assert(std::is_trivially_copyable<StringView>::value,
              "StringView is not trivially copyable");
static_assert(std::is_trivially_copy_constructible<StringView>::value,
              "StringView is not trivially copyable");
static_assert(std::is_trivially_copy_assignable<StringView>::value,
              "StringView is not trivially copyable");
static_assert(std::is_trivially_move_constructible<StringView>::value,
              "StringView is not trivially copyable");
static_assert(std::is_trivially_move_assignable<StringView>::value,
              "StringView is not trivially copyable");


/** Immutable Unicode String Literal
 * StringLiteral is a wrapper of C-string defined in place in the code.
 * Naturally it is immutable.
 */
struct StringLiteral: public StringView {
    using StringView::size_type;

    using StringView::value_type;

    constexpr StringLiteral() noexcept = default;

    template<size_t N>
    constexpr StringLiteral(char const (&str)[N]) :
            StringView(N -1, &str[0])
    {
    }
};


inline void
swap(StringView& lhs, StringView& rhs) noexcept {
    lhs.swap(rhs);
}

inline void
swap(StringLiteral& lhs, StringLiteral& rhs) noexcept {
    lhs.swap(rhs);
}


inline
bool operator== (StringView lhv, StringView rhv) noexcept {
    return lhv.equals(rhv);
}

inline
bool operator!= (StringView lhv, StringView rhv) noexcept {
    return !lhv.equals(rhv);
}

inline
bool operator== (char const* rhv, StringView str) noexcept {
    return str.equals(rhv);
}

inline
bool operator== (StringView const& str, char const* rhv) noexcept {
    return str.equals(rhv);
}

inline
bool operator!= (char const* rhv, StringView const& str) noexcept {
    return !str.equals(rhv);
}

inline
bool operator!= (StringView const& str, char const* rhv) noexcept {
    return !str.equals(rhv);
}


inline
bool operator< (StringView const& lhs, StringView const& rhs) {
    return lhs.compareTo(rhs) < 0;
}


inline
bool operator<= (StringView const& lhs, StringView const& rhs) {
	return lhs.compareTo(rhs) <= 0;
}

inline
bool operator> (StringView const& lhs, StringView const& rhs) {
	return lhs.compareTo(rhs) > 0;
}

inline
bool operator>= (StringView const& lhs, StringView const& rhs) {
	return lhs.compareTo(rhs) >= 0;
}


}  // namespace Solace

/*
namespace std {
template <>
struct hash<Solace::StringView> {
    size_t operator() (Solace::StringView const& k) const noexcept {
        return k.hashCode();
    }
};

}  // namespace std
*/

#endif  // SOLACE_STRINGLITERAL_HPP
