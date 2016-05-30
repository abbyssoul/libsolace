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
 * @file: test/test_optional.cpp
 * @author: soultaker
 *
 * Created on: 29 Feb 2016
*******************************************************************************/
#include <solace/optional.hpp>			// Class being tested
#include <solace/string.hpp>

#include <cppunit/extensions/HelperMacros.h>


using namespace Solace;


class TestOptional : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestOptional);
        CPPUNIT_TEST(testConstructionIntegrals);
        CPPUNIT_TEST(testConstruction);
        CPPUNIT_TEST(testAssignment);

        CPPUNIT_TEST(testEmpty);
        CPPUNIT_TEST(testString);
        CPPUNIT_TEST(testGetRaises);
        CPPUNIT_TEST(testOrElse);
        CPPUNIT_TEST(testMap);
        CPPUNIT_TEST(testFlatMap);
    CPPUNIT_TEST_SUITE_END();

private:
public:

    class SomeTestType {
    public:
        static int InstanceCount;

        int x;
        float f;

        const char* somethingElse;

        SomeTestType(): x(), f(), somethingElse("THIS IS ERROR") {
            ++InstanceCount;
        }

        SomeTestType(int a, float b, const char* c) : x(a), f(b), somethingElse(c)
        {
            ++InstanceCount;
        }

        SomeTestType(const SomeTestType& t): x(t.x), f(t.f), somethingElse(t.somethingElse)
        {
            ++InstanceCount;
        }

        SomeTestType(SomeTestType&& t): x(t.x), f(t.f), somethingElse(t.somethingElse)
        {

        }

        ~SomeTestType()
        {
            --InstanceCount;
        }

        bool operator== (const SomeTestType& rhs) const {
            return  x == rhs.x &&
                    std::abs(f - rhs.f) < 1e-4f &&
                    somethingElse == rhs.somethingElse;
        }
    };

    void setUp() override {
        CPPUNIT_ASSERT_EQUAL(0, SomeTestType::InstanceCount);
    }

    void tearDown() override {
        CPPUNIT_ASSERT_EQUAL(0, SomeTestType::InstanceCount);
    }

    void testConstructionIntegrals() {

        {
            {
                const Optional<int> v;
                CPPUNIT_ASSERT(v.isNone());
            }

            {
                const int x = 32;
                auto v2 = Optional<int>::of(x);
                CPPUNIT_ASSERT(v2.isSome());

                CPPUNIT_ASSERT_EQUAL(x, v2.get());
            }
        }

        {
            {
                const Optional<uint64> v;
                CPPUNIT_ASSERT(v.isNone());
            }

            {
                uint64 x = 32;
                auto v2 = Optional<uint64>::of(x);
                CPPUNIT_ASSERT(v2.isSome());

                CPPUNIT_ASSERT_EQUAL(x, v2.get());
            }
        }
    }

    Optional<String> moveOptionalString(const String& str) {
        return Optional<String>::of(str);
    }

    SomeTestType moveSomeTestType(int x, float y, const char* z) {
        return SomeTestType{x, y, z};
    }

    void testConstruction() {
        {
            const Optional<SomeTestType> v;
            CPPUNIT_ASSERT(v.isNone());

            SomeTestType t(1, 32.1f, "Xuy");
            auto v2 = Optional<SomeTestType>::of(t);
            CPPUNIT_ASSERT(v2.isSome());
        }

        {
            const String test("hello");

            const Optional<String> v0;
            CPPUNIT_ASSERT(v0.isNone());

            Optional<String> v1("hello");
            CPPUNIT_ASSERT(v1.isSome());

            v1 = v0;
            CPPUNIT_ASSERT(v0.isNone());
            CPPUNIT_ASSERT_EQUAL(v1, v0);

            v1 = moveOptionalString(test);
            CPPUNIT_ASSERT(v1.isSome());
            CPPUNIT_ASSERT_EQUAL(test, v1.get());

            const Optional<String> v3(moveOptionalString(test));
            CPPUNIT_ASSERT(v3.isSome());
            CPPUNIT_ASSERT_EQUAL(test, v3.get());

            CPPUNIT_ASSERT_EQUAL(v1, v3);
        }
    }

    void testAssignment() {
        {
            auto v1 = Optional<SomeTestType>::none();
            auto v2 = Optional<SomeTestType>::of(SomeTestType(3, 2.718f, "Test value"));

            CPPUNIT_ASSERT_EQUAL(1, SomeTestType::InstanceCount);
            v1 = v2;
            CPPUNIT_ASSERT_EQUAL(2, SomeTestType::InstanceCount);
            v2 = Optional<SomeTestType>::none();
            CPPUNIT_ASSERT_EQUAL(1, SomeTestType::InstanceCount);
            v1 = v2;
            CPPUNIT_ASSERT_EQUAL(0, SomeTestType::InstanceCount);
        }

        CPPUNIT_ASSERT_EQUAL(0, SomeTestType::InstanceCount);
        {
            auto v2 = Optional<SomeTestType>::of(moveSomeTestType(3, 2.718f, "Test value"));
            Optional<SomeTestType> v1 = None();

            CPPUNIT_ASSERT_EQUAL(1, SomeTestType::InstanceCount);
            v1 = v2;
            CPPUNIT_ASSERT_EQUAL(2, SomeTestType::InstanceCount);
        }
        CPPUNIT_ASSERT_EQUAL(0, SomeTestType::InstanceCount);
    }

    /**
     * Test implementation and contract of IComparable
     */
    void testEmpty() {
        auto v1 = Optional<int>::none();

        CPPUNIT_ASSERT(v1.isNone());
        CPPUNIT_ASSERT(!v1.isSome());

        auto v2 = Optional<SomeTestType>::none();

        CPPUNIT_ASSERT(v2.isNone());
        CPPUNIT_ASSERT(!v2.isSome());
    }

    void testString() {
        const String test("hello");

        Optional<String> v1 = Optional<String>::of("hello");

        CPPUNIT_ASSERT(v1.isSome());
        CPPUNIT_ASSERT(!v1.isNone());

        CPPUNIT_ASSERT_EQUAL(test, v1.get());
    }

    void testGetRaises() {
        Optional<String> v1 = Optional<String>::none();

        CPPUNIT_ASSERT_THROW(v1.get(), NoSuchElementException);
    }

    void testOrElse() {
        const SomeTestType test(2, 0.0f, "hello");
        const SomeTestType testElse(321, -1.0f, "world");

        auto v1 = Optional<SomeTestType>::of(test);
        auto v2 = Optional<SomeTestType>::none();

        CPPUNIT_ASSERT(v1.isSome());
        CPPUNIT_ASSERT(v2.isNone());

        CPPUNIT_ASSERT_EQUAL(test, v1.orElse(testElse));
        CPPUNIT_ASSERT_EQUAL(testElse, v2.orElse(testElse));
    }

    void testMap() {
        SomeTestType test(32, 2.4, "Test string");

        auto f = [&test](const int& content) -> SomeTestType {
            return SomeTestType(test.x * content, test.f, test.somethingElse);
        };

        const auto v1 = Optional<int>::of(18)
                                        .map<SomeTestType>(f);
        CPPUNIT_ASSERT(v1.isSome());
        CPPUNIT_ASSERT_EQUAL(SomeTestType(test.x * 18, test.f, test.somethingElse), v1.get());

        const auto v2 = Optional<SomeTestType>::none()
                                        .map<int>([](const SomeTestType& value) {
                                            return value.x;
                                        });

        CPPUNIT_ASSERT(v2.isNone());
        CPPUNIT_ASSERT_EQUAL(48, v2.orElse(48));
    }

    void testFlatMap() {
        SomeTestType test(32, 2.4, "Test string");

        auto f = [](const SomeTestType& content) {
            return Optional<int>::of(content.x * 2);
        };

        const auto v1 = Optional<SomeTestType>::of(test)
                .flatMap<int>(f);

        CPPUNIT_ASSERT(v1.isSome());
        CPPUNIT_ASSERT_EQUAL(test.x*2, v1.get());

        const auto v2 = Optional<SomeTestType>::none()
                .flatMap<int>(f);

        CPPUNIT_ASSERT(v2.isNone());
        CPPUNIT_ASSERT_EQUAL(42, v2.orElse(42));
    }

};


std::ostream& operator<<(std::ostream& ostr, const TestOptional::SomeTestType& t) {
    return ostr << "SomeTestType(" << t.x << ", " << t.f << ", \"" << t.somethingElse << "\"";
}

int TestOptional::SomeTestType::InstanceCount = 0;

CPPUNIT_TEST_SUITE_REGISTRATION(TestOptional);
