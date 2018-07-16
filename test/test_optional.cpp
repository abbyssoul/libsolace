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
        CPPUNIT_TEST(testConstructionOf);
        CPPUNIT_TEST(testMoveConstructionOf);

        CPPUNIT_TEST(testConstructorTypeConvertion);
//        CPPUNIT_TEST(testAssignment);
        CPPUNIT_TEST(testSwap);
        CPPUNIT_TEST(testMoveAssignemnt);
        CPPUNIT_TEST(testEquals);

        CPPUNIT_TEST(testEmpty);
        CPPUNIT_TEST(testString);
        CPPUNIT_TEST(testGetRaises);
        CPPUNIT_TEST(testOrElse);
        CPPUNIT_TEST(testMap);
        CPPUNIT_TEST(testFlatMap);
        CPPUNIT_TEST(testFilter);
        CPPUNIT_TEST(testMoveOnlyResult);
        CPPUNIT_TEST(testMoveOnlyMapper);
        CPPUNIT_TEST(testMoveOnlyMove);

    CPPUNIT_TEST_SUITE_END();

private:

    Optional<String> moveOptionalString(String&& value) {
        return Optional<String>(value);
    }

    SimpleType moveSimpleType(int x, int y, int z) {
        return SimpleType{x, y, z};
    }

    MoveOnlyType moveMoveonlyType(int value) {
        return MoveOnlyType{value};
    }

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
                auto v2 = Optional<int>(x);
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
                auto v2 = Optional<uint64>(x);
                CPPUNIT_ASSERT(v2.isSome());

                CPPUNIT_ASSERT_EQUAL(x, v2.get());
            }
        }
    }

    void testConstructionOf() {
        SimpleType t(1, -32, 3);
        CPPUNIT_ASSERT(Optional<SimpleType>(t).isSome());
    }

    void testMoveConstructionOf() {
        CPPUNIT_ASSERT_EQUAL(0, MoveOnlyType::InstanceCount);

        const int randomNumber = 3987;
        auto v2 = Optional<MoveOnlyType>(moveMoveonlyType(randomNumber));
        CPPUNIT_ASSERT(v2.isSome());
        CPPUNIT_ASSERT_EQUAL(randomNumber, v2.get().x_);
        CPPUNIT_ASSERT_EQUAL(1, MoveOnlyType::InstanceCount);
    }

    void testConstruction() {
        {
            CPPUNIT_ASSERT(Optional<int>().isNone());
            CPPUNIT_ASSERT(Optional<SimpleType>().isNone());
            CPPUNIT_ASSERT(Optional<int>{none}.isNone());
        }

        {
            CPPUNIT_ASSERT_EQUAL(0, MoveOnlyType::InstanceCount);
            const Optional<MoveOnlyType> v;
            CPPUNIT_ASSERT(v.isNone());
            CPPUNIT_ASSERT_EQUAL(0, MoveOnlyType::InstanceCount);
        }
    }

    void testConstructorTypeConvertion() {
        CPPUNIT_ASSERT_EQUAL(0, PimitiveType::InstanceCount);

        Optional<PimitiveType> ptype = Optional<int>(321);

        CPPUNIT_ASSERT_EQUAL(1, PimitiveType::InstanceCount);
        CPPUNIT_ASSERT_EQUAL(321, ptype.get().x);
    }

    /*
    void testAssignment() {
        {
            auto v1 = Optional<SimpleType>{};
            auto v2 = Optional<SimpleType>({3, 2, -123});

            CPPUNIT_ASSERT(v1.isNone());
            CPPUNIT_ASSERT_EQUAL(1, SimpleType::InstanceCount);
            v1 = v2;
            CPPUNIT_ASSERT_EQUAL(2, SimpleType::InstanceCount);
            CPPUNIT_ASSERT(v1.isSome());
            v2 = Optional<SimpleType>{};
            CPPUNIT_ASSERT_EQUAL(1, SimpleType::InstanceCount);
            v1 = v2;
            CPPUNIT_ASSERT_EQUAL(0, SimpleType::InstanceCount);
        }

        CPPUNIT_ASSERT_EQUAL(0, SimpleType::InstanceCount);
        {
            auto v2 = Optional<SimpleType>(moveSimpleType(3, 2718, 321));
            Optional<SimpleType> v1 = None();

            CPPUNIT_ASSERT(v1.isNone());
            CPPUNIT_ASSERT_EQUAL(1, SimpleType::InstanceCount);
            v1 = v2;
            CPPUNIT_ASSERT_EQUAL(2, SimpleType::InstanceCount);
        }
    }
*/

    void testSwap() {
        {
            auto v1 = Optional<int>{};
            auto v2 = Optional<int>{};

            CPPUNIT_ASSERT(v1.isNone());
            swap(v1, v2);

            v2 = Optional<int>(3);
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
            auto v1 = Optional<SimpleType>{};
            auto v2 = Optional<SimpleType>{};

            CPPUNIT_ASSERT_EQUAL(0, SimpleType::InstanceCount);
            CPPUNIT_ASSERT(v1.isNone());
            swap(v1, v2);
            CPPUNIT_ASSERT(v1.isNone());
            CPPUNIT_ASSERT(v2.isNone());
            CPPUNIT_ASSERT_EQUAL(0, SimpleType::InstanceCount);

            v2 = Optional<SimpleType>(SimpleType{1, -3, 412});
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
            auto v1 = Optional<MoveOnlyType>{};
            auto v2 = Optional<MoveOnlyType>{};

            CPPUNIT_ASSERT_EQUAL(0, MoveOnlyType::InstanceCount);
            CPPUNIT_ASSERT(v1.isNone());
            swap(v1, v2);
            CPPUNIT_ASSERT(v1.isNone());
            CPPUNIT_ASSERT(v2.isNone());
            CPPUNIT_ASSERT_EQUAL(0, SimpleType::InstanceCount);

            v2 = Optional<MoveOnlyType>(MoveOnlyType{998});
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

    void testMoveAssignemnt() {
        String test("hello");

        Optional<String> v1;
        CPPUNIT_ASSERT(v1.isNone());

        v1 = moveOptionalString(std::move(test));
        CPPUNIT_ASSERT(v1.isSome());
        CPPUNIT_ASSERT_EQUAL(String("hello"), v1.get());

        String test2("something different");
        v1 = std::move(test2);

        CPPUNIT_ASSERT(v1.isSome());
        CPPUNIT_ASSERT_EQUAL(String("something different"), v1.get());
    }

    void testEquals() {

        String test("hello");
        const Optional<String> v1(moveOptionalString(std::move(test)));

        String alsoHello("hello");
        const Optional<String> v3(moveOptionalString(std::move(alsoHello)));

        CPPUNIT_ASSERT_EQUAL(v1, v3);
    }


    void testEmpty() {
        auto v1 = Optional<int>{};

        CPPUNIT_ASSERT(v1.isNone());
        CPPUNIT_ASSERT(!v1.isSome());

        {
            const auto v = Optional<SimpleType>{};
            CPPUNIT_ASSERT_EQUAL(0, SimpleType::InstanceCount);

            CPPUNIT_ASSERT(v.isNone());
            CPPUNIT_ASSERT(!v.isSome());
            CPPUNIT_ASSERT_THROW(v.get(), NoSuchElementException);
        }
        CPPUNIT_ASSERT_EQUAL(0, SimpleType::InstanceCount);

        {
            auto v = Optional<MoveOnlyType>{};
            CPPUNIT_ASSERT_EQUAL(0, MoveOnlyType::InstanceCount);

            CPPUNIT_ASSERT(v.isNone());
            CPPUNIT_ASSERT(!v.isSome());
            CPPUNIT_ASSERT_THROW(v.get(), NoSuchElementException);
        }
        CPPUNIT_ASSERT_EQUAL(0, MoveOnlyType::InstanceCount);
    }

    void testString() {
        auto const v1 = Optional<String>("hello");

        CPPUNIT_ASSERT(v1.isSome());
        CPPUNIT_ASSERT(!v1.isNone());

        CPPUNIT_ASSERT_EQUAL(String("hello"), v1.get());
    }

    void testGetRaises() {
        CPPUNIT_ASSERT_THROW(Optional<String>{}.get(), NoSuchElementException);
    }

    void testOrElse() {
        const SimpleType test(2, 0.0f, 2);
        const SimpleType testElse(321, -1, 5);

        auto v1 = Optional<SimpleType>(test);
        auto v2 = Optional<SimpleType>{};

        CPPUNIT_ASSERT(v1.isSome());
        CPPUNIT_ASSERT(v2.isNone());

        CPPUNIT_ASSERT_EQUAL(test, v1.orElse(testElse));
        CPPUNIT_ASSERT_EQUAL(testElse, v2.orElse(testElse));
    }

    static int my_atoi(const char* c) {
        return atoi(c);
    }

    void testMap() {
        SimpleType test(32, 24, -3212);

        auto f = [&test](const int& content) -> SimpleType {
            return SimpleType(test.x * content, test.y, test.z);
        };

        // Test mapping using lambda
        const auto v1 = Optional<int>(18)
                                        .map(f);
        CPPUNIT_ASSERT(v1.isSome());
        CPPUNIT_ASSERT_EQUAL(SimpleType(test.x * 18, test.y, test.z), v1.get());

        const auto v2 = Optional<SimpleType>{}
                                        .map([](SimpleType const& value) {
                                            return value.x;
                                        });

        CPPUNIT_ASSERT(v2.isNone());
        CPPUNIT_ASSERT_EQUAL(48, v2.orElse(48));

        // Test mapping using regular C-function
        const auto meaningOfLife = Optional<const char*>("42")
                                        .map(my_atoi);
        CPPUNIT_ASSERT(meaningOfLife.isSome());
        CPPUNIT_ASSERT_EQUAL(42, meaningOfLife.get());

        {
            auto v = Optional<MoveOnlyType>(MoveOnlyType(1024));
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
            return Optional<int>(content.x * 2);
        };

        const auto v1 = Optional<SimpleType>(test)
                .flatMap<int>(f);

        CPPUNIT_ASSERT(v1.isSome());
        CPPUNIT_ASSERT_EQUAL(test.x*2, v1.get());

        const auto v2 = Optional<SimpleType>{}
                .flatMap<int>(f);

        CPPUNIT_ASSERT(v2.isNone());
        CPPUNIT_ASSERT_EQUAL(42, v2.orElse(42));
    }

    void testFilter() {
        CPPUNIT_ASSERT(Optional<int>(4412)
                        .filter([](int t) { return t > 20;})
                        .isSome());

        CPPUNIT_ASSERT(Optional<int>{}
                        .filter([](int t) { return t > 0;})
                        .isNone());

        CPPUNIT_ASSERT(Optional<SimpleType>{}
                        .filter([](const SimpleType& t) { return t.x != 0;})
                        .isNone());

        CPPUNIT_ASSERT(Optional<SimpleType>({32, 72, -858})
                        .filter([](const SimpleType& t) { return t.x >= 0;})
                        .isSome());

        CPPUNIT_ASSERT(Optional<MoveOnlyType>(in_place, 32)
                        .filter([](const MoveOnlyType& t) { return t.x_ != 0;})
                        .isSome());
    }

    void testMoveOnlyResult() {
        CPPUNIT_ASSERT(Optional<MoveOnlyType>{}.isNone());

        CPPUNIT_ASSERT_EQUAL(0, MoveOnlyType::InstanceCount);
        auto mover = [] (int v) {
            return MoveOnlyType(v);
        };

        auto r = Optional<MoveOnlyType>(mover(321));

        CPPUNIT_ASSERT(r.isSome());
        CPPUNIT_ASSERT_EQUAL(321, r.get().x_);
        CPPUNIT_ASSERT_EQUAL(1, MoveOnlyType::InstanceCount);
    }

    void testMoveOnlyMapper() {
        auto r = Optional<MoveOnlyType>(MoveOnlyType(32));

        /* FIXME: This is broken as map() does not support functors that move value out.
        auto op = r.map([](MoveOnlyType&& m) {
                return m.x_ * 2;
        });

        CPPUNIT_ASSERT(op.isSome());
        CPPUNIT_ASSERT_EQUAL(64, op.get());
        */
        CPPUNIT_ASSERT_EQUAL(1, MoveOnlyType::InstanceCount);
    }


    void testMoveOnlyMove() {
        auto r = Optional<MoveOnlyType>(in_place, 732);

        auto p = [&r]() { return r.move(); } ();
        CPPUNIT_ASSERT_EQUAL(2, MoveOnlyType::InstanceCount);
        CPPUNIT_ASSERT_EQUAL(732, p.x_);
        CPPUNIT_ASSERT_EQUAL(732, r.get().x_);
        CPPUNIT_ASSERT_EQUAL(732, r.orElse(MoveOnlyType(-9876)).x_);
    }
};


CPPUNIT_TEST_SUITE_REGISTRATION(TestOptional);
