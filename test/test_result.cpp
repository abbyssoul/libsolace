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
 *******************************************************************************/
#include <solace/result.hpp>			// Class being tested

#include <solace/unit.hpp>
#include <solace/error.hpp>
#include <solace/string.hpp>
#include <solace/exception.hpp>
#include <solace/posixErrorDomain.hpp>


#include <cmath>

#include <gtest/gtest.h>

#include "mockTypes.hpp"

using namespace Solace;


class TestResult : public ::testing::Test {

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


    void SetUp() override {
        EXPECT_EQ(0, SimpleType::InstanceCount);
        EXPECT_EQ(0, SomeTestType::InstanceCount);
        EXPECT_EQ(0, MoveOnlyType::InstanceCount);
    }

    void TearDown() override {
        EXPECT_EQ(0, SimpleType::InstanceCount);
        EXPECT_EQ(0, SomeTestType::InstanceCount);
        EXPECT_EQ(0, MoveOnlyType::InstanceCount);
    }

    Result<void, Error>
    fail(StringLiteral message) {
        return Err(makeError(BasicError::InvalidInput, message));
    }

	Result<int, SimpleType>
	failSimple(int x, int y, int z) {
		SimpleType result{x, y, z};

		return mv(result);
	}

	Result<int, MoveOnlyType>
	failMoveonly(int x) {
		MoveOnlyType result{x};

		return mv(result);
	}
};

TEST_F(TestResult, testErrfactoryProducesErrorFromCopy) {
    SimpleType value;
    Result<int, SimpleType> r = Err(value);

	EXPECT_TRUE(r.isError());
	EXPECT_EQ(2, SimpleType::InstanceCount);
}

TEST_F(TestResult, testErrfactoryProducesErrorFromMovedValue) {
    SimpleType value;
	Result<int, SimpleType> r = Err(mv(value));

	EXPECT_TRUE(r.isError());
	EXPECT_EQ(2, SimpleType::InstanceCount);
}


TEST_F(TestResult, testErrorByCopy) {
	Result<int, SimpleType> r = failSimple(33, 222, 1);

	EXPECT_TRUE(r.isError());
	EXPECT_EQ(r.getError().y, 222);
	EXPECT_EQ(1, SimpleType::InstanceCount);
}

TEST_F(TestResult, testErrorByMove) {
	Result<int, MoveOnlyType> r = failMoveonly(-7276);

	EXPECT_TRUE(r.isError());
	EXPECT_EQ(r.getError().x_, -7276);
	EXPECT_EQ(1, MoveOnlyType::InstanceCount);
}

TEST_F(TestResult, testVoidFailure) {
    {
        Result<void, Error> r = fail("Bad errors 432 about to happen");

        EXPECT_TRUE(!r.isOk());
        EXPECT_TRUE(r.isError());
    }

    bool value = false;
    {
        Result<void, Error> other = fail("Maybe no errors 9922 here");
        other.orElse([&value](Error&&){
            value = true;
        });

        EXPECT_TRUE(other.isError());
        EXPECT_TRUE(value);
    }
}


TEST_F(TestResult, testTypeConvertion) {
    {
        Result<int, Unit> r = Ok(10);
        EXPECT_TRUE(r.isOk());
    }

    {
        Result<PimitiveType, Unit> r = Ok<PimitiveType>(10);
        EXPECT_TRUE(r.isOk());
    }

    {
        Result<PimitiveType, Unit> r = Result<int, Unit>(Ok(10));
        EXPECT_TRUE(r.isOk());
    }
}

TEST_F(TestResult, testConstructionIntegrals) {
    {
        Result<void, int> v = Ok();
        EXPECT_TRUE(v.isOk());
    }
    {
        Unit x;
		Result<Unit, int> v = Ok(mv(x));
        EXPECT_TRUE(v.isOk());
    }
    {
        int x = 675;
        Result<Unit, int> v = Err(x);
        EXPECT_TRUE(v.isError());
        EXPECT_EQ(x, v.getError());
    }

    {
        int x = 8832;
		Result<int, Unit> v = Ok(mv(x));
        EXPECT_TRUE(v.isOk());
        EXPECT_EQ(x, v.unwrap());
    }

    {
        char x = 'x';
        Result<int, char> v = Err(x);
        EXPECT_TRUE(v.isError());
        EXPECT_EQ(x, v.getError());
    }
}


TEST_F(TestResult, testErrorTypeCoersion) {
	SimpleType errValue{3, 2, 1};
	Result<Unit, SimpleType> v{ errValue };
	EXPECT_TRUE(v.isError());
	EXPECT_EQ(v.getError().y, 2);
}

TEST_F(TestResult, testValueTypeCoersion) {
	SimpleType value{3, 2, 1};
	Result<SimpleType, Unit> v{ value };
	EXPECT_TRUE(v.isOk());
	EXPECT_EQ(v.unwrap().y, 2);
}


TEST_F(TestResult, testConstruction) {

    {  // Unit result
        {
            auto const v = []() -> Result<Unit, int> {
                return Ok(Unit());
            } ();

            EXPECT_TRUE(v.isOk());
        }

        {
            auto const v = []() -> Result<Unit, int> {
                return Err(-1);
            } ();

            EXPECT_TRUE(v.isError());
        }
    }

    {  // Integral result
        {
            int x = 321;
            const auto& v = [](int y) -> Result<int, float> {

				return Ok(mv(y));
            } (x);

            EXPECT_TRUE(v.isOk());
            EXPECT_EQ(x, v.unwrap());
        }

        {
            const char x = 'x';
            const auto& v = [](char t) -> Result<int, char> {
				return Err(mv(t));
            } (x);

            EXPECT_TRUE(v.isError());
            EXPECT_EQ(x, v.getError());
        }
    }

    EXPECT_EQ(0, SomeTestType::InstanceCount);
    {  // non-Pod result
        {
            const auto& v = []() -> Result<SomeTestType, int> {
                auto r = SomeTestType{321, 3.1415f, "Somethere"};

				return Ok(mv(r));
            } ();

            EXPECT_TRUE(v.isOk());
            EXPECT_EQ(321, v.unwrap().x);
            EXPECT_EQ(1, SomeTestType::InstanceCount);
        }

        EXPECT_EQ(0, SomeTestType::InstanceCount);
        {
            const auto& v = []() -> Result<SomeTestType, int> {
                return Err(-998);
            } ();

            EXPECT_TRUE(v.isError());
            EXPECT_EQ(-998, v.getError());
            EXPECT_EQ(0, SomeTestType::InstanceCount);
        }
    }
}

TEST_F(TestResult, testMoveAssignment) {
    {
        Result<SomeTestType, int> v1 = Err(321);
        Result<SomeTestType, int> v2 = Ok<SomeTestType>({3, 2.718f, "Test value"});

        EXPECT_TRUE(v1.isError());
        EXPECT_TRUE(v2.isOk());
        EXPECT_EQ(3, v2.unwrap().x);
        EXPECT_EQ(1, SomeTestType::InstanceCount);

		v1 = mv(v2);
        EXPECT_EQ(1, SomeTestType::InstanceCount);
        EXPECT_TRUE(v1.isOk());
        EXPECT_TRUE(v2.isError());
        EXPECT_EQ(3, v1.unwrap().x);

//            v1 = v2;
//            EXPECT_EQ(0, SomeTestType::InstanceCount);
//            EXPECT_TRUE(v1.isError());
//            EXPECT_TRUE(v2.isError());
    }

    // Check preconditions
    EXPECT_EQ(0, SimpleType::InstanceCount);
    EXPECT_EQ(0, SomeTestType::InstanceCount);
    {
        auto mover = [] (bool isOk) -> Result<PimitiveType, SomeTestType> {
            if (isOk)
                return Ok<PimitiveType>({321});
            else
                return Err<SomeTestType>({ 3, 2.3f, "Bad things happend" });
        };


        Result<PimitiveType, SomeTestType> v = mover(true);
        EXPECT_EQ(1, PimitiveType::InstanceCount);
        EXPECT_EQ(0, SomeTestType::InstanceCount);

        const PimitiveType& res = v.unwrap();
        EXPECT_EQ(321, res.x);      // Needed to keep compiler happy
        EXPECT_EQ(1, PimitiveType::InstanceCount);
        EXPECT_EQ(0, SomeTestType::InstanceCount);

        Result<PimitiveType, SomeTestType> nak = mover(false);
        EXPECT_EQ(1, PimitiveType::InstanceCount);
        EXPECT_EQ(1, SomeTestType::InstanceCount);

        const SomeTestType& errRes = nak.getError();
        EXPECT_EQ(3, errRes.x);      // Needed to keep compiler happy
        EXPECT_EQ(1, PimitiveType::InstanceCount);
        EXPECT_EQ(1, SomeTestType::InstanceCount);
    }

    // Check post condition
    EXPECT_EQ(0, SimpleType::InstanceCount);
    EXPECT_EQ(0, SomeTestType::InstanceCount);
}


TEST_F(TestResult, getResultOnErrorThrows) {
    Result<int, char> v = Err('e');

    EXPECT_THROW(v.unwrap(), Exception);
}

TEST_F(TestResult, getErrorOnOkThrows) {
    Result<int, char> v = Ok(32);

    EXPECT_THROW(v.getError(), Exception);
}

TEST_F(TestResult, dereferencingOk) {
    Result<int, char> v = Ok(32);

    EXPECT_EQ(32, *v);
}

TEST_F(TestResult, dereferencingErrThrows) {
    Result<int, char> v = Err('3');

    EXPECT_THROW(*v, Exception);
}


TEST_F(TestResult, testVoidResult) {
    Result<void, int> v = Ok();

    EXPECT_TRUE(v.isOk());

    bool thenCalled = false;
    auto derivedOk = v.then([&thenCalled]() {
        thenCalled = true;
        return Ok<int>(312);
    });

    EXPECT_TRUE(thenCalled);
    EXPECT_TRUE(derivedOk.isOk());
    EXPECT_EQ(312, derivedOk.unwrap());

    auto derivedErr = v.then([]() -> Result<const char*, int> { return Err<int>(-5); });
    EXPECT_TRUE(derivedErr.isError());
    EXPECT_EQ(-5, derivedErr.getError());
}

TEST_F(TestResult, testThen) {
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
        EXPECT_EQ(998, cValue);
        EXPECT_EQ(42, thenValue);
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
        EXPECT_EQ(-776, cValue);
        EXPECT_EQ(240, thenValue);
    }
}

TEST_F(TestResult, testThenArgumentIgnored) {
    /* TODO(abbyssoul): A separate PR is required
    auto f = [](bool isOk) -> Result<int, const char*> {
        if (isOk)
            return Ok(42);
        else
            return Err("somthing wrong");
    };

    {  // Test that success handler is called on success
        bool thenCalled = false;
        f(true)
            .then([&thenCalled]() {
                thenCalled = true;
            });

        // Make sure that success handler was called
        EXPECT_TRUE(thenCalled);
    }
*/
}

TEST_F(TestResult, testThenChaining) {

    // Good chain
    Result<int, SimpleType> goodResult = Ok<int>(42);

    auto const alsoGood = goodResult.then([](int r) { return Ok<int>(r / 2); });
    EXPECT_TRUE(alsoGood.isOk());
    EXPECT_EQ(42/2, alsoGood.unwrap());

    auto lessGood = alsoGood.then([](int r) { return Ok<int>(r - 2); });
    EXPECT_TRUE(lessGood.isOk());
    EXPECT_EQ(42/2 - 2, lessGood.unwrap());


    // Error chain
    Result<int, PimitiveType> badResult = Err<PimitiveType>(18);

    auto alsoNotGood = badResult.then([](int r) { return Ok<float>(r / 2); });
    EXPECT_TRUE(alsoNotGood.isError());

    auto stillNotGood = alsoNotGood.then([](int r) { return Ok<int>(r + 21); });
    EXPECT_TRUE(stillNotGood.isError());

    auto recovered = stillNotGood.orElse([](const PimitiveType& x) { return Ok<int>(x.x + 2); });

    EXPECT_TRUE(recovered.isOk());
    EXPECT_EQ(20, recovered.unwrap());
}


TEST_F(TestResult, testThenComposition) {
    Result<int, SimpleType> initialResult = Ok<int>(112);

    Result<std::function<int()>, SimpleType> finalResult = initialResult
            .then([](int x)     { return Ok<float32>(x / 10); })
            .then([](float32 x) { return Ok<int>( floor(x) + 30); })
            .then([](int x)     { return Ok<std::function<int()>>([x]() { return (1 + x); }); });

    EXPECT_TRUE(finalResult.isOk());
    EXPECT_EQ(42, finalResult.unwrap()());

    auto sq =  [](int x) -> Result<int, int> { return Ok<int>(x * x); };
    auto err = [](int x) -> Result<int, int> { return Err(x); };

    Result<int, int> ok2 = Ok(2);
    Result<int, int> err3 = Err(3);
    EXPECT_TRUE(Ok(2) == ok2.orElse(sq).orElse(sq));
    EXPECT_TRUE(Ok(2) == ok2.orElse(err).orElse(sq));
    EXPECT_TRUE(Ok(9) == err3.orElse(sq).orElse(err));
    EXPECT_TRUE(Err(3) == err3.orElse(err).orElse(err));
}

TEST_F(TestResult, testThenComposition_cv) {
    Result<int, SimpleType> initialResult = Ok<int>(112);

    Result<std::function<int()>, SimpleType> finalResult = initialResult
            .then([](int x)     { return Ok<float32>(x / 10); })
            .then([](float32 x) { return Ok<int>(floor(x) + 30); })
            .then([](int x)     { return Ok<std::function<int()>>([x]() { return (1 + x); }); });

    EXPECT_TRUE(finalResult.isOk());
    EXPECT_EQ(42, finalResult.unwrap()());

    auto sq =  [](int x) -> Result<int, int> { return Ok<int>(x * x); };
    auto err = [](int x) -> Result<int, int> { return Err(x); };

    Result<int, int> ok2 = Ok(2);
    Result<int, int> err3 = Err(3);
    EXPECT_TRUE(Ok(2) == ok2.orElse(sq).orElse(sq));
    EXPECT_TRUE(Ok(2) == ok2.orElse(err).orElse(sq));
    EXPECT_TRUE(Ok(9) == err3.orElse(sq).orElse(err));
    EXPECT_TRUE(Err(3) == err3.orElse(err).orElse(err));
}


TEST_F(TestResult, testThenToVoid) {
    Result<int, SimpleType> initialResult = Ok<int>(112);

    auto fmap =[](int) -> Result<void, SimpleType> { return Ok(); };

    Result<void, SimpleType> finalResult = initialResult
            .then([&fmap](int x) { return fmap(x); });

    EXPECT_TRUE(finalResult.isOk());
}



TEST_F(TestResult, testMapError) {
    Result<int, PimitiveType> res = Err<PimitiveType>(112);

    EXPECT_TRUE(Err<StringLiteral>("Error is 112") == res.mapError([](PimitiveType const& x) {
                return (x.x == 112)
                    ? StringLiteral{"Error is 112"}
                    : StringLiteral{"Error is unknown"};
                }));

    Result<void, PimitiveType> res2 = Err<PimitiveType>(321);
    Result<void, SimpleType> mappedRes = Err(SimpleType{321, 1, 2});
    EXPECT_TRUE(mappedRes == res2.mapError([](PimitiveType const& x) {
                    return Err(SimpleType{x.x, 1, 2});
                }));
}


TEST_F(TestResult, testMoveOnlyObjects) {
    {
        Result<MoveOnlyType, SimpleType> res = Err<SimpleType>({112, 2, -1});
        EXPECT_TRUE(res.isError());
    }

    {
        Result<MoveOnlyType, SimpleType> res = [] () {
            MoveOnlyType t(123);

			return Ok(mv(t));
        } ();

        EXPECT_TRUE(res.isOk());
    }

    {
        Result<int, MoveOnlyType> res = [] () {
            MoveOnlyType t(123);

			return Err(mv(t));
        } ();

        EXPECT_TRUE(res.isError());
    }
}

TEST_F(TestResult, testThenMovesObjects) {
    Result<MoveOnlyType, SimpleType> res = Ok<MoveOnlyType>({112});

    bool movedOut = false;
    res.then([&movedOut](MoveOnlyType&& m) {
        movedOut = m.x_ == 112;
        EXPECT_TRUE(m.InstanceCount = 1);
    });

    EXPECT_TRUE(movedOut);
}

TEST_F(TestResult, testErrorCoersion) {
	auto l1 =[] () -> Result<void, SimpleType> {
		return SimpleType{1, 2, 3};
	};

	auto l2 = [&]() -> Result<int, SimpleType> {
		auto x = l1();
		if (!x) {
			return x.getError();
		} else {
			return Ok<int>(321);
		}
	};

	auto result = l2();
	EXPECT_TRUE(result.isError());

	auto const expectedErrorValue = SimpleType{1, 2, 3};
	EXPECT_EQ(result.getError(), expectedErrorValue);
}

std::ostream& operator<<(std::ostream& ostr, const TestResult::SomeTestType& t) {
    return ostr << "SomeTestType(" << t.x << ", " << t.f << ", \"" << t.somethingElse << "\"";
}

int TestResult::SomeTestType::InstanceCount = 0;
