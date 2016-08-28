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

public:


    void setUp() override {
        CPPUNIT_ASSERT_EQUAL(0, SomeTestType::InstanceCount);
    }

    void tearDown() override {
        CPPUNIT_ASSERT_EQUAL(0, SomeTestType::InstanceCount);
    }


    void testConstructionIntegrals() {
        {
            Unit x;
            const auto& v = Ok<Unit, int>(std::move(x));
            CPPUNIT_ASSERT(v.isOk());
        }
        {
            int x = 675;
            const auto& v = Err<Unit, int>(std::move(x));
            CPPUNIT_ASSERT(v.isError());
            CPPUNIT_ASSERT_EQUAL(x, v.getError());
        }

        {
            int x = 8832;
            const auto& v = Ok<int, int>(std::move(x));
            CPPUNIT_ASSERT(v.isOk());
            CPPUNIT_ASSERT_EQUAL(x, v.getResult());
        }

        {
            int x = 543;
            const auto& v = Err<int, int>(std::move(x));
            CPPUNIT_ASSERT(v.isError());
            CPPUNIT_ASSERT_EQUAL(x, v.getError());
        }
    }


    void testConstruction() {
        {  // Unit result
            {
                const auto& v = []() -> Result<Unit, int> {
                    return Ok<Unit, int>(Unit());
                } ();

                CPPUNIT_ASSERT(v.isOk());
            }

            {
                const auto& v = []() -> Result<Unit, int> {
                    return Err<Unit, int>(-1);
                } ();

                CPPUNIT_ASSERT(v.isError());
            }
        }

        {  // Integral result
            {
                int x = 321;
                const auto& v = [](int y) -> Result<int, int> {

                    return Ok<int, int>(std::move(y));
                } (x);

                CPPUNIT_ASSERT(v.isOk());
                CPPUNIT_ASSERT_EQUAL(x, v.getResult());
            }

            {
                const int x = 6431;
                const auto& v = [](int t) -> Result<int, int> {
                    return Err<int, int>(std::move(t));
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
                    return Ok<SomeTestType, int>(std::move(r));
                } ();

                CPPUNIT_ASSERT(v.isOk());
                CPPUNIT_ASSERT_EQUAL(321, v.getResult().x);
                CPPUNIT_ASSERT_EQUAL(1, SomeTestType::InstanceCount);
            }

            CPPUNIT_ASSERT_EQUAL(0, SomeTestType::InstanceCount);
            {
                const auto& v = []() -> Result<SomeTestType, int> {
                    return Err<SomeTestType, int>(-998);
                } ();

                CPPUNIT_ASSERT(v.isError());
                CPPUNIT_ASSERT_EQUAL(-998, v.getError());
                CPPUNIT_ASSERT_EQUAL(0, SomeTestType::InstanceCount);
            }
        }
    }

    void testMoveAssignment() {
        {
            auto v1 = Err<SomeTestType, int>(321);
            auto v2 = Ok<SomeTestType, int>({3, 2.718f, "Test value"});

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
        const auto v = Err<int, int>(32);

        CPPUNIT_ASSERT_THROW(v.getResult(), Exception);
    }

    void getErrorOnOkThrows() {
        const auto v = Ok<int, int>(32);

        CPPUNIT_ASSERT_THROW(v.getError(), Exception);
    }


    void testThen() {
        auto f = [](bool isOk) -> Result<int, int> {
            return isOk
                    ? Ok<int, int>(42)
                    : Err<int, int>(24);
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
            CPPUNIT_ASSERT_EQUAL(24, thenValue);
        }

    }

};


std::ostream& operator<<(std::ostream& ostr, const TestResult::SomeTestType& t) {
    return ostr << "SomeTestType(" << t.x << ", " << t.f << ", \"" << t.somethingElse << "\"";
}

int TestResult::SomeTestType::InstanceCount = 0;

CPPUNIT_TEST_SUITE_REGISTRATION(TestResult);

