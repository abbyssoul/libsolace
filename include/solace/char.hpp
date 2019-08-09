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
 *	@file		solace/char.hpp
 *	@brief		Basic Unicode character type / code point
 ******************************************************************************/
#pragma once
#ifndef SOLACE_CHAR_HPP
#define SOLACE_CHAR_HPP

#include "solace/types.hpp"
#include "solace/memoryView.hpp"


namespace Solace {

/**
 * Immutable UTF Character.
 *
 */
struct Char {

    // Widest code point is 4 bytes width.
	using value_type = uint32;
	using size_type = uint16;

    // Max bytes used for a representation
    static constexpr size_type max_bytes = sizeof(uint32);

public:
    /** Default constructor for <empty> character. */
	constexpr Char() noexcept
		: _value{}
    {}

    /** Construct new character from a byte value */
	constexpr Char(byte c) noexcept
		: _value{0}
	{
		_bytes[0] = c;
		_bytes[1] = 0;
	}

    /** Construct new character from an ASCII char */
    Char(char c);

    /** Construct new character from Unicode code-point value */
	Char(value_type codePoint) noexcept;

    /** Copy-Construct character. */
	Char(Char const& c) noexcept
		: _value(c._value)
	{
    }

    /** Move-Construct character. */
	Char(Char&& c) noexcept
		: _value{mv(c._value)}
	{
	}

    /** Construct new character from array of UTF-8 bytes (octets)
     *
     * @note: Size of array should be no more than <imp. def.> bytes
     */
	Char(MemoryView bytes);

    /** Returns the code-point value of the character. */
	constexpr value_type getValue() const noexcept {
        return _value;
    }

    /** Returns the code-point value of the character. */
    size_type getBytesCount() const noexcept;

    /** Get raw data representation of the code-point */
    // FIXME: No good reason to use it, ever!
    const char* c_str() const noexcept;

    /** Get raw bytes representation of the code-point */
	MemoryView getBytes() const;

    /** Returns true if this character is equal to given. */
	constexpr bool equals(Char const& rhs) const noexcept {
        return (getValue() == rhs.getValue());
    }

    /** Returns true if this character is not equal to given. */
	bool operator< (Char const& rhs) const noexcept {
        return (getValue() < rhs.getValue());
    }
    /** Returns true if this character is not equal to given. */
	bool operator<= (Char const& rhs) const noexcept {
        return (getValue() <= rhs.getValue());
    }
    /** Returns true if this character is not equal to given. */
	bool operator> (Char const& rhs) const noexcept {
        return (getValue() > rhs.getValue());
    }
    /** Returns true if this character is not equal to given. */
	bool operator>= (Char const& rhs) const noexcept {
        return (getValue() >= rhs.getValue());
    }

    //!< True is given character is a digit.
	bool isDigit() const noexcept;

    //!< True is given character is hex digit.
	bool isXDigit() const noexcept;

    //!< True is given character is a letter.
	bool isLetter() const noexcept;

    //!< True is given character is digit or letter.
	bool isLetterOrDigit() const noexcept;

    //!< True is given character is in upper case.
	bool isUpperCase() const noexcept;

    //!< True is given character is in lower case.
	bool isLowerCase() const noexcept;

    //!< True is given character is space.
	bool isWhitespace() const noexcept;

    //!< True if given character might be used as identifier.
	bool isSymbol() const noexcept;

    //!< True if given character might be used as first symbol of an identifier.
	bool isFirstSymbol() const noexcept;

    //!< True if given character is a new line character
	bool isNewLine() const noexcept;

    //!< Check if character is a control character
	bool isCntrl() const noexcept;

    //!< Check if character has graphical representation
	bool isGraphical() const noexcept;

    //!< Check if character is printable
	bool isPrintable() const noexcept;

    //!< Check if character is a punctuation character
	bool isPunctuation() const noexcept;

    /** Converts the character argument to lower case.
     *
     * @return Lower case of the character or argument if conversion is N/A
     */
    Char toLower() const;

    /** Converts the character argument to upper case.
     *
     * @return Upper case of the character or argument if conversion is N/A
     */
    Char toUpper() const;

public:	 // The only acceptable mutations:

    /** Value swap */
    Char& swap(Char& rhs) noexcept;

    /** Copy assignment */
	Char& operator= (Char const& rhs) noexcept {
        Char(rhs).swap(*this);

        return *this;
    }

    /** Move assignment */
    Char& operator= (Char&& rhs) noexcept {
        return swap(rhs);
    }

public:

    static const Char Eof;

public:

    //!< True if two given characters are equal.
	static bool equals(Char const& a, Char const& b) {
        return a.equals(b);
    }

    //!< True is given character is a digit.
	static bool isDigit(Char const& c) noexcept { return c.isDigit(); }

    //!< True is given character is hex digit.
	static bool isXDigit(Char const& c) noexcept { return c.isXDigit(); }

    //!< True is given character is a letter.
	static bool isLetter(Char const& c) noexcept { return c.isLetter(); }

    //!< True is given character is digit or letter.
	static bool isLetterOrDigit(Char const& c) noexcept { return c.isLetterOrDigit(); }

    //!< True is given character is in upper case.
	static bool isUpperCase(Char const& c) noexcept { return c.isUpperCase(); }

    //!< True is given character is in lower case.
	static bool isLowerCase(Char const& c) noexcept { return c.isLowerCase(); }

    //!< True is given character is space.
	static bool isWhitespace(Char const& c) noexcept { return c.isWhitespace(); }

    /** Determine if a character may be used in an identifier.
     *
     * @return True if given character might be used as identifier.
     */
	static bool isSymbol(Char const& c) noexcept { return c.isSymbol(); }

    /** Determine if a character may be used in an identifier.
     *
     * @return True if given character might be used as first symbol of an identifier.
     */
	static bool isFirstSymbol(Char const& c) noexcept { return c.isFirstSymbol(); }

    /** Determine if a character is a new line character
     *
     * @return True if given character is a new line character
     */
	static bool isNewLine(Char const& c) noexcept { return c.isNewLine(); }

    //!< Check if character is a control character
	static bool isCntrl(Char const& c) noexcept { return c.isCntrl(); }

    //!< Check if character has graphical representation
	static bool isGraphical(Char const& c) noexcept { return c.isGraphical(); }

    //!< Check if character is printable
	static bool isPrintable(Char const& c) noexcept { return c.isPrintable(); }

    //!< Check if character is a punctuation character
	static bool isPunctuation(Char const& c) noexcept { return c.isPunctuation(); }

    /** Converts the character argument to lower case.
     *
     * @return Lower case of the character or argument if conversion is N/A
     */
	static Char toLower(Char const& c) noexcept { return c.toLower(); }

    /** Converts the character argument to upper case.
     *
     * @return Upper case of the character or argument if conversion is N/A
     */
	static Char toUpper(Char const& c) noexcept { return c.toUpper(); }

private:

    union {
        value_type 	_value;
        byte		_bytes[max_bytes + 1];
    };  // Unicode code-point

};


inline void swap(Char& lhs, Char& rhs) noexcept {
    lhs.swap(rhs);
}

}  // End of namespace Solace
#endif  // SOLACE_CHAR_HPP
