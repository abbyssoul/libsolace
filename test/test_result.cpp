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
 * libSolace Unit Test Suit for Result<>
 * @file: test/test_result.cpp
 * @author: soultaker
 *
 * Created on: 14 Aug 2016
*******************************************************************************/
#include <solace/result.hpp>			// Class being tested

#include <solace/unit.hpp>
#include <solace/error.hpp>
#include <solace/string.hpp>
#include <solace/exception.hpp>
#include <cmath>

#include <cppunit/extensions/HelperMacros.h>

#include "mockTypes.hpp"

#include <fmt/format.h>

using namespace Solace;


class TestResult : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestResult);
        CPPUNIT_TEST(testFailure);
        CPPUNIT_TEST(testConstructionIntegrals);
        CPPUNIT_TEST(testConstruction);
        CPPUNIT_TEST(testMoveAssignment);

        CPPUNIT_TEST(getResultOnErrorThrows);
        CPPUNIT_TEST(getErrorOnOkThrows);

        CPPUNIT_TEST(testVoidResult);
        CPPUNIT_TEST(testThen);
        CPPUNIT_TEST(testThenChaining);
        CPPUNIT_TEST(testThenComposition);
        CPPUNIT_TEST(testThenComposition_cv);
        CPPUNIT_TEST(testTypeConvertion);
        CPPUNIT_TEST(testMapError);
        CPPUNIT_TEST(testMoveOnlyObjects);
        CPPUNIT_TEST(testThenMovesObjects);

    CPPUNIT_TEST_SUITE_END();

public:

    class SomeTestType {
    public:
        static int InstanceCount;

        int x;
        float f;

        const char* somethingElse;

        ~SomeTestType() {
            --InstanceCount;
        }

        SomeTestType(): x(), f(), somethingElse("THIS IS AN ERROR") {
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

        SomeTestType& operator= (const SomeTestType& rhs) {
            x = rhs.x;
            f = rhs.f;
            somethingElse = rhs.somethingElse;

            return (*this);
        }

        SomeTestType& operator= (SomeTestType&& rhs) {
            std::swap(x, rhs.x);
            std::swap(f, rhs.f);
            std::swap(somethingElse, rhs.somethingElse);

            return (*this);
        }

        bool operator== (const SomeTestType& rhs) const {
            return ((x == rhs.x) &&
                    (std::abs(f - rhs.f) < 1e-4f) &&
                    (somethingElse == rhs.somethingElse));
        }
    };


public:


    void setUp() override {
        CPPUNIT_ASSERT_EQUAL(0, SimpleType::InstanceCount);
        CPPUNIT_ASSERT_EQUAL(0, SomeTestType::InstanceCount);
        CPPUNIT_ASSERT_EQUAL(0, MoveOnlyType::InstanceCount);
    }

    void tearDown() override {
        CPPUNIT_ASSERT_EQUAL(0, SimpleType::InstanceCount);
        CPPUNIT_ASSERT_EQUAL(0, SomeTestType::InstanceCount);
        CPPUNIT_ASSERT_EQUAL(0, MoveOnlyType::InstanceCount);
    }


    Result<void, Error>
    fail(std::string&& message) {
        return Err(Error(std::move(message)));
    }

    void testFailure() {
        {
            Result<void, Error> r = fail(fmt::format("Bad errors '{}' about to happen", 3221));

            CPPUNIT_ASSERT(!r.isOk());
            CPPUNIT_ASSERT(r.isError());
        }

        bool value = false;
        {
            Result<void, Error> other = fail(fmt::format("Maybe no errors '{}' here", 9922));
            other.orElse([&value](Error&&){
                value = true;
            });

            CPPUNIT_ASSERT(other.isError());
            CPPUNIT_ASSERT(value);
        }
    }


    void testTypeConvertion() {
        {
            Result<int, Unit> r = Ok(10);
            CPPUNIT_ASSERT(r.isOk());
        }

        {
            Result<PimitiveType, Unit> r = Ok<PimitiveType>(10);
            CPPUNIT_ASSERT(r.isOk());
        }

        {
            Result<PimitiveType, Unit> r = Result<int, Unit>(Ok(10));
            CPPUNIT_ASSERT(r.isOk());
        }
    }

    void testConstructionIntegrals() {
        {
            Result<void, int> v = Ok();
            CPPUNIT_ASSERT(v.isOk());
        }
        {
            Unit x;
            Result<Unit, int> v = Ok(std::move(x));
            CPPUNIT_ASSERT(v.isOk());
        }
        {
            int x = 675;
            Result<Unit, int> v = Err(x);
            CPPUNIT_ASSERT(v.isError());
            CPPUNIT_ASSERT_EQUAL(x, v.getError());
        }

        {
            int x = 8832;
            Result<int, Unit> v = Ok(std::move(x));
            CPPUNIT_ASSERT(v.isOk());
            CPPUNIT_ASSERT_EQUAL(x, v.unwrap());
        }

        {
            char x = 'x';
            Result<int, char> v = Err(x);
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
                CPPUNIT_ASSERT_EQUAL(x, v.unwrap());
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
                CPPUNIT_ASSERT_EQUAL(321, v.unwrap().x);
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
            CPPUNIT_ASSERT_EQUAL(3, v2.unwrap().x);
            CPPUNIT_ASSERT_EQUAL(1, SomeTestType::InstanceCount);

            v1 = std::move(v2);
            CPPUNIT_ASSERT_EQUAL(1, SomeTestType::InstanceCount);
            CPPUNIT_ASSERT(v1.isOk());
            CPPUNIT_ASSERT(v2.isError());
            CPPUNIT_ASSERT_EQUAL(3, v1.unwrap().x);

//            v1 = v2;
//            CPPUNIT_ASSERT_EQUAL(0, SomeTestType::InstanceCount);
//            CPPUNIT_ASSERT(v1.isError());
//            CPPUNIT_ASSERT(v2.isError());
        }

        // Check preconditions
        CPPUNIT_ASSERT_EQUAL(0, SimpleType::InstanceCount);
        CPPUNIT_ASSERT_EQUAL(0, SomeTestType::InstanceCount);
        {
            auto mover = [] (bool isOk) -> Result<PimitiveType, SomeTestType> {
                if (isOk)
                    return Ok<PimitiveType>({321});
                else
                    return Err<SomeTestType>({ 3, 2.3f, "Bad things happend" });
            };


            Result<PimitiveType, SomeTestType> v = mover(true);
            CPPUNIT_ASSERT_EQUAL(1, PimitiveType::InstanceCount);
            CPPUNIT_ASSERT_EQUAL(0, SomeTestType::InstanceCount);

            const PimitiveType& res = v.unwrap();
            CPPUNIT_ASSERT_EQUAL(321, res.x);      // Needed to keep compiler happy
            CPPUNIT_ASSERT_EQUAL(1, PimitiveType::InstanceCount);
            CPPUNIT_ASSERT_EQUAL(0, SomeTestType::InstanceCount);

            Result<PimitiveType, SomeTestType> nak = mover(false);
            CPPUNIT_ASSERT_EQUAL(1, PimitiveType::InstanceCount);
            CPPUNIT_ASSERT_EQUAL(1, SomeTestType::InstanceCount);

            const SomeTestType& errRes = nak.getError();
            CPPUNIT_ASSERT_EQUAL(3, errRes.x);      // Needed to keep compiler happy
            CPPUNIT_ASSERT_EQUAL(1, PimitiveType::InstanceCount);
            CPPUNIT_ASSERT_EQUAL(1, SomeTestType::InstanceCount);
        }

        // Check post condition
        CPPUNIT_ASSERT_EQUAL(0, SimpleType::InstanceCount);
        CPPUNIT_ASSERT_EQUAL(0, SomeTestType::InstanceCount);
    }


    void getResultOnErrorThrows() {
        Result<int, char> v = Err('e');

        CPPUNIT_ASSERT_THROW(v.unwrap(), Exception);
    }

    void getErrorOnOkThrows() {
        Result<int, char> v = Ok(32);

        CPPUNIT_ASSERT_THROW(v.getError(), Exception);
    }

    void testVoidResult() {
        Result<void, int> v = Ok();

        CPPUNIT_ASSERT(v.isOk());

        bool thenCalled = false;
        auto derivedOk = v.then([&thenCalled]() {
            thenCalled = true;
            return Ok<int>(312);
        });

        CPPUNIT_ASSERT(thenCalled);
        CPPUNIT_ASSERT(derivedOk.isOk());
        CPPUNIT_ASSERT_EQUAL(312, derivedOk.unwrap());

        auto derivedErr = v.then([]() -> Result<const char*, int> { return Err<int>(-5); });
        CPPUNIT_ASSERT(derivedErr.isError());
        CPPUNIT_ASSERT_EQUAL(-5, derivedErr.getError());
    }

    void testThen() {
        auto f = [](bool isOk) -> Result<int, const char*> {
            if (isOk)
                return Ok(42);
            else
                return Err("somthing wrong");
        };

        {  // Test that success handler is called on success
            int thenValue = 0;
            const int cValue = f(true)
                    .then([&thenValue](const int& value) {
                        thenValue = value;

                        return Ok<int>(998);
                    })
                    .unwrap();

            // Make sure that success handler was called
            CPPUNIT_ASSERT_EQUAL(998, cValue);
            CPPUNIT_ASSERT_EQUAL(42, thenValue);
        }

        {  // Make sure that errback is called on failure
            int thenValue = 0;
            const int cValue = f(false)
                    .then([&thenValue](int value) {
                        thenValue = value;

                        return Ok<int>(-198);
                    })
                    .orElse([&thenValue](const char*) {
                        thenValue = 240;

                        return Ok<int>(-776);
                    })
                    .unwrap();

            // Make sure that errback handler was called
            CPPUNIT_ASSERT_EQUAL(-776, cValue);
            CPPUNIT_ASSERT_EQUAL(240, thenValue);
        }
    }


    void testThenChaining() {

        // Good chain
        Result<int, SimpleType> goodResult = Ok<int>(42);

        auto alsoGood = goodResult.then([](int r) { return Ok<int>(r / 2); });
        CPPUNIT_ASSERT(alsoGood.isOk());
        CPPUNIT_ASSERT_EQUAL(42/2, alsoGood.unwrap());

        auto lessGood = alsoGood.then([](int r) { return Ok<int>(r - 2); });
        CPPUNIT_ASSERT(lessGood.isOk());
        CPPUNIT_ASSERT_EQUAL(42/2 - 2, lessGood.unwrap());


        // Error chain
        Result<int, PimitiveType> badResult = Err<PimitiveType>(18);

        auto alsoNotGood = badResult.then([](int r) { return Ok<float>(r / 2); });
        CPPUNIT_ASSERT(alsoNotGood.isError());

        auto stillNotGood = alsoNotGood.then([](int r) { return Ok<int>(r + 21); });
        CPPUNIT_ASSERT(stillNotGood.isError());

        auto recovered = stillNotGood.orElse([](const PimitiveType& x) { return Ok<int>(x.x + 2); });

        CPPUNIT_ASSERT(recovered.isOk());
        CPPUNIT_ASSERT_EQUAL(20, recovered.unwrap());
    }


    void testThenComposition() {
        Result<int, SimpleType> initialResult = Ok<int>(112);

        Result<std::function<int()>, SimpleType> finalResult = initialResult
                .then([](int x)     { return Ok<float32>(x / 10); })
                .then([](float32 x) { return Ok<int>( floor(x) + 30); })
                .then([](int x)     { return Ok<std::function<int()>>([x]() { return (1 + x); }); });

        CPPUNIT_ASSERT(finalResult.isOk());
        CPPUNIT_ASSERT_EQUAL(42, finalResult.unwrap()());

        auto sq =  [](int x) -> Result<int, int> { return Ok<int>(x * x); };
        auto err = [](int x) -> Result<int, int> { return Err(x); };

        Result<int, int> ok2 = Ok(2);
        Result<int, int> err3 = Err(3);
        CPPUNIT_ASSERT(Ok(2) == ok2.orElse(sq).orElse(sq));
        CPPUNIT_ASSERT(Ok(2) == ok2.orElse(err).orElse(sq));
        CPPUNIT_ASSERT(Ok(9) == err3.orElse(sq).orElse(err));
        CPPUNIT_ASSERT(Err(3) == err3.orElse(err).orElse(err));
    }

    void testThenComposition_cv() {
        Result<int, SimpleType> initialResult = Ok<int>(112);

        Result<std::function<int()>, SimpleType> finalResult = initialResult
                .then([](int x)     { return Ok<float32>(x / 10); })
                .then([](float32 x) { return Ok<int>(floor(x) + 30); })
                .then([](int x)     { return Ok<std::function<int()>>([x]() { return (1 + x); }); });

        CPPUNIT_ASSERT(finalResult.isOk());
        CPPUNIT_ASSERT_EQUAL(42, finalResult.unwrap()());

        auto sq =  [](int x) -> Result<int, int> { return Ok<int>(x * x); };
        auto err = [](int x) -> Result<int, int> { return Err(x); };

        Result<int, int> ok2 = Ok(2);
        Result<int, int> err3 = Err(3);
        CPPUNIT_ASSERT(Ok(2) == ok2.orElse(sq).orElse(sq));
        CPPUNIT_ASSERT(Ok(2) == ok2.orElse(err).orElse(sq));
        CPPUNIT_ASSERT(Ok(9) == err3.orElse(sq).orElse(err));
        CPPUNIT_ASSERT(Err(3) == err3.orElse(err).orElse(err));
    }


    void testMapError() {
        Result<int, PimitiveType> res = Err<PimitiveType>(112);

        CPPUNIT_ASSERT(Err<String>("Error is 112") == res.mapError([](const PimitiveType& x){
            return String("Error is ").concat(String::valueOf(x.x));
        }) );
    }

    void testMoveOnlyObjects() {
        {
            Result<MoveOnlyType, SimpleType> res = Err<SimpleType>({112, 2, -1});
            CPPUNIT_ASSERT(res.isError());
        }

        {
            Result<MoveOnlyType, SimpleType> res = [] () {
                MoveOnlyType t(123);

                return Ok(std::move(t));
            } ();

            CPPUNIT_ASSERT(res.isOk());
        }

        {
            Result<int, MoveOnlyType> res = [] () {
                MoveOnlyType t(123);

                return Err(std::move(t));
            } ();

            CPPUNIT_ASSERT(res.isError());
        }
    }

    void testThenMovesObjects() {
        Result<MoveOnlyType, SimpleType> res = Ok<MoveOnlyType>({112});

        bool movedOut = false;
        res.then([&movedOut](MoveOnlyType&& m) {
            movedOut = m.x_ == 112;
            CPPUNIT_ASSERT(m.InstanceCount = 1);
        });

        CPPUNIT_ASSERT(movedOut);
    }

};


std::ostream& operator<<(std::ostream& ostr, const TestResult::SomeTestType& t) {
    return ostr << "SomeTestType(" << t.x << ", " << t.f << ", \"" << t.somethingElse << "\"";
}

int TestResult::SomeTestType::InstanceCount = 0;


CPPUNIT_TEST_SUITE_REGISTRATION(TestResult);

