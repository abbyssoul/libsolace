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
 * @file: test/test_result.cpp
 * @author: soultaker
 *
 * Created on: 14 Aug 2016
*******************************************************************************/
#include <solace/result.hpp>			// Class being tested

#include <solace/unit.hpp>
#include <solace/string.hpp>
#include <solace/exception.hpp>
#include <cmath>

#include <cppunit/extensions/HelperMacros.h>


using namespace Solace;


class TestResult : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestResult);
        CPPUNIT_TEST(testConstructionIntegrals);
        CPPUNIT_TEST(testConstruction);
        CPPUNIT_TEST(testMoveAssignment);

        CPPUNIT_TEST(getResultOnErrorThrows);
        CPPUNIT_TEST(getErrorOnOkThrows);

        CPPUNIT_TEST(testThen);
        CPPUNIT_TEST(testThenChaining);
        CPPUNIT_TEST(testTypeConvertion);

    CPPUNIT_TEST_SUITE_END();

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
            ++InstanceCount;
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


    class SimpleType {
    public:
        SimpleType(int x): x_(x) {}

        int x_;
    };

public:


    void setUp() override {
        CPPUNIT_ASSERT_EQUAL(0, SomeTestType::InstanceCount);
    }

    void tearDown() override {
        CPPUNIT_ASSERT_EQUAL(0, SomeTestType::InstanceCount);
    }

    void testTypeConvertion() {
        {
            const Result<int, Unit> r = Ok(10);
            CPPUNIT_ASSERT(r.isOk());
        }

        {
            const Result<SimpleType, Unit> r = Ok<SimpleType>(10);
            CPPUNIT_ASSERT(r.isOk());
        }

        {
            const Result<SimpleType, Unit> r = Result<int, Unit>(Ok(10));
            CPPUNIT_ASSERT(r.isOk());
        }
    }

    void testConstructionIntegrals() {
        {
            Unit x;
            const Result<Unit, int> v = Ok(std::move(x));
            CPPUNIT_ASSERT(v.isOk());
        }
        {
            int x = 675;
            const Result<Unit, int> v = Err(x);
            CPPUNIT_ASSERT(v.isError());
            CPPUNIT_ASSERT_EQUAL(x, v.getError());
        }

        {
            int x = 8832;
            const Result<int, Unit> v = Ok(std::move(x));
            CPPUNIT_ASSERT(v.isOk());
            CPPUNIT_ASSERT_EQUAL(x, v.getResult());
        }

        {
            char x = 'x';
            const Result<int, char> v = Err(x);
            CPPUNIT_ASSERT(v.isError());
            CPPUNIT_ASSERT_EQUAL(x, v.getError());
        }
    }


    void testConstruction() {
        {  // Unit result
            {
                const auto& v = []() -> Result<Unit, int> {
                    return Ok(Unit());
                } ();

                CPPUNIT_ASSERT(v.isOk());
            }

            {
                const auto& v = []() -> Result<Unit, int> {
                    return Err(-1);
                } ();

                CPPUNIT_ASSERT(v.isError());
            }
        }

        {  // Integral result
            {
                int x = 321;
                const auto& v = [](int y) -> Result<int, float> {

                    return Ok(std::move(y));
                } (x);

                CPPUNIT_ASSERT(v.isOk());
                CPPUNIT_ASSERT_EQUAL(x, v.getResult());
            }

            {
                const char x = 'x';
                const auto& v = [](char t) -> Result<int, char> {
                    return Err(std::move(t));
                } (x);

                CPPUNIT_ASSERT(v.isError());
                CPPUNIT_ASSERT_EQUAL(x, v.getError());
            }
        }

        CPPUNIT_ASSERT_EQUAL(0, SomeTestType::InstanceCount);
        {  // non-Pod result
            {
                const auto& v = []() -> Result<SomeTestType, int> {
                    auto r = SomeTestType{321, 3.1415f, "Somethere"};

                    return Ok(std::move(r));
                } ();

                CPPUNIT_ASSERT(v.isOk());
                CPPUNIT_ASSERT_EQUAL(321, v.getResult().x);
                CPPUNIT_ASSERT_EQUAL(1, SomeTestType::InstanceCount);
            }

            CPPUNIT_ASSERT_EQUAL(0, SomeTestType::InstanceCount);
            {
                const auto& v = []() -> Result<SomeTestType, int> {
                    return Err(-998);
                } ();

                CPPUNIT_ASSERT(v.isError());
                CPPUNIT_ASSERT_EQUAL(-998, v.getError());
                CPPUNIT_ASSERT_EQUAL(0, SomeTestType::InstanceCount);
            }
        }
    }

    void testMoveAssignment() {
        {
            Result<SomeTestType, int> v1 = Err(321);
            Result<SomeTestType, int> v2 = Ok<SomeTestType>({3, 2.718f, "Test value"});

            CPPUNIT_ASSERT(v1.isError());
            CPPUNIT_ASSERT(v2.isOk());
            CPPUNIT_ASSERT_EQUAL(3, v2.getResult().x);
            CPPUNIT_ASSERT_EQUAL(1, SomeTestType::InstanceCount);

            v1 = std::move(v2);
            CPPUNIT_ASSERT_EQUAL(1, SomeTestType::InstanceCount);
            CPPUNIT_ASSERT(v1.isOk());
            CPPUNIT_ASSERT(v2.isError());
            CPPUNIT_ASSERT_EQUAL(3, v1.getResult().x);

//            v1 = v2;
//            CPPUNIT_ASSERT_EQUAL(0, SomeTestType::InstanceCount);
//            CPPUNIT_ASSERT(v1.isError());
//            CPPUNIT_ASSERT(v2.isError());
        }
    }


    void getResultOnErrorThrows() {
        const Result<int, char> v = Err('e');

        CPPUNIT_ASSERT_THROW(v.getResult(), Exception);
    }

    void getErrorOnOkThrows() {
        const Result<int, char> v = Ok(32);

        CPPUNIT_ASSERT_THROW(v.getError(), Exception);
    }


    void testThen() {
        auto f = [](bool isOk) -> Result<int, float> {
            return isOk
                    ? Result<int, float>(Ok(42))
                    : Result<int, float>(Err(240.f));
        };

        {  // Test that success handler is called on success
            int thenValue = 0;
            int cValue = f(true).then<int>(
                        [&thenValue](const int& value) {
                            thenValue = value;

                            return 998;
                        },
                        [&thenValue](const int& errCode) {
                            thenValue = errCode;

                            return -776;
                        });

            // Make sure that success handler was called
            CPPUNIT_ASSERT_EQUAL(998, cValue);
            CPPUNIT_ASSERT_EQUAL(42, thenValue);
        }

        {  // Make sure that errback is called on failure
            int thenValue = 0;
            int cValue = f(false)
                    .then<int>(
                        [&thenValue](int value) {
                            thenValue = value;

                            return 998;
                        },
                        [&thenValue](int errCode) {
                            thenValue = errCode;
                            return -776;
                        });

            // Make sure that errback handler was called
            CPPUNIT_ASSERT_EQUAL(-776, cValue);
            CPPUNIT_ASSERT_EQUAL(240, thenValue);
        }
    }


    void testThenChaining() {

        // Good chain
        Result<int, SimpleType> goodResult = Ok<int>(42);

        auto alsoGood = goodResult.then([](int r) { return r / 2; });
        CPPUNIT_ASSERT(alsoGood.isOk());
        CPPUNIT_ASSERT_EQUAL(42/2, alsoGood.getResult());

        auto lessGood = alsoGood.then([](int r) { return r - 2; });
        CPPUNIT_ASSERT(lessGood.isOk());
        CPPUNIT_ASSERT_EQUAL(42/2 - 2, lessGood.getResult());


        // Error chain
        Result<int, SimpleType> badResult = Err<SimpleType>(18);

        auto alsoNotGood = badResult.then([](int r) { return r / 2; });
        CPPUNIT_ASSERT(alsoNotGood.isError());

        auto stillNotGood = alsoNotGood.then([](int r) { return r + 21; });
        CPPUNIT_ASSERT(stillNotGood.isError());

        auto recovered = stillNotGood.orElse([](const SimpleType& x) { return x.x_ + 2; });

        CPPUNIT_ASSERT(recovered.isOk());
        CPPUNIT_ASSERT_EQUAL(20, recovered.getResult());
    }

};


std::ostream& operator<<(std::ostream& ostr, const TestResult::SomeTestType& t) {
    return ostr << "SomeTestType(" << t.x << ", " << t.f << ", \"" << t.somethingElse << "\"";
}

int TestResult::SomeTestType::InstanceCount = 0;

CPPUNIT_TEST_SUITE_REGISTRATION(TestResult);

