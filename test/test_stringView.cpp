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
 *	@file		test/test_stringView.cpp
 *	@author		abbyssoul
 *
 ******************************************************************************/
 #include <solace/stringView.hpp>  // Class being tested
#include <solace/exception.hpp>

#include <gtest/gtest.h>

#include <cstring>

using namespace Solace;

using array_size_t = Array<StringView>::size_type;



TEST(TestStringView, testConstructionFromNull) {
    const char* nullCString = nullptr;

    EXPECT_THROW(StringView nullString(nullCString), Exception);
}

TEST(TestStringView, testConstructEmptyString) {
    StringView value;
    EXPECT_TRUE(value.empty());
    EXPECT_EQ(static_cast<StringView::size_type>(0), value.size());
    EXPECT_EQ(static_cast<StringView::size_type>(0), value.length());

    EXPECT_TRUE(StringView("").empty());
}

TEST(TestStringView, testConstructionUnsized) {
    const char *cstr = "world";
    EXPECT_TRUE(!StringView("Non-empty").empty());
    EXPECT_EQ(static_cast<StringView::size_type>(5), StringView(cstr).size());
    EXPECT_EQ(static_cast<StringView::size_type>(5), StringView(cstr).length());
    EXPECT_EQ(cstr, StringView(cstr).data());
}

TEST(TestStringView, testConstructionSized) {
    const char *cstr = "world";

    EXPECT_EQ(static_cast<StringView::size_type>(3), StringView(cstr, 3).length());
    EXPECT_EQ(static_cast<StringView::size_type>(3), StringView(&cstr[2], 3).length());
    EXPECT_EQ(cstr, StringView(cstr, 3).data());
    EXPECT_EQ(cstr + 2, StringView(&cstr[2], 3).data());
}

TEST(TestStringView, testAssignmentFromCstring) {
    StringView value("world");
    EXPECT_EQ(static_cast<StringView::size_type>(5), value.length());
    EXPECT_EQ(0, strcmp("world", value.data()));

    value = "Something else";
    EXPECT_EQ(static_cast<StringView::size_type>(14), value.size());
    EXPECT_EQ(0, strcmp("Something else", value.data()));
}


/**
    * @see StringView::operator=
    */
TEST(TestStringView, testAssignment) {
    StringView value("world");
    StringView other("Completely different value");

    value = other;

    EXPECT_EQ(static_cast<StringView::size_type>(26), value.length());
    EXPECT_EQ(0, strcmp("Completely different value", value.data()));
}


/**
    * @see StringView::equals
    * @see StringView::operator==
    */
TEST(TestStringView, testEquality) {
    StringView value1("hello");
    StringView value2("Completely different value");
    StringView value3("hello");

    EXPECT_TRUE(value1.equals("hello"));
    EXPECT_TRUE(!value1.equals("hello here"));
    EXPECT_TRUE(!value1.equals("here hello"));

    EXPECT_TRUE(value1.equals(value1));
    EXPECT_TRUE(!value1.equals(value2));
    EXPECT_TRUE(value1.equals(value3));
    EXPECT_TRUE(value3.equals(value1));

    EXPECT_EQ(value1, "hello");
    EXPECT_NE(value1, "hello here");
    EXPECT_NE(value1, "hi hello");

    EXPECT_TRUE(value1 == value1);
    EXPECT_TRUE(!(value1 == value2));
    EXPECT_TRUE(value1 != value2);
    EXPECT_TRUE(value1 == value3);
    EXPECT_TRUE(value3 == value1);
}

/**
    * @see StringView::length
    */
TEST(TestStringView, testLength) {
    // FIXME: Add utf9 example

    EXPECT_EQ(static_cast<StringView::size_type>(0), StringView().length());
    EXPECT_EQ(static_cast<StringView::size_type>(0), StringView("").length());
    EXPECT_EQ(static_cast<StringView::size_type>(5), StringView("world").length());
}


/**
    * @see StringView::startsWith
    */
TEST(TestStringView, testStartsWith) {
    EXPECT_TRUE(StringView().startsWith('\0'));
    EXPECT_TRUE(!StringView("Hello world").startsWith('\0'));
    EXPECT_TRUE(StringView("Hello world").startsWith('H'));
    EXPECT_TRUE(StringView("Hello world").startsWith("Hello"));
    EXPECT_TRUE(!StringView("Hello world").startsWith("world"));
    EXPECT_TRUE(!StringView("Some")
                    .startsWith("Some very long statement that can't possibly"));
}

/**
    * @see StringView::endsWith
    */
TEST(TestStringView, testEndsWith) {
    EXPECT_TRUE(!StringView("Hello world!").endsWith('\0'));
    EXPECT_TRUE(StringView("Hello world!").endsWith('!'));
    EXPECT_TRUE(StringView("Hello world!").endsWith("world!"));
    EXPECT_TRUE(!StringView("Hello world").endsWith("hello"));
    EXPECT_TRUE(!StringView("Hello world")
                    .endsWith("Some very long statement that can't possibly"));
}

/**
    * @see StringView::substring
    */
TEST(TestStringView, testSubstring) {
    const StringView source("Hello, world! Good bye, World - and again!");
    const StringView bye("bye");
    const StringView andAgain("and again!");

    EXPECT_EQ(StringView(),
                            StringView().substring(0));

    EXPECT_EQ(StringView("world"),
                            source.substring(7, 5));

    EXPECT_EQ(bye,
                            source.substring(source.indexOf(bye).get(), bye.length()));

    EXPECT_EQ(andAgain,
                            source.substring(source.indexOf(andAgain).get()));

    EXPECT_THROW(StringView("hi").substring(13),
                            Exception);

    EXPECT_THROW(StringView("hi").substring(0, 8),
                            Exception);

    EXPECT_THROW(StringView("hi").substring(2, 1),
                            Exception);
}

/**
    * @see StringView::trim
    */
TEST(TestStringView, testTrim) {
    EXPECT_TRUE(StringView().trim().empty());

    // Total trim
    EXPECT_TRUE(StringView("   ").trim().empty());

    // Trim identity
    EXPECT_TRUE(StringView("Hello, world!").trim().equals("Hello, world!"));

    // Trim start
    EXPECT_TRUE(StringView("  Hello, world!").trim().equals("Hello, world!"));

    // Trim both
    EXPECT_TRUE(StringView(" Hello, world!   ").trim().equals("Hello, world!"));

    // Trim End
    EXPECT_TRUE(StringView("Hello, world!  ").trim().equals("Hello, world!"));
}


/**
    * @see StringView::indexOf
    */
TEST(TestStringView, testIndexOf) {
    const StringView src("Hello, world! $$Blarg");

    // Happy case:
    EXPECT_EQ('!', src[src.indexOf('!').get()]);
    EXPECT_EQ(static_cast<StringView::size_type>(12),
                            src.indexOf('!').get());
    EXPECT_EQ(static_cast<StringView::size_type>(7),
                            src.indexOf("world").get());
    EXPECT_EQ(static_cast<StringView::size_type>(3),
                            StringView("hello").indexOf("lo").get());


    EXPECT_EQ(static_cast<StringView::size_type>(14),
                            src.indexOf('$', 12).get());
    EXPECT_EQ(static_cast<StringView::size_type>(15),
                            src.indexOf("$Bl", 3).get());

    // Not found case:
    EXPECT_TRUE(src.indexOf('!', 14).isNone());
    EXPECT_TRUE(src.indexOf("awesome").isNone());
    EXPECT_TRUE(src.indexOf("World", src.length() - 3).isNone());
    EXPECT_TRUE(StringView("hi").indexOf("hi, long string").isNone());

    // Fail case:
    EXPECT_THROW(StringView("hi").indexOf('i', 5),
                            Exception);
    EXPECT_THROW(StringView("hi").indexOf("hi", 5),
                            Exception);
}

/**
    * @see StringView::lastIndexOf
    */
TEST(TestStringView, testLastIndexOf) {
    const StringView source("Hello, World! Good bye, World - and again rld!");
    const StringView world("World");

    EXPECT_EQ('!', source[source.lastIndexOf('!').get()]);
    EXPECT_EQ(static_cast<StringView::size_type>(45), source.lastIndexOf('!').get());
    EXPECT_EQ(static_cast<StringView::size_type>(24), source.lastIndexOf(world).get());
    EXPECT_EQ(static_cast<StringView::size_type>(24), source.lastIndexOf(world, 12).get());
    EXPECT_EQ(static_cast<StringView::size_type>(42), source.lastIndexOf("rld!").get());

    // Not found case:
    EXPECT_TRUE(source.lastIndexOf('x').isNone());
    EXPECT_TRUE(source.lastIndexOf('x', 45).isNone());
    EXPECT_TRUE(source.lastIndexOf("awesome").isNone());
    EXPECT_TRUE(source.lastIndexOf(world, source.length() - 3).isNone());
    EXPECT_TRUE(StringView("hi, i,").lastIndexOf("i, long string").isNone());

    // Fail case:
    EXPECT_THROW(StringView("hi").lastIndexOf('i', 5),
                            Exception);
    EXPECT_THROW(StringView("hi").lastIndexOf("hi", 5),
                            Exception);
}

/**
    * @see StringView::contains
    */
TEST(TestStringView, testContains) {
    EXPECT_TRUE(StringView("Hello, world!").contains('e'));
    EXPECT_TRUE(StringView("Hello, world!").contains("world"));
    EXPECT_TRUE(!StringView("hi").contains('!'));
    EXPECT_TRUE(!StringView("hi").contains("hight"));
}

/**
    * @see StringView::hashCode
    */
TEST(TestStringView, testHashCode) {
    EXPECT_NE(StringView("Hello otu there").hashCode(), 0);

    EXPECT_TRUE(StringView("Hello otu there").hashCode() !=
                    StringView("Hello out there").hashCode());
}

TEST(TestStringView, testSplitByChar) {

    // Splitting empty string gives you 1 item in a collection - empty string
    EXPECT_EQ(static_cast<array_size_t>(1),
                            StringView()
                            .split('x')
                            .size());

    {   // Normal split
        auto result = StringView("boo:and:foo").split(':');

        EXPECT_EQ(static_cast<array_size_t>(3), result.size());
        EXPECT_EQ(StringView("boo"), result[0]);
        EXPECT_EQ(StringView("and"), result[1]);
        EXPECT_EQ(StringView("foo"), result[2]);
    }

    {   // Normal split 2
        auto result = StringView("warning,performance,portability,")
                .split(',');

        EXPECT_EQ(static_cast<array_size_t>(4), result.size());
        EXPECT_EQ(StringView("warning"), result[0]);
        EXPECT_EQ(StringView("performance"), result[1]);
        EXPECT_EQ(StringView("portability"), result[2]);
        EXPECT_EQ(StringView(), result[3]);
    }

    {   // Normal split with empty token in the middle
        auto result = StringView("boo::foo").split(':');

        EXPECT_EQ(static_cast<array_size_t>(3), result.size());
        EXPECT_EQ(StringView("boo"), result[0]);
        EXPECT_EQ(StringView(),      result[1]);
        EXPECT_EQ(StringView("foo"), result[2]);
    }

    {   // Normal split with empty token in the middle and in the end
        auto result = StringView("boo::foo:").split(':');

        EXPECT_EQ(static_cast<array_size_t>(4), result.size());
        EXPECT_EQ(StringView("boo"), result[0]);
        EXPECT_EQ(StringView(),      result[1]);
        EXPECT_EQ(StringView("foo"), result[2]);
        EXPECT_EQ(StringView(),      result[3]);
    }

    {   // No splitting token in the string
        auto result = StringView("boo").split(':');

        EXPECT_EQ(static_cast<array_size_t>(1), result.size());
        EXPECT_EQ(StringView("boo"), result[0]);
    }

}

TEST(TestStringView, testSplitByStringToken) {
    // Splitting empty string gives you 1 item in a collection - empty string
    EXPECT_EQ(static_cast<array_size_t>(1),
                            StringView()
                            .split("tok")
                            .size());

    {   // Narmal split
        auto result = StringView("boo:!and:!foo").split(":!");

        EXPECT_EQ(static_cast<array_size_t>(3), result.size());
        EXPECT_EQ(StringView("boo"), result[0]);
        EXPECT_EQ(StringView("and"), result[1]);
        EXPECT_EQ(StringView("foo"), result[2]);
    }
    {   // Narmal split
        auto result = StringView("boo:!and:!").split(":!");

        EXPECT_EQ(static_cast<array_size_t>(3), result.size());
        EXPECT_EQ(StringView("boo"), result[0]);
        EXPECT_EQ(StringView("and"), result[1]);
        EXPECT_EQ(StringView(), result[2]);
    }
    {   // Narmal split
        auto result = StringView("boo:!:!foo:!").split(":!");

        EXPECT_EQ(static_cast<array_size_t>(4), result.size());
        EXPECT_EQ(StringView("boo"), result[0]);
        EXPECT_EQ(StringView(),      result[1]);
        EXPECT_EQ(StringView("foo"), result[2]);
        EXPECT_EQ(StringView(),      result[3]);
    }

    {   // No splitting token in the string
        auto result = StringView("boo").split("other");

        EXPECT_EQ(static_cast<array_size_t>(1), result.size());
        EXPECT_EQ(StringView("boo"), result[0]);
    }
}
