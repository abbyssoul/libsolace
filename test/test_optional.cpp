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
#include <solace/exception.hpp>
#include <cmath>

#include <cppunit/extensions/HelperMacros.h>
#include "mockTypes.hpp"


using namespace Solace;


class TestOptional : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestOptional);
        CPPUNIT_TEST(testConstructionIntegrals);
        CPPUNIT_TEST(testConstruction);
        CPPUNIT_TEST(testAssignment);
        CPPUNIT_TEST(testSwap);

        CPPUNIT_TEST(testEmpty);
        CPPUNIT_TEST(testString);
        CPPUNIT_TEST(testGetRaises);
        CPPUNIT_TEST(testOrElse);
        CPPUNIT_TEST(testMap);
        CPPUNIT_TEST(testFlatMap);
        CPPUNIT_TEST(testFilter);
        CPPUNIT_TEST(testMoveOnlyResult);
        CPPUNIT_TEST(testMoveOnlyMapper);

    CPPUNIT_TEST_SUITE_END();

private:
public:

    void setUp() override {
        CPPUNIT_ASSERT_EQUAL(0, PimitiveType::InstanceCount);
        CPPUNIT_ASSERT_EQUAL(0, SimpleType::InstanceCount);
        CPPUNIT_ASSERT_EQUAL(0, MoveOnlyType::InstanceCount);
    }

    void tearDown() override {
        CPPUNIT_ASSERT_EQUAL(0, PimitiveType::InstanceCount);
        CPPUNIT_ASSERT_EQUAL(0, SimpleType::InstanceCount);
        CPPUNIT_ASSERT_EQUAL(0, MoveOnlyType::InstanceCount);
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

    SimpleType moveSimpleType(int x, int y, int z) {
        return SimpleType{x, y, z};
    }

    void testConstruction() {
        {
            const Optional<SimpleType> v;
            CPPUNIT_ASSERT(v.isNone());

            SimpleType t(1, -32, 3);
            auto v2 = Optional<SimpleType>::of(t);
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

        {
            CPPUNIT_ASSERT_EQUAL(0, MoveOnlyType::InstanceCount);
            const Optional<MoveOnlyType> v;
            CPPUNIT_ASSERT(v.isNone());
            CPPUNIT_ASSERT_EQUAL(0, MoveOnlyType::InstanceCount);

            MoveOnlyType t(3987);
            auto v2 = Optional<MoveOnlyType>::of(std::move(t));
            CPPUNIT_ASSERT(v2.isSome());
            CPPUNIT_ASSERT_EQUAL(2, MoveOnlyType::InstanceCount);
        }

    }

    void testAssignment() {
        {
            auto v1 = Optional<SimpleType>::none();
            auto v2 = Optional<SimpleType>::of({3, 2, -123});

            CPPUNIT_ASSERT(v1.isNone());
            CPPUNIT_ASSERT_EQUAL(1, SimpleType::InstanceCount);
            v1 = v2;
            CPPUNIT_ASSERT_EQUAL(2, SimpleType::InstanceCount);
            CPPUNIT_ASSERT(v1.isSome());
            v2 = Optional<SimpleType>::none();
            CPPUNIT_ASSERT_EQUAL(1, SimpleType::InstanceCount);
            v1 = v2;
            CPPUNIT_ASSERT_EQUAL(0, SimpleType::InstanceCount);
        }

        CPPUNIT_ASSERT_EQUAL(0, SimpleType::InstanceCount);
        {
            auto v2 = Optional<SimpleType>::of(moveSimpleType(3, 2718, 321));
            Optional<SimpleType> v1 = None();

            CPPUNIT_ASSERT(v1.isNone());
            CPPUNIT_ASSERT_EQUAL(1, SimpleType::InstanceCount);
            v1 = v2;
            CPPUNIT_ASSERT_EQUAL(2, SimpleType::InstanceCount);
        }
    }


    void testSwap() {
        {
            auto v1 = Optional<int>::none();
            auto v2 = Optional<int>::none();

            CPPUNIT_ASSERT(v1.isNone());
            swap(v1, v2);

            v2 = Optional<int>::of(3);
            CPPUNIT_ASSERT(v1.isNone());
            CPPUNIT_ASSERT(v2.isSome());
            CPPUNIT_ASSERT_EQUAL(3, v2.get());

            swap(v1, v2);
            CPPUNIT_ASSERT(v1.isSome());
            CPPUNIT_ASSERT(v2.isNone());

            CPPUNIT_ASSERT_EQUAL(3, v1.get());
            CPPUNIT_ASSERT_THROW(v2.get(), Solace::NoSuchElementException);
        }
        {
            auto v1 = Optional<SimpleType>::none();
            auto v2 = Optional<SimpleType>::none();

            CPPUNIT_ASSERT_EQUAL(0, SimpleType::InstanceCount);
            CPPUNIT_ASSERT(v1.isNone());
            swap(v1, v2);
            CPPUNIT_ASSERT(v1.isNone());
            CPPUNIT_ASSERT(v2.isNone());
            CPPUNIT_ASSERT_EQUAL(0, SimpleType::InstanceCount);

            v2 = Optional<SimpleType>::of(SimpleType{1, -3, 412});
            CPPUNIT_ASSERT(v1.isNone());
            CPPUNIT_ASSERT(v2.isSome());
            CPPUNIT_ASSERT_EQUAL(1, SimpleType::InstanceCount);
            CPPUNIT_ASSERT_EQUAL(-3, v2.get().y);

            swap(v1, v2);
            CPPUNIT_ASSERT_EQUAL(1, SimpleType::InstanceCount);
            CPPUNIT_ASSERT(v1.isSome());
            CPPUNIT_ASSERT(v2.isNone());

            CPPUNIT_ASSERT_EQUAL(412, v1.get().z);
            CPPUNIT_ASSERT_THROW(v2.get(), NoSuchElementException);
        }

        {
            auto v1 = Optional<MoveOnlyType>::none();
            auto v2 = Optional<MoveOnlyType>::none();

            CPPUNIT_ASSERT_EQUAL(0, MoveOnlyType::InstanceCount);
            CPPUNIT_ASSERT(v1.isNone());
            swap(v1, v2);
            CPPUNIT_ASSERT(v1.isNone());
            CPPUNIT_ASSERT(v2.isNone());
            CPPUNIT_ASSERT_EQUAL(0, SimpleType::InstanceCount);

            v2 = Optional<MoveOnlyType>::of(MoveOnlyType{998});
            CPPUNIT_ASSERT(v1.isNone());
            CPPUNIT_ASSERT(v2.isSome());
            CPPUNIT_ASSERT_EQUAL(1, MoveOnlyType::InstanceCount);
            CPPUNIT_ASSERT_EQUAL(998, v2.get().x_);

            swap(v1, v2);
            CPPUNIT_ASSERT_EQUAL(1, MoveOnlyType::InstanceCount);
            CPPUNIT_ASSERT(v1.isSome());
            CPPUNIT_ASSERT(v2.isNone());

            CPPUNIT_ASSERT_EQUAL(998, v1.get().x_);
            CPPUNIT_ASSERT_THROW(v2.get(), NoSuchElementException);
        }
    }


    void testEmpty() {
        auto v1 = Optional<int>::none();

        CPPUNIT_ASSERT(v1.isNone());
        CPPUNIT_ASSERT(!v1.isSome());

        {
            const auto v = Optional<SimpleType>::none();
            CPPUNIT_ASSERT_EQUAL(0, SimpleType::InstanceCount);

            CPPUNIT_ASSERT(v.isNone());
            CPPUNIT_ASSERT(!v.isSome());
            CPPUNIT_ASSERT_THROW(v.get(), NoSuchElementException);
        }
        CPPUNIT_ASSERT_EQUAL(0, SimpleType::InstanceCount);

        {
            auto v = Optional<MoveOnlyType>::none();
            CPPUNIT_ASSERT_EQUAL(0, MoveOnlyType::InstanceCount);

            CPPUNIT_ASSERT(v.isNone());
            CPPUNIT_ASSERT(!v.isSome());
            CPPUNIT_ASSERT_THROW(v.get(), NoSuchElementException);
        }
        CPPUNIT_ASSERT_EQUAL(0, MoveOnlyType::InstanceCount);
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
        const SimpleType test(2, 0.0f, 2);
        const SimpleType testElse(321, -1, 5);

        auto v1 = Optional<SimpleType>::of(test);
        auto v2 = Optional<SimpleType>::none();

        CPPUNIT_ASSERT(v1.isSome());
        CPPUNIT_ASSERT(v2.isNone());

        CPPUNIT_ASSERT_EQUAL(test, v1.orElse(testElse));
        CPPUNIT_ASSERT_EQUAL(testElse, v2.orElse(testElse));
    }

    void testMap() {
        SimpleType test(32, 24, -3212);

        auto f = [&test](const int& content) -> SimpleType {
            return SimpleType(test.x * content, test.y, test.z);
        };

        // Test mapping using lambda
        const auto v1 = Optional<int>::of(18)
                                        .map(f);
        CPPUNIT_ASSERT(v1.isSome());
        CPPUNIT_ASSERT_EQUAL(SimpleType(test.x * 18, test.y, test.z), v1.get());

        const auto v2 = Optional<SimpleType>::none()
                                        .map([](const SimpleType& value) {
                                            return value.x;
                                        });

        CPPUNIT_ASSERT(v2.isNone());
        CPPUNIT_ASSERT_EQUAL(48, v2.orElse(48));

        // Test mapping using regular C-function
        const auto meaningOfLife = Optional<const char*>::of("42")
                                        .map(atoi);
        CPPUNIT_ASSERT(meaningOfLife.isSome());
        CPPUNIT_ASSERT_EQUAL(42, meaningOfLife.get());

        {
            auto v = Optional<MoveOnlyType>::of(MoveOnlyType(1024));
            const auto d = v.map([] (MoveOnlyType& q) {
                return q.x_ / 2;
            });

            CPPUNIT_ASSERT(v.isSome());
            CPPUNIT_ASSERT(d.isSome());
            CPPUNIT_ASSERT_EQUAL(1024, v.get().x_);
            CPPUNIT_ASSERT_EQUAL(512, d.get());
            CPPUNIT_ASSERT_EQUAL(1, MoveOnlyType::InstanceCount);
        }
    }

    void testFlatMap() {
        SimpleType test(32, 72, -312);

        auto f = [](const SimpleType& content) {
            return Optional<int>::of(content.x * 2);
        };

        const auto v1 = Optional<SimpleType>::of(test)
                .flatMap<int>(f);

        CPPUNIT_ASSERT(v1.isSome());
        CPPUNIT_ASSERT_EQUAL(test.x*2, v1.get());

        const auto v2 = Optional<SimpleType>::none()
                .flatMap<int>(f);

        CPPUNIT_ASSERT(v2.isNone());
        CPPUNIT_ASSERT_EQUAL(42, v2.orElse(42));
    }

    void testFilter() {
        CPPUNIT_ASSERT(Optional<SimpleType>::none()
                        .filter([](const SimpleType& t) { return t.x != 0;})
                        .isNone());

        CPPUNIT_ASSERT(Optional<int>::of(4412)
                        .filter([](int t) { return t > 20;})
                        .isSome());

        CPPUNIT_ASSERT(Optional<SimpleType>::of({32, 72, -858})
                        .filter([](const SimpleType& t) { return t.x != 0;})
                        .isSome());
    }

    void testMoveOnlyResult() {
        CPPUNIT_ASSERT(Optional<MoveOnlyType>::none().isNone());
        CPPUNIT_ASSERT_EQUAL(0, MoveOnlyType::InstanceCount);

        Optional<MoveOnlyType> r =  [] (int v) {
            return MoveOnlyType(v);
        } (321);

        CPPUNIT_ASSERT(r.isSome());
        CPPUNIT_ASSERT_EQUAL(321, r.get().x_);
        CPPUNIT_ASSERT_EQUAL(1, MoveOnlyType::InstanceCount);
    }

    void testMoveOnlyMapper() {
        auto r = Optional<MoveOnlyType>::of(MoveOnlyType(32));

        /* FIXME: This is broken as map() does not support functors that move value out.
        auto op = r.map([](MoveOnlyType&& m) {
                return m.x_ * 2;
        });

        CPPUNIT_ASSERT(op.isSome());
        CPPUNIT_ASSERT_EQUAL(64, op.get());
        */
        CPPUNIT_ASSERT_EQUAL(1, MoveOnlyType::InstanceCount);
    }
};


CPPUNIT_TEST_SUITE_REGISTRATION(TestOptional);
