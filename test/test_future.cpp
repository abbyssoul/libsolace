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
 * @file: test/io/async/test_future.cpp
 * @author: soultaker
 *
 * Created on: 29/05/2017
*******************************************************************************/
#include <solace/future.hpp>  // Class being tested
#include <solace/exception.hpp>
#include <solace/array.hpp>
#include <solace/output_utils.hpp>

#include <gtest/gtest.h>

#include "mockTypes.hpp"

#include <thread>
#include <chrono>


using namespace Solace;


static const AtomValue kFutureTestCatergory = atom("fut-test");

inline
Error makeFutureError(int errCode, StringLiteral tag) {
    return Error{kFutureTestCatergory, errCode, tag};
}



static int resolveVoidFunc() {
    return 99881;
}


template<typename T>
Future<T> makeOrphant() {
    auto p = Promise<T>();

    return p.getFuture();
}



TEST(TestFuture, orphanIntegralFutureThrows) {
    auto f = makeOrphant<int>();

    int x = 9;

    EXPECT_THROW(f.then([&x](int i) { x += i;}),
                            Solace::Exception);
    EXPECT_THROW(f.onError([&x](Solace::Error&& ) { return x += 12; }),
                            Solace::Exception);
}

TEST(TestFuture, orphanVoidFutureThrows) {
    auto f = makeOrphant<void>();

    int x = 9;

    EXPECT_THROW(f.then([&x]() { x += 12;}),
                            Solace::Exception);
    EXPECT_THROW(f.onError([&x](Solace::Error&& ) { x += 12;}),
                            Solace::Exception);
}

TEST(TestFuture, destoyingIntFuturePropagatesViaThen) {
    Promise<int> p;
    bool resolved1 = false;


    {
        p.getFuture().then([&resolved1](int) {
            resolved1 = true;

            return;
        });
    }

    p.setValue(321);

    EXPECT_TRUE(resolved1);
}


TEST(TestFuture, destoyingVoidFuturePropagatesViaThen) {
    Promise<void> p;
    bool resolved1 = false;


    {
        p.getFuture().then([&resolved1]() {
            resolved1 = true;
        });
    }

    p.setValue();

    EXPECT_TRUE(resolved1);
}

TEST(TestFuture, integralFutureIntegralContinuation) {
    auto p = Promise<int>();
    auto f = p.getFuture();
    bool resolved1 = false;
    bool resolved2 = false;

    f.then([&resolved1](int x) {
        resolved1 = (x == 120);

        return 'y';
    })
    .then([&resolved2](char c) {
        resolved2 = (c == 'y');
    });

    p.setValue(120);

    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(resolved2);
}

TEST(TestFuture, integralFutureVoidContinuation) {
    auto p = Promise<int>();
    auto f = p.getFuture();
    bool resolved1 = false;
    bool resolved2 = false;

    f.then([&resolved1](int x) {
        resolved1 = (x == 120);
    })
    .then([&resolved2]() {
            resolved2 = true;
        });

    p.setValue(120);

    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(resolved2);
}


TEST(TestFuture, voidFutureIntegralContinuation) {
    auto p = Promise<void>();
    auto f = p.getFuture();
    bool resolved1 = false;
    bool resolved2 = false;

    f.then([&resolved1]() {
        resolved1 = true;

        return 124;
    })
    .then([&resolved2](int x) {
            resolved2 = (x == 124);
        });

    p.setValue();

    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(resolved2);
}

TEST(TestFuture, voidFutureVoidContinuation) {
    auto p = Promise<void>();
    auto f = p.getFuture();
    bool resolved1 = false;
    bool resolved2 = false;

    // f: Future<void>
    f.then([&resolved1]() {
        resolved1 = true;
    })  //  : Future<void>
    .then([]() {
        return 321;
    })  // : Future<int>
    .then([&resolved2](int x) {
        resolved2 = (x == 321);
    });  // : Future<void>

    p.setValue();

    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(resolved2);
}



TEST(TestFuture, structFutureErrorContinuation) {
    auto p = Promise<int>();
    auto f = p.getFuture();
    bool resolved1 = false;
    bool resolved2 = false;
    bool resolved3 = false;

    f.then([&resolved1](int x) {
        resolved1 = (x == 120);

        return SimpleType(3, 5, 7);
    })
    .onError([&resolved2](Error&& e) {
        resolved2 = (e.value() == 789);

        return SimpleType(4, 8, 12);
    })
    .then([&resolved3](SimpleType&& c) {
        resolved3 = (c.x == 4);
    });

    p.setError(makeFutureError(789, "structFutureErrorContinuation"));

    EXPECT_TRUE(!resolved1);
    EXPECT_TRUE(resolved2);
    EXPECT_TRUE(resolved3);
}

TEST(TestFuture, integralFutureErrorContinuation) {
    auto p = Promise<int>();
    auto f = p.getFuture();
    bool resolved1 = false;
    bool resolved2 = false;
    bool resolved3 = false;

    f.then([&resolved1](int x) {
        resolved1 = (x == 120);

        return 'y';
    })
    .onError([&resolved2](Error&& e) {
        resolved2 = (e.value() == 4433);

        return 'n';
    })
    .then([&resolved3](char c) {
        resolved3 = (c == 'n');
    });

    p.setError(makeFutureError(4433, "integralFutureErrorContinuation"));

    EXPECT_TRUE(!resolved1);
    EXPECT_TRUE(resolved2);
    EXPECT_TRUE(resolved3);
}


TEST(TestFuture, voidFutureErrorContinuation) {
    auto p = Promise<void>();
    auto f = p.getFuture();
    bool resolved1 = false;
    bool resolved2 = false;
    bool resolved3 = false;

    f.then([&resolved1]() {
        resolved1 = true;
    })
    .onError([&resolved2](Error&& e) {
        resolved2 = (e.value() == 789);
    })
    .then([&resolved3]() {
        resolved3 = true;
    });

    p.setError(makeFutureError(789, "voidFutureErrorContinuation"));

    EXPECT_TRUE(!resolved1);
    EXPECT_TRUE(resolved2);
    EXPECT_TRUE(resolved3);
}


TEST(TestFuture, structFutureErrorResultErrors) {
    auto p = Promise<SimpleType>();
    auto f = p.getFuture();
    bool resolved1 = false;
    bool resolved2 = false;
    bool resolved3 = false;
    bool resolved4 = false;

    f.then([&resolved1](SimpleType&& ) {
        resolved1 = true;

        return SimpleType(17, 4, 1);
    })
    .onError([&resolved2](Error&& e) -> Result<SimpleType, Error> {
        resolved2 = (e.value() == 5355);

        return Err(makeFutureError(-8152, "structFutureErrorResultErrors-1"));
    })
    .then([&resolved3](SimpleType&& ) {
        resolved3 = true;
    })
    .onError([&resolved4](Error&& e) {
        resolved4 = (e.value() == -8152);
    });

    p.setError(makeFutureError(5355, "structFutureErrorResultErrors-2"));

    EXPECT_TRUE(!resolved1);
    EXPECT_TRUE(resolved2);
    EXPECT_TRUE(!resolved3);
    EXPECT_TRUE(resolved4);
}

TEST(TestFuture, integralFutureErrorResultErrors) {
    auto p = Promise<int>();
    auto f = p.getFuture();
    bool resolved1 = false;
    bool resolved2 = false;
    bool resolved3 = false;
    bool resolved4 = false;

    f.then([&resolved1](int ) {
        resolved1 = true;

        return 345;
    })
    .onError([&resolved2](Error&& e) -> Result<int, Error> {
        resolved2 = (e.value() == 5355);

        return Err(makeFutureError(-8152, "integralFutureErrorResultErrors-1"));
    })
    .then([&resolved3](int ) {
        resolved3 = true;
    })
    .onError([&resolved4](Error&& e) {
        resolved4 = (e.value() == -8152);
    });

    p.setError(makeFutureError(5355, "integralFutureErrorResultErrors-2"));

    EXPECT_TRUE(!resolved1);
    EXPECT_TRUE(resolved2);
    EXPECT_TRUE(!resolved3);
    EXPECT_TRUE(resolved4);
}

TEST(TestFuture, voidFutureErrorResultErrors) {
    auto p = Promise<void>();
    auto f = p.getFuture();
    bool resolved1 = false;
    bool resolved2 = false;
    bool resolved3 = false;
    bool resolved4 = false;

    f.then([&resolved1]() {
        resolved1 = true;
    })
    .onError([&resolved2](Error&& e) -> Result<void, Error> {
        resolved2 = (e.value() == 7744);

        return Err(makeFutureError(-4424, "voidFutureErrorResultErrors-1"));
    })
    .then([&resolved3]() {
        resolved3 = true;
    })
    .onError([&resolved4](Error&& e) {
        resolved4 = (e.value() == -4424);
    });

    p.setError(makeFutureError(7744, "voidFutureErrorResultErrors-2"));

    EXPECT_TRUE(!resolved1);
    EXPECT_TRUE(resolved2);
    EXPECT_TRUE(!resolved3);
    EXPECT_TRUE(resolved4);
}


//------------------------------------------------------------------------------------------------------------------
// Tests for continuations returning Futures
//------------------------------------------------------------------------------------------------------------------

TEST(TestFuture, integralFutureIntegralFutureContinuation) {
    bool resolved1 = false;
    bool resolved2 = false;

    Promise<int> p1;
    auto f = p1.getFuture();

    Promise<char> p2;

    f.then([&resolved1, &p2](int x) {
        resolved1 = (x == 310);

        return p2.getFuture();
    })
    .then([&resolved2](char x) {
        resolved2 = (x == 'y');
    });

    p1.setValue(310);
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(!resolved2);

    p2.setValue('y');
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(resolved2);
}

TEST(TestFuture, voidFutureIntegralFutureContinuation) {
    bool resolved1 = false;
    bool resolved2 = false;

    auto p1 = Promise<void>();
    auto f = p1.getFuture();

    auto p2 = Promise<int>();

    f.then([&resolved1, &p2]() {
        resolved1 = true;

        return p2.getFuture();
    })
    .then([&resolved2](int x) {
        resolved2 = (x == -9817);
    });

    p1.setValue();
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(!resolved2);

    p2.setValue(-9817);
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(resolved2);
}

TEST(TestFuture, integralFutureVoidFutureContinuation) {
    bool resolved1 = false;
    bool resolved2 = false;

    auto p1 = Promise<int>();
    auto f = p1.getFuture();

    auto p2 = Promise<void>();

    f.then([&resolved1, &p2](int x) {
        resolved1 = (x == 887);

        return p2.getFuture();
    })
    .then([&resolved2]() {
        resolved2 = true;
    });

    p1.setValue(887);
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(!resolved2);

    p2.setValue();
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(resolved2);
}

TEST(TestFuture, voidFutureVoidFutureContinuation) {
    bool resolved1 = false;
    bool resolved2 = false;

    auto p1 = Promise<void>();
    auto f = p1.getFuture();

    auto p2 = Promise<void>();

    f.then([&resolved1, &p2]() {
        resolved1 = true;

        return p2.getFuture();
    })
    .then([&resolved2]() {
        resolved2 = true;
    });

    p1.setValue();
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(!resolved2);

    p2.setValue();
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(resolved2);
}



TEST(TestFuture, integralFutureIntegralFutureErrorsContinuation) {
    bool resolved1 = false;
    bool resolved2 = false;
    bool resolved3 = false;

    auto p1 = Promise<char>();
    auto f = p1.getFuture();

    auto p2 = Promise<int>();

    f.then([&resolved1, &p2](char x) {
        resolved1 = (x == 'n');

        return p2.getFuture();
    })
    .then([&resolved2](int ) {
        resolved2 = true;
    })
    .onError([&resolved3](Error&& e) {
        resolved3 = (e.value() == 123);
    });

    p1.setValue('n');
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(!resolved2);
    EXPECT_TRUE(!resolved3);

    p2.setError(makeFutureError(123, "integralFutureIntegralFutureErrorsContinuation"));
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(!resolved2);
    EXPECT_TRUE(resolved3);
}

TEST(TestFuture, voidFutureIntegralFutureErrorsContinuation) {
    bool resolved1 = false;
    bool resolved2 = false;
    bool resolved3 = false;

    auto p1 = Promise<void>();
    auto f = p1.getFuture();

    auto p2 = Promise<int>();

    f.then([&resolved1, &p2]() {
        resolved1 = true;

        return p2.getFuture();
    })
    .then([&resolved2](int ) {
        resolved2 = true;
    })
    .onError([&resolved3](Error&& e) {
        resolved3 = (e.value() == 321);
    });

    p1.setValue();
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(!resolved2);
    EXPECT_TRUE(!resolved3);

    p2.setError(makeFutureError(321, "voidFutureIntegralFutureErrorsContinuation"));
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(!resolved2);
    EXPECT_TRUE(resolved3);
}

TEST(TestFuture, integralFutureVoidFutureErrorsContinuation) {
    bool resolved1 = false;
    bool resolved2 = false;
    bool resolved3 = false;

    auto p1 = Promise<char>();
    auto f = p1.getFuture();

    auto p2 = Promise<void>();

    f.then([&resolved1, &p2](char x) {
        resolved1 = (x == 'n');

        return p2.getFuture();
    })
    .then([&resolved2]() {
        resolved2 = true;
    })
    .onError([&resolved3](Error&& e) {
        resolved3 = (e.value() == -231);
    });

    p1.setValue('n');
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(!resolved2);
    EXPECT_TRUE(!resolved3);

    p2.setError(makeFutureError(-231, "integralFutureVoidFutureErrorsContinuation"));
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(!resolved2);
    EXPECT_TRUE(resolved3);
}

TEST(TestFuture, voidFutureVoidFutureErrorsContinuation) {
    bool resolved1 = false;
    bool resolved2 = false;
    bool resolved3 = false;

    auto p1 = Promise<void>();
    auto f = p1.getFuture();

    auto p2 = Promise<void>();

    f.then([&resolved1, &p2]() {
        resolved1 = true;

        return p2.getFuture();
    })
    .then([&resolved2]() {
        resolved2 = true;
    })
    .onError([&resolved3](Error&& e) {
        resolved3 = (e.value() == 543);
    });

    p1.setValue();
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(!resolved2);
    EXPECT_TRUE(!resolved3);

    p2.setError(makeFutureError(543, "voidFutureVoidFutureErrorsContinuation"));
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(!resolved2);
    EXPECT_TRUE(resolved3);
}


//------------------------------------------------------------------------------------------------------------------
// Tests for continuations returning Results
//------------------------------------------------------------------------------------------------------------------

TEST(TestFuture, integralFutureIntegralResultContinuation) {
    bool resolved1 = false;
    bool resolved2 = false;

    auto p1 = Promise<char>();
    auto f = p1.getFuture();

    f.then([&resolved1](char x) -> Result<int, Error> {
        resolved1 = (x == 'y');

        return Ok(6568);
    })
    .then([&resolved2](int x) {
        resolved2 = (x == 6568);
    });

    p1.setValue('y');
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(resolved2);
}

TEST(TestFuture, voidFutureIntegralResultContinuation) {
    bool resolved1 = false;
    bool resolved2 = false;

    auto p1 = Promise<void>();
    auto f = p1.getFuture();

    f.then([&resolved1]() -> Result<int, Error> {
        resolved1 = true;

        return Ok(321);
    })
    .then([&resolved2](int x) {
        resolved2 = (x == 321);
    });

    p1.setValue();
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(resolved2);
}

TEST(TestFuture, integralFutureVoidResultContinuation) {
    bool resolved1 = false;
    bool resolved2 = false;

    auto p1 = Promise<int>();
    auto f = p1.getFuture();

    f.then([&resolved1](int x) -> Result<void, Error> {
        resolved1 = (x == 982);

        return Ok();
    })
    .then([&resolved2]() {
        resolved2 = true;
    });

    p1.setValue(982);
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(resolved2);
}

TEST(TestFuture, voidFutureVoidResultContinuation) {
    bool resolved1 = false;
    bool resolved2 = false;

    auto p1 = Promise<void>();
    auto f = p1.getFuture();

    f.then([&resolved1]() -> Result<void, Error> {
        resolved1 = true;

        return Ok();
    })
    .then([&resolved2]() {
        resolved2 = true;
    });

    p1.setValue();
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(resolved2);
}



TEST(TestFuture, integralFutureIntegralResultErrorsContinuation) {
    bool resolved1 = false;
    bool resolved2 = false;
    bool resolved3 = false;

    auto p1 = Promise<char>();
    auto f = p1.getFuture();

    f.then([&resolved1](char x) -> Result<int, Error> {
        resolved1 = (x == 'n');

        return Err(makeFutureError(-525, "integralFutureIntegralResultErrorsContinuation"));
    })
    .then([&resolved2](int ) {
        resolved2 = true;
    })
    .onError([&resolved3](Error&& e) {
        resolved3 = (e.value() == -525);
    });

    p1.setValue('n');
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(!resolved2);
    EXPECT_TRUE(resolved3);
}

TEST(TestFuture, voidFutureIntegralResultErrorsContinuation) {
    bool resolved1 = false;
    bool resolved2 = false;
    bool resolved3 = false;

    auto p1 = Promise<void>();
    auto f = p1.getFuture();

    f.then([&resolved1]() -> Result<int, Error> {
        resolved1 = true;

        return Err(makeFutureError(-525, "voidFutureIntegralResultErrorsContinuation"));
    })
    .then([&resolved2](int x) {
        resolved2 = (x == 6568);
    })
    .onError([&resolved3](Error&& e) {
        resolved3 = (e.value() == -525);
    });

    p1.setValue();
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(!resolved2);
    EXPECT_TRUE(resolved3);
}

TEST(TestFuture, integralFutureVoidResultErrorsContinuation) {
    bool resolved1 = false;
    bool resolved2 = false;
    bool resolved3 = false;

    auto p1 = Promise<char>();
    auto f = p1.getFuture();

    f.then([&resolved1](char x) -> Result<void, Error> {
        resolved1 = (x == 'n');

        return Err(makeFutureError(-525, "integralFutureVoidResultErrorsContinuation"));
    })
    .then([&resolved2]() {
        resolved2 = true;
    })
    .onError([&resolved3](Error&& e) {
        resolved3 = (e.value() == -525);
    });

    p1.setValue('n');
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(!resolved2);
    EXPECT_TRUE(resolved3);
}

TEST(TestFuture, voidFutureVoidResultErrorsContinuation) {
    bool resolved1 = false;
    bool resolved2 = false;
    bool resolved3 = false;

    Promise<void> p1;
    auto f = p1.getFuture();

    f.then([&resolved1](void) -> Result<void, Error> {
        resolved1 = true;

        return Err(makeFutureError(95546, "voidFutureVoidResultErrorsContinuation"));
    })
    .then([&resolved2]() {
        resolved2 = true;
    })
    .onError([&resolved3](Error&& e) {
        resolved3 = (e.value() == 95546);
    });

    p1.setValue();
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(!resolved2);
    EXPECT_TRUE(resolved3);
}



TEST(TestFuture, testThenWithStandaloneFunction) {
    bool resolved1 = false;
    bool errored1 = false;

    Promise<void> p1;
    auto f = p1.getFuture();

    f.then(resolveVoidFunc)
            .then([&resolved1](int x) {
        resolved1 = (x == 99881);
    })
    .onError([&errored1](Error&& ) {
        errored1 = true;
    });

    p1.setValue();

    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(!errored1);
}









TEST(TestFuture, testThenResultContinuation) {
    auto p = Promise<int>();
    auto f = p.getFuture();
    bool resolved = false;

    bool firstCallbackOk = false;
    bool secondCallbackOk = false;

    f.then([&firstCallbackOk](int x) -> Result<char, Error> {
        firstCallbackOk = (x == 120);
        return Ok( (x == 120) ? 'Y': 'N');
    })
    .then([&resolved, &secondCallbackOk](const char& c) {
        secondCallbackOk = (c == 'Y');
        EXPECT_EQ('Y', c);

        resolved = true;
    });

    p.setValue(120);

    EXPECT_TRUE(firstCallbackOk);
    EXPECT_TRUE(secondCallbackOk);
    EXPECT_TRUE(resolved);
}


TEST(TestFuture, testThenVoidResultContinuation) {
    auto p = Promise<int>();
    auto f = p.getFuture();
    bool resolved = false;

    bool firstCallbackOk = false;
    bool secondCallbackOk = false;

    f.then([&firstCallbackOk](int x) -> Result<void, Error> {
        firstCallbackOk = (x == 120);

        return Ok();
    })
    .then([&resolved, &secondCallbackOk]() {
        secondCallbackOk = true;

        resolved = true;
    });

    p.setValue(120);

    EXPECT_TRUE(firstCallbackOk);
    EXPECT_TRUE(secondCallbackOk);
    EXPECT_TRUE(resolved);
}



TEST(TestFuture, testThenFutureContinuation) {
    auto p1 = Promise<int>();
    auto f1 = p1.getFuture();

    auto p2 = Promise<SimpleType>();

    bool firstCallbackOk = false;
    bool secondCallbackOk = false;

    auto ff = [&p2](int SOLACE_UNUSED(x)) {
        return p2.getFuture();
    };

    f1.then([&firstCallbackOk, &ff](int x) {
        firstCallbackOk = (x == 120);

        return ff(x / 2);
    })
    .then([&secondCallbackOk](const SimpleType& c) {
        secondCallbackOk = (c.x == 60);
    });

    p1.setValue(120);
    EXPECT_TRUE(firstCallbackOk);
    EXPECT_TRUE(!secondCallbackOk);

    p2.setValue(SimpleType(60, 1, 3));
    EXPECT_TRUE(firstCallbackOk);
    EXPECT_TRUE(secondCallbackOk);
}


TEST(TestFuture, testOnErrorHandler) {
    auto p1 = Promise<int>();
    auto f1 = p1.getFuture();

    bool firstCallbackOk = false;
    bool secondCallbackOk = false;

    f1.then([&firstCallbackOk](int x) {
        firstCallbackOk = (x == 120);

        return 2;
    }).onError([&secondCallbackOk](Error&& ) {
        secondCallbackOk = true;

        return 3;
    });


    p1.setError(makeFutureError(0, "testOnErrorHandler"));
    EXPECT_TRUE(!firstCallbackOk);
    EXPECT_TRUE(secondCallbackOk);
}

TEST(TestFuture, testOnErrorRestoresTheChain) {
    auto p1 = Promise<int>();
    auto f1 = p1.getFuture();

    bool firstCallbackOk = false;
    bool secondCallbackOk = false;
    bool thirdCallbackOk = false;

    f1.then([&firstCallbackOk](int x) {
        firstCallbackOk = (x == 120);

        return 2;
    }).onError([&secondCallbackOk](Error&& e) {
        if (e)
            secondCallbackOk = true;

        return 17;
    }).then([&thirdCallbackOk](int x) {
        thirdCallbackOk = (x == 17);
    });


    p1.setError(makeFutureError(1, "testOnErrorRestoresTheChain"));
    EXPECT_TRUE(!firstCallbackOk);
    EXPECT_TRUE(secondCallbackOk);
    EXPECT_TRUE(thirdCallbackOk);
}

TEST(TestFuture, testOnErrorSkippedOnSuccess) {
    auto p1 = Promise<int>();
    auto f1 = p1.getFuture();

    bool firstCallbackOk = false;
    bool secondCallbackOk = false;
    bool thirdCallbackOk = false;

    f1.then([&firstCallbackOk](int x) {
        firstCallbackOk = (x == 120);

        return 2;
    }).onError([&secondCallbackOk](Error&& e) {

        if (e)
            secondCallbackOk = true;

        return -31;
    }).then([&thirdCallbackOk](int x) {
        thirdCallbackOk = (x == 2);
    });


    p1.setValue(120);
    EXPECT_TRUE(firstCallbackOk);
    EXPECT_TRUE(!secondCallbackOk);
    EXPECT_TRUE(thirdCallbackOk);
}


TEST(TestFuture, testIntegralPromiseThrowsOnDoubleSetValue) {
    Promise<int> promise;

    promise.setValue(123);
    EXPECT_THROW(promise.setValue(-3123), Solace::Exception);
}

TEST(TestFuture, testVoidPromiseThrowsOnDoubleSetValue) {
    Promise<void> promise;

    promise.setValue();
    EXPECT_THROW(promise.setValue(), Solace::Exception);
}

TEST(TestFuture, testIntegralPromiseThrowsOnDoubleSetError) {
    Promise<int> promise;

    promise.setError(makeFutureError(991, "testIntegralPromiseThrowsOnDoubleSetError-1"));
    EXPECT_THROW(promise.setError(makeFutureError(-187, "testIntegralPromiseThrowsOnDoubleSetError-2")),
                 Solace::Exception);
}

TEST(TestFuture, testVoidPromiseThrowsOnDoubleSetError) {
    Promise<void> promise;

    promise.setError(makeFutureError(993, "testVoidPromiseThrowsOnDoubleSetError"));
    EXPECT_THROW(promise.setError(makeFutureError(-186, "testVoidPromiseThrowsOnDoubleSetError")), Solace::Exception);
}


TEST(TestFuture, testCollectIntgralWhenAllSuccess) {
    const int bias = -338;
    const uint testGroupSize = 16;

    std::vector<Future<int>> futures;
    futures.reserve(testGroupSize);

    std::vector<Promise<int>> promises(testGroupSize);
    for (auto& promise : promises) {
        futures.push_back(promise.getFuture());
    }

    Future<std::vector<int>> futureArray = collect(futures);

    bool futureArrayReady = false;
    futureArray.then([&futureArrayReady](std::vector<int>&& values) {
        for (Array<int>::size_type index = 0; index < values.size(); ++index) {
            if (values[index] != (bias + static_cast<int>(index))) {
                return;
            }
        }

        futureArrayReady = true;
    });

    EXPECT_TRUE(!futureArrayReady);

    int index = 0;
    for (auto& promise : promises) {
        promise.setValue(bias + index);
        ++index;
    }

    EXPECT_TRUE(futureArrayReady);
}

TEST(TestFuture, testCollectVoidWhenAllSuccess) {
    const uint testGroupSize = 8;

    std::vector<Promise<void>> promises(testGroupSize);
    std::vector<Future<void>> futures;
    futures.reserve(testGroupSize);
    for (auto& promise : promises) {
        futures.push_back(promise.getFuture());
    }

    Future<void> futureArray = collect(futures);

    bool futureArrayReady = false;
    futureArray.then([&futureArrayReady]() {
        futureArrayReady = true;
    });

    EXPECT_TRUE(!futureArrayReady);

    for (auto& promise : promises) {
        promise.setValue();
    }

    EXPECT_TRUE(futureArrayReady);
}


TEST(TestFuture, testCollectIntegralWhenOneFailure) {
    const int bias = -338;
    const uint testGroupSize = 16;
    const int failEach = 12;

    std::vector<Promise<int>> promises(testGroupSize);
    std::vector<Future<int>> futures;
    futures.reserve(testGroupSize);
    for (auto& promise : promises) {
        futures.push_back(promise.getFuture());
    }

    Future<std::vector<int>> futureArray = collect(futures);

    bool futureArrayReady = false;
    bool futureArrayErrored = false;
    futureArray.then([&futureArrayReady](std::vector<int>&& values) {
        for (Array<int>::size_type index = 0; index < values.size(); ++index) {
            if (values[index] != (bias + static_cast<int>(index))) {
                return;
            }
        }

        futureArrayReady = true;
    })
    .onError([&futureArrayErrored](Error&& ) {
        futureArrayErrored = true;
    });

    EXPECT_TRUE(!futureArrayReady);
    EXPECT_TRUE(!futureArrayErrored);

    int index = 0;
    for (auto& promise : promises) {
        if ((index % failEach) == 0) {
            promise.setError(makeFutureError(321, "testCollectIntegralWhenOneFailure"));
        } else {
            promise.setValue(bias + index);
        }

        ++index;
    }

    EXPECT_TRUE(!futureArrayReady);
    EXPECT_TRUE(futureArrayErrored);
}


TEST(TestFuture, testCollectVoidWhenOneFailure) {
    const uint testGroupSize = 16;
    const uint failEach = 12;

    std::vector<Promise<void>> promises(testGroupSize);
    std::vector<Future<void>> futures;
    futures.reserve(testGroupSize);
    for (auto& promise : promises) {
        futures.push_back(promise.getFuture());
    }

    Future<void> futureArray = collect(futures);

    bool futureArrayReady = false;
    bool futureArrayErrored = false;
    futureArray
        .then([&futureArrayReady]() {
            futureArrayReady = true;
        })
        .onError([&futureArrayErrored](Error&& ) {
            futureArrayErrored = true;
        });


    EXPECT_TRUE(!futureArrayReady);
    EXPECT_TRUE(!futureArrayErrored);

    int index = 0;
    for (auto& promise : promises) {
        if ((index % failEach) == 0) {
            promise.setError(makeFutureError(3211, "testCollectVoidWhenOneFailure"));
        } else {
            promise.setValue();
        }

      ++index;
    }

    EXPECT_TRUE(!futureArrayReady);
    EXPECT_TRUE(futureArrayErrored);
}


TEST(TestFuture, testThenFiredDeletesClosure) {
    Promise<int> p;
    auto f = p.getFuture();

    EXPECT_EQ(0, PimitiveType::InstanceCount);
    f.then([t = PimitiveType(132)](int ) {

    });

    EXPECT_EQ(1, PimitiveType::InstanceCount);
    p.setValue(-17);
    EXPECT_EQ(0, PimitiveType::InstanceCount);
}


TEST(TestFuture, readyFuture) {
    bool thenFired = false;
    bool futureErrored = false;

    Future<PimitiveType> f = makeFuture(PimitiveType(817));

    EXPECT_EQ(1, PimitiveType::InstanceCount);

    f.then([&thenFired](PimitiveType&& value) {
        thenFired = (value.x == 817);
    })
    .onError([&futureErrored](Error&& ) {
        futureErrored = true;
    });


    EXPECT_TRUE(thenFired);
    EXPECT_TRUE(!futureErrored);
    EXPECT_EQ(0, PimitiveType::InstanceCount);
}
