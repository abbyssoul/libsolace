#include <solace/string.hpp>  // Class being tested
#include <solace/exception.hpp>

#include <gtest/gtest.h>

#include <cstring>

using namespace Solace;

class TestString : public ::testing::Test {

public:

    typedef Array<String>::size_type array_size_t;

	static const char* someConstString;

    void setUp() {
	}

    void tearDown() {
	}

	static String moveMe() {
		return String { someConstString };
	}
};

TEST_F(TestString, testConstruction_null) {
    {   // NullPointer smoke test
        const char* nullCString = nullptr;

        EXPECT_THROW(String nullString(nullCString), Exception);
    }


    // Null string is null, cap
    /*
    EXPECT_TRUE(nullString.empty());
    EXPECT_TRUE(nullString.equals(String::Null));

    EXPECT_TRUE(String::Null.equals(nullCString));
    EXPECT_EQ(0, String::Null.compareTo(nullCString));
    EXPECT_EQ(false, nullString.contains(nullCString));

    EXPECT_EQ(nullCString, nullString.c_str());
    EXPECT_EQ(nullString, nullString.concat(nullCString));

    {
        const String v("Some random string");

        EXPECT_TRUE(!v.equals(String::Null));
        EXPECT_TRUE(!v.equals(nullCString));

        EXPECT_TRUE(v.indexOf(nullCString).isNone());
        EXPECT_TRUE(v.lastIndexOf(nullCString).isNone());
        EXPECT_EQ(0, v.compareTo(nullCString));
        EXPECT_EQ(false, v.contains(nullCString));
        EXPECT_EQ(v, v.concat(nullCString));
        EXPECT_EQ(v, nullString.concat(v));
    }*/
}

/**
    * Test construction calls
    */
TEST_F(TestString, testConstruction) {
    EXPECT_TRUE(String{}.equals(String::Empty));
    EXPECT_TRUE(String{}.empty());

    const char* source = "some cstr source";
    const String cstrFromTo(source + 5, 4);
    EXPECT_TRUE(cstrFromTo.equals("cstr"));


    const String cstr(someConstString);
    EXPECT_TRUE(cstr.equals(someConstString));

    const String moved = moveMe();


    const String strCopy(cstr);
    EXPECT_TRUE(strCopy.equals(cstr));
}

/**
    * Tests assignment
    */
TEST_F(TestString, testAssignment) {
    String str1;
    String substr;
    EXPECT_TRUE(str1.empty());
    {
        const String str2("some string");
        str1 = str2;
        substr = str2.substring(5, 3);
    }

    EXPECT_EQ(String("some string"), str1);
    EXPECT_EQ(String("str"), substr);
}

/**
    * @brief Test string equality functions.
    */
TEST_F(TestString, testEquality) {
    static const char* source1 = "some test string";
    static const char* source2 = "some other test string";
    static const char* source3 = "some test string";

    const String str1 = source1;
    const String str2 = source2;
    const String str3 = source3;
    const String str4 = str3;

    // Test if It is reflexive
    EXPECT_TRUE(String::Empty.equals(String::Empty));
    EXPECT_TRUE(str1.equals(str1));
    EXPECT_TRUE(str1.equals(source1));
    EXPECT_TRUE(str1 == source1);
    EXPECT_TRUE(str1 == str1);

    EXPECT_TRUE(str2.equals(source2));
    EXPECT_TRUE(str2 == source2);

    EXPECT_TRUE(str1 != source2);
    EXPECT_TRUE(str2 != str1);
    EXPECT_TRUE(str1 != str2);

    // Test if It is symmetric
    EXPECT_TRUE(str1.equals(source3));
    EXPECT_TRUE(str3.equals(source1));
    EXPECT_TRUE(str1.equals(str3));
    EXPECT_TRUE(str3.equals(str1));

    // Test if It is transitive
    EXPECT_TRUE(str1.equals(str3));
    EXPECT_TRUE(str3.equals(str4));
    EXPECT_TRUE(str4.equals(str1));

    EXPECT_TRUE(str1 == source3);
    EXPECT_TRUE(str3 == source1);

    EXPECT_TRUE(str1 == str3);
    EXPECT_TRUE(str3 == str1);

    EXPECT_EQ(str1, str3);
    EXPECT_EQ(str3, str1);
}

TEST_F(TestString, testContains) {
    const String source("Hello, world!  ");
    const String world("world");

    EXPECT_TRUE(source.contains(world));
    EXPECT_TRUE(source.contains('!'));
    EXPECT_TRUE(!source.contains("a"));
}

TEST_F(TestString, testLength) {
    const String source("");
    const String world("world");
    // FIXME: Add utf9 example

    EXPECT_EQ(static_cast<String::size_type>(0), source.length());
    EXPECT_EQ(static_cast<String::size_type>(5), world.length());
}

TEST_F(TestString, testReplace) {
    const String source("attraction{holder}");
    const String dest0("aXXracXion{holder}");
    const String dest1("aWORDraction{holder}");
    const String dest2("attractionVALUE");
    const String value("VALUE");

    EXPECT_EQ(source, source.replace(value, "{holder}"));
    EXPECT_EQ(dest0, source.replace('t', 'X'));
    EXPECT_EQ(dest1, source.replace("tt", "WORD"));
    EXPECT_EQ(dest2, source.replace("{holder}", value));
}

TEST_F(TestString, testSplit) {
    const String dest0("boo");
    const String dest1("and");
    const String dest2("foo");
    const String source("boo:and:foo");

    {   // Normal split
        auto result = source.split(":");

        EXPECT_EQ(static_cast<array_size_t>(3), result.size());
        EXPECT_EQ(dest0, result[0]);
        EXPECT_EQ(dest1, result[1]);
        EXPECT_EQ(dest2, result[2]);
    }

    {   // No splitting token in the string
        auto result = dest0.split(":");

        EXPECT_EQ(static_cast<array_size_t>(1), result.size());
        EXPECT_EQ(dest0, result[0]);
    }

    {   // No splitting token in the string
        auto result = source.split("/");

        EXPECT_EQ(static_cast<array_size_t>(1), result.size());
        EXPECT_EQ(source, result[0]);
    }

    {   // No splitting token in the string and wrong RegExp
        auto result = source.split("1");

        EXPECT_EQ(static_cast<array_size_t>(1), result.size());
        EXPECT_EQ(source, result[0]);
    }

    {   // No splitting token in the string
        auto result = String(":foo").split(":");

        EXPECT_EQ(static_cast<array_size_t>(2), result.size());
        EXPECT_EQ(String::Empty, result[0]);
        EXPECT_EQ(dest2, result[1]);
    }
}

TEST_F(TestString, testIndexOf) {
    const String source("Hello, World! Good bye, World ");
    const String world("World");

    // Happy case:
    EXPECT_EQ(static_cast<String::size_type>(7), source.indexOf(world).get());
    EXPECT_EQ(static_cast<String::size_type>(12), source.indexOf('!').get());
    EXPECT_EQ(static_cast<String::size_type>(24), source.indexOf(world, 12).get());
    EXPECT_EQ(static_cast<String::size_type>(19), source.indexOf("bye", 3).get());

    // Fail case:
    EXPECT_TRUE(source.indexOf("awesome").isNone());
    EXPECT_TRUE(source.indexOf("World", source.length() - 3).isNone());
    EXPECT_TRUE(source.indexOf("World", source.length() + 3).isNone());
    EXPECT_TRUE(source.indexOf('!', source.length() - 3).isNone());
    EXPECT_TRUE(source.indexOf('!', source.length()).isNone());
    EXPECT_TRUE(source.indexOf('!', source.length() + 25).isNone());

    EXPECT_TRUE(world.indexOf(source).isNone());
    EXPECT_TRUE(world.indexOf("Some very long and obscure string??").isNone());
    EXPECT_TRUE(world.indexOf(source, world.size() + 3).isNone());
    EXPECT_TRUE(world.indexOf("Some very long and obscure string??", world.size() + 3).isNone());

    EXPECT_TRUE(world.indexOf('/').isNone());
    EXPECT_TRUE(world.indexOf('!', 3321).isNone());
    EXPECT_TRUE(world.indexOf('!', source.length() + 25).isNone());
}

TEST_F(TestString, testLastIndexOf) {
    const String source("Hello, World! Good bye, World - and again!");
    const String world("World");

    EXPECT_EQ(static_cast<String::size_type>(24), source.lastIndexOf(world).get());
    EXPECT_EQ(static_cast<String::size_type>(41), source.lastIndexOf('!').get());
    EXPECT_EQ(static_cast<String::size_type>(24), source.lastIndexOf(world, 12).get());
    EXPECT_EQ(static_cast<String::size_type>(19), source.lastIndexOf("bye", 12).get());

    // Fail case:
    EXPECT_TRUE(source.lastIndexOf('!', source.length()).isNone());
    EXPECT_TRUE(source.lastIndexOf('!', source.length() + 25).isNone());
    EXPECT_TRUE(world.lastIndexOf(source).isNone());
    EXPECT_TRUE(world.lastIndexOf("Some very long and obscure string??").isNone());
    EXPECT_TRUE(world.lastIndexOf(source, world.size() + 3).isNone());
    EXPECT_TRUE(world.lastIndexOf("Some very long and obscure string??", world.size() + 3).isNone());

    EXPECT_TRUE(world.lastIndexOf('/').isNone());
}

TEST_F(TestString, testConcat) {
    const String hello("Hello");
    const String space(", ");
    const String world("world!");
    const String target("Hello, world!");

    EXPECT_EQ(hello, String::Empty.concat(hello));
    EXPECT_EQ(hello, hello.concat(String::Empty));

    EXPECT_EQ(target, hello.concat(space.concat(world)));
    EXPECT_EQ(target, hello.concat(", world!"));
}

/**
    * @see String::substring
    */
TEST_F(TestString, testSubstring) {
    const String source("Hello, World! Good bye, World - and again!");
    const String world("World");
    const String bye("bye");
    const String and_again("and again!");

    EXPECT_EQ(world, source.substring(7, 5));
    EXPECT_EQ(bye, source.substring(source.indexOf(bye).get(), bye.length()));
    EXPECT_EQ(and_again, source.substring(source.indexOf(and_again).get()));
    EXPECT_EQ(String("Good bye, World - and again!"),
                            source.substring(source.indexOf("Good").get(), 1042));
}

/**
    * @see String::trim
    */
TEST_F(TestString, testTrim) {
    String testString;

    EXPECT_TRUE(testString.empty());
    EXPECT_TRUE(testString.trim().empty());

    // Total trim
    EXPECT_TRUE(String("   ").trim().empty());

    // Trim identity
    {
        const String trimmed("Hello, world!");
        const String toTrim("Hello, world!");
        testString = toTrim.trim();
        EXPECT_TRUE(testString == toTrim);
        EXPECT_EQ(trimmed, testString);
    }

    // Trim start
    {
        const String trimmed("Hello, world!");
        const String toTrim(" Hello, world!");
        testString = toTrim.trim();
        EXPECT_TRUE(testString != toTrim);
        EXPECT_EQ(trimmed, testString);
    }

    // Trim both
    {
        const String trimmed("Hello, world!");
        const String toTrim("  Hello, world!  ");
        testString = toTrim.trim();
        EXPECT_TRUE(testString != toTrim);
        EXPECT_EQ(trimmed, testString);
    }

    // Trim End
    {
        const String trimmed("Hello, world !");
        const String toTrim("Hello, world !  ");
        testString = toTrim.trim();
        EXPECT_TRUE(testString != toTrim);
        EXPECT_EQ(trimmed, testString);
    }
}

/**
    * Test string's toLowerCase
    */
TEST_F(TestString, testToLowerCase) {
    // Lower case -> lower case
    {
        const String lowerCaseSource("hello there");
        const String lowerCase_toLower = lowerCaseSource.toLowerCase();

        // Identity:
        EXPECT_EQ(lowerCaseSource, lowerCase_toLower);
    }

    // Mixed case -> lower case
    {
        const String mixedCaseSource("Hello Out There!");
        const String mixedCaseSource_toLower = mixedCaseSource.toLowerCase();

        EXPECT_TRUE(mixedCaseSource != mixedCaseSource_toLower);
        EXPECT_TRUE(mixedCaseSource_toLower.equals("hello out there!"));
    }

    // Upper case -> lower case
    {
        const String upperCaseSource("THERE@OUT*HELLO*&^%");
        const String upperCaseSource_toLower = upperCaseSource.toLowerCase();

        EXPECT_TRUE(upperCaseSource != upperCaseSource_toLower);
        EXPECT_TRUE(upperCaseSource_toLower.equals("there@out*hello*&^%"));
    }
}

/**
    * Test string's toUpperCase
    */
TEST_F(TestString, testToUpperCase) {
    // Lower case -> upper case
    {
        const String lowerCaseSource("hello@there-out*&%!1");
        const String lowerCase_toUpper = lowerCaseSource.toUpperCase();

        EXPECT_TRUE(lowerCaseSource != lowerCase_toUpper);
        EXPECT_EQ(String("HELLO@THERE-OUT*&%!1"), lowerCase_toUpper);
    }

    // Mixed case -> lower case
    {
        const String mixedCaseSource("Hello @ Out^&There!");
        const String mixedCaseSource_toUpper = mixedCaseSource.toUpperCase();

        EXPECT_TRUE(mixedCaseSource != mixedCaseSource_toUpper);
        EXPECT_EQ(String("HELLO @ OUT^&THERE!"), mixedCaseSource_toUpper);
    }

    // Upper case -> lower case
    {
        const String upperCaseSource("THERE@OUT*HELLO*&^%");
        const String upperCaseSource_toUpper = upperCaseSource.toUpperCase();

        // Identity
        EXPECT_EQ(upperCaseSource, upperCaseSource_toUpper);
    }
}

/**
    * Test string's 'startsWith'
    */
TEST_F(TestString, testStartsWith) {
    const String source("Hello, world out there!");
    const String hello("Hello");
    const String there("there!");
    const String overlong("Hello, world out there! And here!");

    EXPECT_TRUE(!String::Empty.startsWith('H'));
    EXPECT_TRUE(!String::Empty.startsWith("Something"));
    EXPECT_TRUE(!source.startsWith(String::Empty));
    EXPECT_TRUE(source.startsWith('H'));
    EXPECT_TRUE(source.startsWith(hello));
    EXPECT_TRUE(!source.startsWith(there));
    EXPECT_TRUE(!source.startsWith(overlong));
    EXPECT_TRUE(!source.startsWith("Some very long statement that can't possibly feet"));
}

/**
    * Test string's 'endsWith'
    */
TEST_F(TestString, testEndsWith) {
    const String source("Hello, world out there !");
    const String hello("Hello");
    const String there("there !");
    const String overlong("Hello, world out there ! And here!");

    EXPECT_TRUE(!String::Empty.endsWith('x'));
    EXPECT_TRUE(!String::Empty.endsWith("Something"));
    EXPECT_TRUE(!source.endsWith(String::Empty));
    EXPECT_TRUE(source.endsWith('!'));
    EXPECT_TRUE(source.endsWith(source));
    EXPECT_TRUE(!source.endsWith(hello));
    EXPECT_TRUE(source.endsWith(there));
    EXPECT_TRUE(!source.endsWith(overlong));
    EXPECT_TRUE(!source.endsWith("Some very long statement that can't possibly feet"));
}

/**
    * Test string's 'hashCode' method.
    */
TEST_F(TestString, testHashCode) {
    const String testString1 = "Hello otu there";
    const String testString2 = "Hello out there";

    EXPECT_TRUE(testString1.hashCode() != 0);
    EXPECT_TRUE(testString2.hashCode() != 0);
    EXPECT_TRUE(testString1 != testString2);
    EXPECT_TRUE(testString1.hashCode() != testString2.hashCode());
}

/**
    * Test string's 'format' methods.
    */
/*
void testFormat() {
    const String hello = "Hello";

    // Format
    // Tests Identity:
    EXPECT_EQ(hello, String::format(hello));
    EXPECT_EQ(hello, String::format("%s", hello));

    EXPECT_EQ(hello, String::format(hello));

    EXPECT_EQ(String("1"), String::format("%d", 1));
    EXPECT_EQ(String("CSTR"), String::format("%s", "CSTR"));
    EXPECT_EQ(String("Hello2World"), String::format("%s", hello, 2, "World"));

    EXPECT_EQ(String::Empty, String::format("", hello));

    EXPECT_EQ(String("xxx"), String::format("xxx", hello));
    EXPECT_THROW(String::format("xxx%s", hello, "bbb"), std::runtime_error);
    EXPECT_THROW(String::format("%sxxx%szxc", hello), std::runtime_error);

    // Append format
    EXPECT_NO_THROW(testString1.appendFormat(strT("hello")));
    EXPECT_EQ(String(strT("hello")), testString1);

    EXPECT_NO_THROW(testString1.appendFormat(strT("%d"), 1));
    EXPECT_EQ(String(strT("hello1")), testString1);

    EXPECT_NO_THROW(testString1.appendFormat(strT(": %s"), testString2));
    EXPECT_EQ(String(strT("hello1: Hello, world!")), testString1);

    // Printf
    EXPECT_NO_THROW(testString1.printf(strT("hello")));
    EXPECT_EQ(String(strT("hello")), testString1);

    EXPECT_NO_THROW(testString1.printf(strT("%d"), 1));
    EXPECT_EQ(String(strT("1")), testString1);

    EXPECT_NO_THROW(testString1.printf(strT("%s"), str1));
    EXPECT_EQ(String(str1), testString1);
}
*/

TEST_F(TestString, testToString) {
    const String ident(someConstString);

    EXPECT_EQ(ident,  ident.toString());
}

TEST_F(TestString, test_cstr) {
    {   // Empty sting is empty
        const char* bait = "";
        const String str("");

        EXPECT_TRUE(str.empty());

        EXPECT_EQ(0, strcmp(bait, str.c_str()));
    }
    {   // Non-empty sting is non-empty
        const String str(someConstString);

        EXPECT_TRUE(!str.empty());

        EXPECT_EQ(0, strcmp(someConstString, str.c_str()));
    }
}

TEST_F(TestString, test_iterable_forEach) {
//        const String ident(someConstString);

//        int summ = 0;
//        for (auto c : ident.view()) {
//            summ += c;
//        }

//        ident.forEach([&summ](const Char& c) {
//            summ -= c.getValue();
//        });

//        EXPECT_EQ(0, summ);
}

const char* TestString::someConstString = "Some static string";