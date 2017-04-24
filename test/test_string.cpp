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
 * libSolace Unit Test Suit
 *	@file		test/test_string.cpp
 *	@author		soutaker
 *
 ******************************************************************************/
#include <solace/string.hpp>  // Class being tested
#include <cppunit/extensions/HelperMacros.h>


#include <cstring>

using namespace Solace;

class TestString : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestString);
        CPPUNIT_TEST(testConstruction);
        CPPUNIT_TEST(testAssignment);
        CPPUNIT_TEST(testEquality);
        CPPUNIT_TEST(testContains);
        CPPUNIT_TEST(testLength);
        CPPUNIT_TEST(testReplace);
        CPPUNIT_TEST(testSplit);
        CPPUNIT_TEST(testIndexOf);
        CPPUNIT_TEST(testLastIndexOf);
        CPPUNIT_TEST(testConcat);
        CPPUNIT_TEST(testSubstring);
        CPPUNIT_TEST(testTrim);
        CPPUNIT_TEST(testToLowerCase);
        CPPUNIT_TEST(testToUpperCase);
        CPPUNIT_TEST(testStartsWith);
        CPPUNIT_TEST(testEndsWith);
        CPPUNIT_TEST(testHashCode);
// TODO(abbyssoul): CPPUNIT_TEST(testFormat);
// TODO(abbyssoul): CPPUNIT_TEST(testValueOf);
        CPPUNIT_TEST(testToString);
        CPPUNIT_TEST(test_cstr);
        CPPUNIT_TEST(test_iterable_forEach);

// TODO(abbyssoul):    CPPUNIT_TEST(testJoin);
// TODO(abbyssoul): Implement null safety first:
        CPPUNIT_TEST(testConstruction_null);
	CPPUNIT_TEST_SUITE_END();

private:

    typedef Array<String>::size_type array_size_t;

	static const char* someConstString;

public:

    void setUp() override {
	}

    void tearDown() override {
	}

	static String moveMe() {
		return String { someConstString };
	}

	void testConstruction_null() {
        {   // NullPointer smoke test
            const char* nullCString = nullptr;

            CPPUNIT_ASSERT_THROW(String nullString(nullCString), std::logic_error);
        }


        // Null string is null, cap
        /*
        CPPUNIT_ASSERT(nullString.empty());
        CPPUNIT_ASSERT(nullString.equals(String::Null));

        CPPUNIT_ASSERT(String::Null.equals(nullCString));
        CPPUNIT_ASSERT_EQUAL(0, String::Null.compareTo(nullCString));
        CPPUNIT_ASSERT_EQUAL(false, nullString.contains(nullCString));

        CPPUNIT_ASSERT_EQUAL(nullCString, nullString.c_str());
        CPPUNIT_ASSERT_EQUAL(nullString, nullString.concat(nullCString));

        {
            const String v("Some random string");

            CPPUNIT_ASSERT(!v.equals(String::Null));
            CPPUNIT_ASSERT(!v.equals(nullCString));

            CPPUNIT_ASSERT(v.indexOf(nullCString).isNone());
            CPPUNIT_ASSERT(v.lastIndexOf(nullCString).isNone());
            CPPUNIT_ASSERT_EQUAL(0, v.compareTo(nullCString));
            CPPUNIT_ASSERT_EQUAL(false, v.contains(nullCString));
            CPPUNIT_ASSERT_EQUAL(v, v.concat(nullCString));
            CPPUNIT_ASSERT_EQUAL(v, nullString.concat(v));
        }*/
	}

	/**
	 * Test construction calls
	 */
	void testConstruction() {
		const String empty;
		const String cstr(someConstString);

        const char* source = "some cstr source";
        const String cstrFromTo(source + 5, 4);
        CPPUNIT_ASSERT(cstrFromTo.equals("cstr"));

        CPPUNIT_ASSERT(String(wrapMemory(const_cast<char*>(source), 4)).equals("some"));
        CPPUNIT_ASSERT(String(wrapMemory(const_cast<char*>(source) + 10, 6)).equals("source"));


		const String strCopy(cstr);

		const String moved = moveMe();

        CPPUNIT_ASSERT(empty.equals(String::Empty));
        CPPUNIT_ASSERT(cstr.equals(someConstString));

        // FIXME(abbyssoul): Should be using buffer
        CPPUNIT_ASSERT(strCopy.equals(cstr));
	}

	/**
	 * Tests assignment
	 */
	void testAssignment() {
		String str1;
		String substr;
        CPPUNIT_ASSERT(str1.empty());
		{
			const String str2("some string");
			str1 = str2;
			substr = str2.substring(5, 3);
		}

        CPPUNIT_ASSERT_EQUAL(String("some string"), str1);
        CPPUNIT_ASSERT_EQUAL(String("str"), substr);
	}

	/**
	 * @brief Test string equality functions.
	 */
	void testEquality() {
		static const char* source1 = "some test string";
		static const char* source2 = "some other test string";
		static const char* source3 = "some test string";

		const String str1 = source1;
		const String str2 = source2;
		const String str3 = source3;
		const String str4 = str3;

        // Test if It is reflexive
        CPPUNIT_ASSERT(String::Empty.equals(String::Empty));
        CPPUNIT_ASSERT(str1.equals(str1));
        CPPUNIT_ASSERT(str1.equals(source1));
        CPPUNIT_ASSERT(str1 == source1);
        CPPUNIT_ASSERT(str1 == str1);

        CPPUNIT_ASSERT(str2.equals(source2));
        CPPUNIT_ASSERT(str2 == source2);

        CPPUNIT_ASSERT(str1 != source2);
        CPPUNIT_ASSERT(str2 != str1);
        CPPUNIT_ASSERT(str1 != str2);

		// Test if It is symmetric
        CPPUNIT_ASSERT(str1.equals(source3));
        CPPUNIT_ASSERT(str3.equals(source1));
        CPPUNIT_ASSERT(str1.equals(str3));
        CPPUNIT_ASSERT(str3.equals(str1));

		// Test if It is transitive
        CPPUNIT_ASSERT(str1.equals(str3));
        CPPUNIT_ASSERT(str3.equals(str4));
        CPPUNIT_ASSERT(str4.equals(str1));

        CPPUNIT_ASSERT(str1 == source3);
        CPPUNIT_ASSERT(str3 == source1);

        CPPUNIT_ASSERT(str1 == str3);
        CPPUNIT_ASSERT(str3 == str1);

        CPPUNIT_ASSERT_EQUAL(str1, str3);
        CPPUNIT_ASSERT_EQUAL(str3, str1);
	}

	void testContains() {
		const String source("Hello, world!  ");
		const String world("world");

        CPPUNIT_ASSERT(source.contains(world));
        CPPUNIT_ASSERT(source.contains('!'));
        CPPUNIT_ASSERT(!source.contains("a"));
	}

	void testLength() {
		const String source("");
		const String world("world");
        // FIXME: Add utf9 example

		CPPUNIT_ASSERT_EQUAL(static_cast<String::size_type>(0), source.length());
		CPPUNIT_ASSERT_EQUAL(static_cast<String::size_type>(5), world.length());
	}

	void testReplace() {
		const String source("attraction{holder}");
		const String dest0("aXXracXion{holder}");
		const String dest1("aWORDraction{holder}");
		const String dest2("attractionVALUE");
		const String value("VALUE");

        CPPUNIT_ASSERT_EQUAL(source, source.replace(value, "{holder}"));
		CPPUNIT_ASSERT_EQUAL(dest0, source.replace('t', 'X'));
		CPPUNIT_ASSERT_EQUAL(dest1, source.replace("tt", "WORD"));
		CPPUNIT_ASSERT_EQUAL(dest2, source.replace("{holder}", value));
	}

	void testSplit() {
		const String dest0("boo");
		const String dest1("and");
		const String dest2("foo");
		const String source("boo:and:foo");

        {   // Normal split
			auto result = source.split(":");

            CPPUNIT_ASSERT_EQUAL(static_cast<array_size_t>(3), result.size());
			CPPUNIT_ASSERT_EQUAL(dest0, result[0]);
			CPPUNIT_ASSERT_EQUAL(dest1, result[1]);
			CPPUNIT_ASSERT_EQUAL(dest2, result[2]);
		}

        {   // No splitting token in the string
			auto result = dest0.split(":");

            CPPUNIT_ASSERT_EQUAL(static_cast<array_size_t>(1), result.size());
			CPPUNIT_ASSERT_EQUAL(dest0, result[0]);
		}

        {   // No splitting token in the string
			auto result = source.split("/");

            CPPUNIT_ASSERT_EQUAL(static_cast<array_size_t>(1), result.size());
			CPPUNIT_ASSERT_EQUAL(source, result[0]);
		}

        {   // No splitting token in the string and wrong RegExp
            auto result = source.split("1");

            CPPUNIT_ASSERT_EQUAL(static_cast<array_size_t>(1), result.size());
            CPPUNIT_ASSERT_EQUAL(source, result[0]);
        }

        {   // No splitting token in the string
            auto result = String(":foo").split(":");

            CPPUNIT_ASSERT_EQUAL(static_cast<array_size_t>(2), result.size());
            CPPUNIT_ASSERT_EQUAL(String::Empty, result[0]);
            CPPUNIT_ASSERT_EQUAL(dest2, result[1]);
        }
    }

	void testIndexOf() {
		const String source("Hello, World! Good bye, World ");
		const String world("World");

		// Happy case:
        CPPUNIT_ASSERT_EQUAL(static_cast<String::size_type>(7), source.indexOf(world).get());
        CPPUNIT_ASSERT_EQUAL(static_cast<String::size_type>(12), source.indexOf('!').get());
        CPPUNIT_ASSERT_EQUAL(static_cast<String::size_type>(24), source.indexOf(world, 12).get());
        CPPUNIT_ASSERT_EQUAL(static_cast<String::size_type>(19), source.indexOf("bye", 3).get());

		// Fail case:
        CPPUNIT_ASSERT(source.indexOf("awesome").isNone());
        CPPUNIT_ASSERT(source.indexOf("World", source.length() - 3).isNone());
        CPPUNIT_ASSERT(source.indexOf("World", source.length() + 3).isNone());
        CPPUNIT_ASSERT(source.indexOf('!', source.length() - 3).isNone());
        CPPUNIT_ASSERT(source.indexOf('!', source.length()).isNone());
        CPPUNIT_ASSERT(source.indexOf('!', source.length() + 25).isNone());

        CPPUNIT_ASSERT(world.indexOf(source).isNone());
        CPPUNIT_ASSERT(world.indexOf("Some very long and obscure string??").isNone());
        CPPUNIT_ASSERT(world.indexOf(source, world.size() + 3).isNone());
        CPPUNIT_ASSERT(world.indexOf("Some very long and obscure string??", world.size() + 3).isNone());

        CPPUNIT_ASSERT(world.indexOf('/').isNone());
        CPPUNIT_ASSERT(world.indexOf('!', 3321).isNone());
        CPPUNIT_ASSERT(world.indexOf('!', source.length() + 25).isNone());
	}

	void testLastIndexOf() {
		const String source("Hello, World! Good bye, World - and again!");
		const String world("World");

        CPPUNIT_ASSERT_EQUAL(static_cast<String::size_type>(24), source.lastIndexOf(world).get());
        CPPUNIT_ASSERT_EQUAL(static_cast<String::size_type>(41), source.lastIndexOf('!').get());
        CPPUNIT_ASSERT_EQUAL(static_cast<String::size_type>(24), source.lastIndexOf(world, 12).get());
        CPPUNIT_ASSERT_EQUAL(static_cast<String::size_type>(19), source.lastIndexOf("bye", 12).get());

		// Fail case:
        CPPUNIT_ASSERT(source.lastIndexOf('!', source.length()).isNone());
        CPPUNIT_ASSERT(source.lastIndexOf('!', source.length() + 25).isNone());
        CPPUNIT_ASSERT(world.lastIndexOf(source).isNone());
        CPPUNIT_ASSERT(world.lastIndexOf("Some very long and obscure string??").isNone());
        CPPUNIT_ASSERT(world.lastIndexOf(source, world.size() + 3).isNone());
        CPPUNIT_ASSERT(world.lastIndexOf("Some very long and obscure string??", world.size() + 3).isNone());

        CPPUNIT_ASSERT(world.lastIndexOf('/').isNone());
	}

	void testConcat() {
		const String hello("Hello");
		const String space(", ");
		const String world("world!");
		const String target("Hello, world!");

		CPPUNIT_ASSERT_EQUAL(hello, String::Empty.concat(hello));
		CPPUNIT_ASSERT_EQUAL(hello, hello.concat(String::Empty));

		CPPUNIT_ASSERT_EQUAL(target, hello.concat(space.concat(world)));
        CPPUNIT_ASSERT_EQUAL(target, hello.concat(", world!"));
	}

	void testSubstring() {
		const String source("Hello, World! Good bye, World - and again!");
		const String world("World");
        const String bye("bye");
		const String and_again("and again!");

		CPPUNIT_ASSERT_EQUAL(world, source.substring(7, 5));
        CPPUNIT_ASSERT_EQUAL(bye, source.substring(source.indexOf(bye).get(), bye.length()));
        CPPUNIT_ASSERT_EQUAL(and_again, source.substring(source.indexOf(and_again).get()));
        CPPUNIT_ASSERT_EQUAL(String("Good bye, World - and again!"),
                             source.substring(source.indexOf("Good").get(), 1042));
	}

	/**
	 * Test string trim method.
	 */
	void testTrim() {
		String testString;

        CPPUNIT_ASSERT(testString.empty());
        CPPUNIT_ASSERT(testString.trim().empty());

        // Total trim
        CPPUNIT_ASSERT(String("   ").trim().empty());

        // Trim identity
        {
            const String trimmed("Hello, world!");
            const String toTrim("Hello, world!");
            testString = toTrim.trim();
            CPPUNIT_ASSERT(testString == toTrim);
            CPPUNIT_ASSERT_EQUAL(trimmed, testString);
        }

        // Trim start
        {
            const String trimmed("Hello, world!");
            const String toTrim(" Hello, world!");
            testString = toTrim.trim();
            CPPUNIT_ASSERT(testString != toTrim);
            CPPUNIT_ASSERT_EQUAL(trimmed, testString);
        }

		// Trim both
        {
            const String trimmed("Hello, world!");
            const String toTrim("  Hello, world!  ");
            testString = toTrim.trim();
            CPPUNIT_ASSERT(testString != toTrim);
            CPPUNIT_ASSERT_EQUAL(trimmed, testString);
        }

		// Trim End
        {
            const String trimmed("Hello, world !");
            const String toTrim("Hello, world !  ");
            testString = toTrim.trim();
            CPPUNIT_ASSERT(testString != toTrim);
            CPPUNIT_ASSERT_EQUAL(trimmed, testString);
        }
	}

	/**
	 * Test string's toLowerCase
	 */
	void testToLowerCase() {
		// Lower case -> lower case
		{
			const String lowerCaseSource("hello there");
			const String lowerCase_toLower = lowerCaseSource.toLowerCase();

			// Identity:
			CPPUNIT_ASSERT_EQUAL(lowerCaseSource, lowerCase_toLower);
		}

		// Mixed case -> lower case
		{
			const String mixedCaseSource("Hello Out There!");
			const String mixedCaseSource_toLower = mixedCaseSource.toLowerCase();

			CPPUNIT_ASSERT(mixedCaseSource != mixedCaseSource_toLower);
			CPPUNIT_ASSERT(mixedCaseSource_toLower.equals("hello out there!"));
		}

		// Upper case -> lower case
		{
			const String upperCaseSource("THERE@OUT*HELLO*&^%");
			const String upperCaseSource_toLower = upperCaseSource.toLowerCase();

			CPPUNIT_ASSERT(upperCaseSource != upperCaseSource_toLower);
			CPPUNIT_ASSERT(upperCaseSource_toLower.equals("there@out*hello*&^%"));
		}
	}

	/**
	 * Test string's toUpperCase
	 */
	void testToUpperCase() {
		// Lower case -> upper case
		{
			const String lowerCaseSource("hello@there-out*&%!1");
			const String lowerCase_toUpper = lowerCaseSource.toUpperCase();

            CPPUNIT_ASSERT(lowerCaseSource != lowerCase_toUpper);
			CPPUNIT_ASSERT_EQUAL(String("HELLO@THERE-OUT*&%!1"), lowerCase_toUpper);
		}

		// Mixed case -> lower case
		{
			const String mixedCaseSource("Hello @ Out^&There!");
			const String mixedCaseSource_toUpper = mixedCaseSource.toUpperCase();

			CPPUNIT_ASSERT(mixedCaseSource != mixedCaseSource_toUpper);
			CPPUNIT_ASSERT_EQUAL(String("HELLO @ OUT^&THERE!"), mixedCaseSource_toUpper);
		}

		// Upper case -> lower case
		{
			const String upperCaseSource("THERE@OUT*HELLO*&^%");
			const String upperCaseSource_toUpper = upperCaseSource.toUpperCase();

			// Identity
			CPPUNIT_ASSERT_EQUAL(upperCaseSource, upperCaseSource_toUpper);
		}
	}

	/**
	 * Test string's 'startsWith'
	 */
	void testStartsWith() {
		const String source("Hello, world out there!");
		const String hello("Hello");
		const String there("there!");
		const String overlong("Hello, world out there! And here!");

        CPPUNIT_ASSERT(!String::Empty.startsWith('H'));
        CPPUNIT_ASSERT(!String::Empty.startsWith("Something"));
        CPPUNIT_ASSERT(!source.startsWith(String::Empty));
        CPPUNIT_ASSERT(source.startsWith('H'));
        CPPUNIT_ASSERT(source.startsWith(hello));
        CPPUNIT_ASSERT(!source.startsWith(there));
        CPPUNIT_ASSERT(!source.startsWith(overlong));
        CPPUNIT_ASSERT(!source.startsWith("Some very long statement that can't possibly feet"));
	}

	/**
	 * Test string's 'startsWith'
	 */
	void testEndsWith() {
		const String source("Hello, world out there !");
		const String hello("Hello");
		const String there("there !");
		const String overlong("Hello, world out there ! And here!");

        CPPUNIT_ASSERT(!String::Empty.endsWith('x'));
        CPPUNIT_ASSERT(!String::Empty.endsWith("Something"));
        CPPUNIT_ASSERT(!source.endsWith(String::Empty));
        CPPUNIT_ASSERT(source.endsWith('!'));
        CPPUNIT_ASSERT(source.endsWith(source));
        CPPUNIT_ASSERT(!source.endsWith(hello));
        CPPUNIT_ASSERT(source.endsWith(there));
        CPPUNIT_ASSERT(!source.endsWith(overlong));
        CPPUNIT_ASSERT(!source.endsWith("Some very long statement that can't possibly feet"));
	}

	/**
	 * Test string's 'hashCode' method.
	 */
	void testHashCode() {
		const String testString1 = "Hello otu there";
		const String testString2 = "Hello out there";

        CPPUNIT_ASSERT(testString1.hashCode() != 0);
        CPPUNIT_ASSERT(testString2.hashCode() != 0);
        CPPUNIT_ASSERT(testString1 != testString2);
        CPPUNIT_ASSERT(testString1.hashCode() != testString2.hashCode());
	}

	/**
	 * Test string's 'format' methods.
	 */
	/*
	void testFormat() {
		const String hello = "Hello";

		// Format
		// Tests Identity:
        CPPUNIT_ASSERT_EQUAL(hello, String::format(hello));
        CPPUNIT_ASSERT_EQUAL(hello, String::format("%s", hello));

        CPPUNIT_ASSERT_EQUAL(hello, String::format(hello));

        CPPUNIT_ASSERT_EQUAL(String("1"), String::format("%d", 1));
        CPPUNIT_ASSERT_EQUAL(String("CSTR"), String::format("%s", "CSTR"));
        CPPUNIT_ASSERT_EQUAL(String("Hello2World"), String::format("%s", hello, 2, "World"));

        CPPUNIT_ASSERT_EQUAL(String::Empty, String::format("", hello));

        CPPUNIT_ASSERT_EQUAL(String("xxx"), String::format("xxx", hello));
        CPPUNIT_ASSERT_THROW(String::format("xxx%s", hello, "bbb"), std::runtime_error);
        CPPUNIT_ASSERT_THROW(String::format("%sxxx%szxc", hello), std::runtime_error);

        // Append format
        CPPUNIT_ASSERT_NO_THROW(testString1.appendFormat(strT("hello")));
        CPPUNIT_ASSERT_EQUAL(String(strT("hello")), testString1);

        CPPUNIT_ASSERT_NO_THROW(testString1.appendFormat(strT("%d"), 1));
        CPPUNIT_ASSERT_EQUAL(String(strT("hello1")), testString1);

        CPPUNIT_ASSERT_NO_THROW(testString1.appendFormat(strT(": %s"), testString2));
        CPPUNIT_ASSERT_EQUAL(String(strT("hello1: Hello, world!")), testString1);

        // Printf
        CPPUNIT_ASSERT_NO_THROW(testString1.printf(strT("hello")));
        CPPUNIT_ASSERT_EQUAL(String(strT("hello")), testString1);

        CPPUNIT_ASSERT_NO_THROW(testString1.printf(strT("%d"), 1));
        CPPUNIT_ASSERT_EQUAL(String(strT("1")), testString1);

        CPPUNIT_ASSERT_NO_THROW(testString1.printf(strT("%s"), str1));
        CPPUNIT_ASSERT_EQUAL(String(str1), testString1);
	}
*/

	void testToString() {
		const String ident(someConstString);

        CPPUNIT_ASSERT_EQUAL(ident,  ident.toString());
	}

	void test_cstr() {
        {   // Empty sting is empty
			const char* bait = "";
			const String str("");

            CPPUNIT_ASSERT(str.empty());

			CPPUNIT_ASSERT_EQUAL(0, strcmp(bait, str.c_str()));
		}
        {   // Non-empty sting is non-empty
			const String str(someConstString);

            CPPUNIT_ASSERT(!str.empty());

			CPPUNIT_ASSERT_EQUAL(0, strcmp(someConstString, str.c_str()));
		}
	}

    void test_iterable_forEach() {
        const String ident(someConstString);

        int summ = 0;
        for (auto c : ident.getBytes()) {
            summ += c;
        }

        ident.forEach([&summ](const Char& c) {
            summ -= c.getValue();
        });

        CPPUNIT_ASSERT_EQUAL(0, summ);
    }

};

const char* TestString::someConstString = "Some static string";

CPPUNIT_TEST_SUITE_REGISTRATION(TestString);
