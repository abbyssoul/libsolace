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

#include <cppunit/extensions/HelperMacros.h>


#include <cstring>

using namespace Solace;

class TestStringView : public CppUnit::TestFixture  {

CPPUNIT_TEST_SUITE(TestStringView);
        CPPUNIT_TEST(testConstructionFromNull);
        CPPUNIT_TEST(testConstructEmptyString);
        CPPUNIT_TEST(testConstructionSized);
        CPPUNIT_TEST(testConstructionUnsized);
        CPPUNIT_TEST(testAssignment);
        CPPUNIT_TEST(testAssignmentFromCstring);
        CPPUNIT_TEST(testEquality);
        CPPUNIT_TEST(testLength);
        CPPUNIT_TEST(testContains);
        CPPUNIT_TEST(testIndexOf);
        CPPUNIT_TEST(testLastIndexOf);
        CPPUNIT_TEST(testSubstring);
        CPPUNIT_TEST(testTrim);
        CPPUNIT_TEST(testStartsWith);
        CPPUNIT_TEST(testEndsWith);
        CPPUNIT_TEST(testHashCode);
        CPPUNIT_TEST(testSplitByChar);
        CPPUNIT_TEST(testSplitByStringToken);
    CPPUNIT_TEST_SUITE_END();

public:
    using array_size_t = Array<StringView>::size_type;

public:

    void testConstructionFromNull() {
        const char* nullCString = nullptr;

        CPPUNIT_ASSERT_THROW(StringView nullString(nullCString), Exception);
    }

    void testConstructEmptyString() {
        StringView value;
        CPPUNIT_ASSERT(value.empty());
        CPPUNIT_ASSERT_EQUAL(static_cast<StringView::size_type>(0), value.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<StringView::size_type>(0), value.length());

        CPPUNIT_ASSERT(StringView("").empty());
    }

    void testConstructionUnsized() {
        const char *cstr = "world";
        CPPUNIT_ASSERT(!StringView("Non-empty").empty());
        CPPUNIT_ASSERT_EQUAL(static_cast<StringView::size_type>(5), StringView(cstr).size());
        CPPUNIT_ASSERT_EQUAL(static_cast<StringView::size_type>(5), StringView(cstr).length());
        CPPUNIT_ASSERT_EQUAL(cstr, StringView(cstr).data());
    }

    void testConstructionSized() {
        const char *cstr = "world";

        CPPUNIT_ASSERT_EQUAL(static_cast<StringView::size_type>(3), StringView(cstr, 3).length());
        CPPUNIT_ASSERT_EQUAL(static_cast<StringView::size_type>(3), StringView(&cstr[2], 3).length());
        CPPUNIT_ASSERT_EQUAL(cstr, StringView(cstr, 3).data());
        CPPUNIT_ASSERT_EQUAL(cstr + 2, StringView(&cstr[2], 3).data());
    }

    void testAssignmentFromCstring() {
        StringView value("world");
        CPPUNIT_ASSERT_EQUAL(static_cast<StringView::size_type>(5), value.length());
        CPPUNIT_ASSERT_EQUAL(0, strcmp("world", value.data()));

        value = "Something else";
        CPPUNIT_ASSERT_EQUAL(static_cast<StringView::size_type>(14), value.size());
        CPPUNIT_ASSERT_EQUAL(0, strcmp("Something else", value.data()));
    }


    /**
     * @see StringView::operator=
     */
    void testAssignment() {
        StringView value("world");
        StringView other("Completely different value");

        value = other;

        CPPUNIT_ASSERT_EQUAL(static_cast<StringView::size_type>(26), value.length());
        CPPUNIT_ASSERT_EQUAL(0, strcmp("Completely different value", value.data()));
    }


    /**
     * @see StringView::equals
     * @see StringView::operator==
     */
    void testEquality() {
        StringView value1("hello");
        StringView value2("Completely different value");
        StringView value3("hello");

        CPPUNIT_ASSERT(value1.equals("hello"));
        CPPUNIT_ASSERT(!value1.equals("hello here"));
        CPPUNIT_ASSERT(!value1.equals("here hello"));

        CPPUNIT_ASSERT(value1.equals(value1));
        CPPUNIT_ASSERT(!value1.equals(value2));
        CPPUNIT_ASSERT(value1.equals(value3));
        CPPUNIT_ASSERT(value3.equals(value1));

        CPPUNIT_ASSERT(value1 == "hello");
        CPPUNIT_ASSERT(!(value1 == "hello here"));
        CPPUNIT_ASSERT(value1 != "hi hello");

        CPPUNIT_ASSERT(value1 == value1);
        CPPUNIT_ASSERT(!(value1 == value2));
        CPPUNIT_ASSERT(value1 != value2);
        CPPUNIT_ASSERT(value1 == value3);
        CPPUNIT_ASSERT(value3 == value1);
    }

    /**
     * @see StringView::length
     */
    void testLength() {
        // FIXME: Add utf9 example

        CPPUNIT_ASSERT_EQUAL(static_cast<StringView::size_type>(0), StringView().length());
        CPPUNIT_ASSERT_EQUAL(static_cast<StringView::size_type>(0), StringView("").length());
        CPPUNIT_ASSERT_EQUAL(static_cast<StringView::size_type>(5), StringView("world").length());
    }


    /**
     * @see StringView::startsWith
     */
    void testStartsWith() {
        CPPUNIT_ASSERT(StringView().startsWith('\0'));
        CPPUNIT_ASSERT(!StringView("Hello world").startsWith('\0'));
        CPPUNIT_ASSERT(StringView("Hello world").startsWith('H'));
        CPPUNIT_ASSERT(StringView("Hello world").startsWith("Hello"));
        CPPUNIT_ASSERT(!StringView("Hello world").startsWith("world"));
        CPPUNIT_ASSERT(!StringView("Some")
                       .startsWith("Some very long statement that can't possibly"));
    }

    /**
     * @see StringView::endsWith
     */
    void testEndsWith() {
        CPPUNIT_ASSERT(!StringView("Hello world!").endsWith('\0'));
        CPPUNIT_ASSERT(StringView("Hello world!").endsWith('!'));
        CPPUNIT_ASSERT(StringView("Hello world!").endsWith("world!"));
        CPPUNIT_ASSERT(!StringView("Hello world").endsWith("hello"));
        CPPUNIT_ASSERT(!StringView("Hello world")
                       .endsWith("Some very long statement that can't possibly"));
    }

    /**
     * @see StringView::substring
     */
    void testSubstring() {
        const StringView source("Hello, world! Good bye, World - and again!");
        const StringView bye("bye");
        const StringView andAgain("and again!");

        CPPUNIT_ASSERT_EQUAL(StringView(),
                             StringView().substring(0));

        CPPUNIT_ASSERT_EQUAL(StringView("world"),
                             source.substring(7, 5));

        CPPUNIT_ASSERT_EQUAL(bye,
                             source.substring(source.indexOf(bye).get(), bye.length()));

        CPPUNIT_ASSERT_EQUAL(andAgain,
                             source.substring(source.indexOf(andAgain).get()));

        CPPUNIT_ASSERT_THROW(StringView("hi").substring(13),
                             Exception);

        CPPUNIT_ASSERT_THROW(StringView("hi").substring(0, 8),
                             Exception);

        CPPUNIT_ASSERT_THROW(StringView("hi").substring(2, 1),
                             Exception);
    }

    /**
     * @see StringView::trim
     */
    void testTrim() {
        CPPUNIT_ASSERT(StringView().trim().empty());

        // Total trim
        CPPUNIT_ASSERT(StringView("   ").trim().empty());

        // Trim identity
        CPPUNIT_ASSERT(StringView("Hello, world!").trim().equals("Hello, world!"));

        // Trim start
        CPPUNIT_ASSERT(StringView("  Hello, world!").trim().equals("Hello, world!"));

        // Trim both
        CPPUNIT_ASSERT(StringView(" Hello, world!   ").trim().equals("Hello, world!"));

        // Trim End
        CPPUNIT_ASSERT(StringView("Hello, world!  ").trim().equals("Hello, world!"));
    }


    /**
     * @see StringView::indexOf
     */
    void testIndexOf() {
        const StringView src("Hello, world! $$Blarg");

        // Happy case:
        CPPUNIT_ASSERT_EQUAL('!', src[src.indexOf('!').get()]);
        CPPUNIT_ASSERT_EQUAL(static_cast<StringView::size_type>(12),
                             src.indexOf('!').get());
        CPPUNIT_ASSERT_EQUAL(static_cast<StringView::size_type>(7),
                             src.indexOf("world").get());
        CPPUNIT_ASSERT_EQUAL(static_cast<StringView::size_type>(3),
                             StringView("hello").indexOf("lo").get());


        CPPUNIT_ASSERT_EQUAL(static_cast<StringView::size_type>(14),
                             src.indexOf('$', 12).get());
        CPPUNIT_ASSERT_EQUAL(static_cast<StringView::size_type>(15),
                             src.indexOf("$Bl", 3).get());

        // Not found case:
        CPPUNIT_ASSERT(src.indexOf('!', 14).isNone());
        CPPUNIT_ASSERT(src.indexOf("awesome").isNone());
        CPPUNIT_ASSERT(src.indexOf("World", src.length() - 3).isNone());
        CPPUNIT_ASSERT(StringView("hi").indexOf("hi, long string").isNone());

        // Fail case:
        CPPUNIT_ASSERT_THROW(StringView("hi").indexOf('i', 5),
                             Exception);
        CPPUNIT_ASSERT_THROW(StringView("hi").indexOf("hi", 5),
                             Exception);
    }

    /**
     * @see StringView::lastIndexOf
     */
    void testLastIndexOf() {
        const StringView source("Hello, World! Good bye, World - and again rld!");
        const StringView world("World");

        CPPUNIT_ASSERT_EQUAL('!', source[source.lastIndexOf('!').get()]);
        CPPUNIT_ASSERT_EQUAL(static_cast<StringView::size_type>(45), source.lastIndexOf('!').get());
        CPPUNIT_ASSERT_EQUAL(static_cast<StringView::size_type>(24), source.lastIndexOf(world).get());
        CPPUNIT_ASSERT_EQUAL(static_cast<StringView::size_type>(24), source.lastIndexOf(world, 12).get());
        CPPUNIT_ASSERT_EQUAL(static_cast<StringView::size_type>(42), source.lastIndexOf("rld!").get());

        // Not found case:
        CPPUNIT_ASSERT(source.lastIndexOf('x').isNone());
        CPPUNIT_ASSERT(source.lastIndexOf('x', 45).isNone());
        CPPUNIT_ASSERT(source.lastIndexOf("awesome").isNone());
        CPPUNIT_ASSERT(source.lastIndexOf(world, source.length() - 3).isNone());
        CPPUNIT_ASSERT(StringView("hi, i,").lastIndexOf("i, long string").isNone());

        // Fail case:
        CPPUNIT_ASSERT_THROW(StringView("hi").lastIndexOf('i', 5),
                             Exception);
        CPPUNIT_ASSERT_THROW(StringView("hi").lastIndexOf("hi", 5),
                             Exception);
    }

    /**
     * @see StringView::contains
     */
    void testContains() {
        CPPUNIT_ASSERT(StringView("Hello, world!").contains('e'));
        CPPUNIT_ASSERT(StringView("Hello, world!").contains("world"));
        CPPUNIT_ASSERT(!StringView("hi").contains('!'));
        CPPUNIT_ASSERT(!StringView("hi").contains("hight"));
    }

    /**
     * @see StringView::hashCode
     */
    void testHashCode() {
        CPPUNIT_ASSERT(StringView("Hello otu there").hashCode() != 0);

        CPPUNIT_ASSERT(StringView("Hello otu there").hashCode() !=
                       StringView("Hello out there").hashCode());
    }

    void testSplitByChar() {

        // Splitting empty string gives you 1 item in a collection - empty string
        CPPUNIT_ASSERT_EQUAL(static_cast<array_size_t>(1),
                             StringView()
                             .split('x')
                             .size());

        {   // Normal split
            auto result = StringView("boo:and:foo").split(':');

            CPPUNIT_ASSERT_EQUAL(static_cast<array_size_t>(3), result.size());
            CPPUNIT_ASSERT_EQUAL(StringView("boo"), result[0]);
            CPPUNIT_ASSERT_EQUAL(StringView("and"), result[1]);
            CPPUNIT_ASSERT_EQUAL(StringView("foo"), result[2]);
        }

        {   // Normal split 2
            auto result = StringView("warning,performance,portability,")
                    .split(',');

            CPPUNIT_ASSERT_EQUAL(static_cast<array_size_t>(4), result.size());
            CPPUNIT_ASSERT_EQUAL(StringView("warning"), result[0]);
            CPPUNIT_ASSERT_EQUAL(StringView("performance"), result[1]);
            CPPUNIT_ASSERT_EQUAL(StringView("portability"), result[2]);
            CPPUNIT_ASSERT_EQUAL(StringView(), result[3]);
        }

        {   // Normal split with empty token in the middle
            auto result = StringView("boo::foo").split(':');

            CPPUNIT_ASSERT_EQUAL(static_cast<array_size_t>(3), result.size());
            CPPUNIT_ASSERT_EQUAL(StringView("boo"), result[0]);
            CPPUNIT_ASSERT_EQUAL(StringView(),      result[1]);
            CPPUNIT_ASSERT_EQUAL(StringView("foo"), result[2]);
        }

        {   // Normal split with empty token in the middle and in the end
            auto result = StringView("boo::foo:").split(':');

            CPPUNIT_ASSERT_EQUAL(static_cast<array_size_t>(4), result.size());
            CPPUNIT_ASSERT_EQUAL(StringView("boo"), result[0]);
            CPPUNIT_ASSERT_EQUAL(StringView(),      result[1]);
            CPPUNIT_ASSERT_EQUAL(StringView("foo"), result[2]);
            CPPUNIT_ASSERT_EQUAL(StringView(),      result[3]);
        }

        {   // No splitting token in the string
            auto result = StringView("boo").split(':');

            CPPUNIT_ASSERT_EQUAL(static_cast<array_size_t>(1), result.size());
            CPPUNIT_ASSERT_EQUAL(StringView("boo"), result[0]);
        }

    }

    void testSplitByStringToken() {
        // Splitting empty string gives you 1 item in a collection - empty string
        CPPUNIT_ASSERT_EQUAL(static_cast<array_size_t>(1),
                             StringView()
                             .split("tok")
                             .size());

        {   // Narmal split
            auto result = StringView("boo:!and:!foo").split(":!");

            CPPUNIT_ASSERT_EQUAL(static_cast<array_size_t>(3), result.size());
            CPPUNIT_ASSERT_EQUAL(StringView("boo"), result[0]);
            CPPUNIT_ASSERT_EQUAL(StringView("and"), result[1]);
            CPPUNIT_ASSERT_EQUAL(StringView("foo"), result[2]);
        }
        {   // Narmal split
            auto result = StringView("boo:!and:!").split(":!");

            CPPUNIT_ASSERT_EQUAL(static_cast<array_size_t>(3), result.size());
            CPPUNIT_ASSERT_EQUAL(StringView("boo"), result[0]);
            CPPUNIT_ASSERT_EQUAL(StringView("and"), result[1]);
            CPPUNIT_ASSERT_EQUAL(StringView(), result[2]);
        }
        {   // Narmal split
            auto result = StringView("boo:!:!foo:!").split(":!");

            CPPUNIT_ASSERT_EQUAL(static_cast<array_size_t>(4), result.size());
            CPPUNIT_ASSERT_EQUAL(StringView("boo"), result[0]);
            CPPUNIT_ASSERT_EQUAL(StringView(),      result[1]);
            CPPUNIT_ASSERT_EQUAL(StringView("foo"), result[2]);
            CPPUNIT_ASSERT_EQUAL(StringView(),      result[3]);
        }

        {   // No splitting token in the string
            auto result = StringView("boo").split("other");

            CPPUNIT_ASSERT_EQUAL(static_cast<array_size_t>(1), result.size());
            CPPUNIT_ASSERT_EQUAL(StringView("boo"), result[0]);
        }
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestStringView);
