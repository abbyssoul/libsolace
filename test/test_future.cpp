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

#include <gtest/gtest.h>

#include "mockTypes.hpp"

#include <thread>
#include <chrono>


using namespace Solace;

static int resolveVoidFunc() {
    return 99881;
}


class TestFuture : public ::testing::Test  {

protected:

    template<typename T>
    Future<T> makeOrphant() {
        auto p = Promise<T>();

        return p.getFuture();
    }

public:

    void setUp() {
	}

    void tearDown() {
	}
};

TEST_F(TestFuture, orphanIntegralFutureThrows) {

    auto f = makeOrphant<int>();

    int x = 9;

    EXPECT_THROW(f.then([&x](int i) { x += i;}),
                            Solace::Exception);
    EXPECT_THROW(f.onError([&x](Solace::Error&& ) { return x += 12; }),
                            Solace::Exception);
}

TEST_F(TestFuture, orphanVoidFutureThrows) {
    auto f = makeOrphant<void>();

    int x = 9;

    EXPECT_THROW(f.then([&x]() { x += 12;}),
                            Solace::Exception);
    EXPECT_THROW(f.onError([&x](Solace::Error&& ) { x += 12;}),
                            Solace::Exception);
}

TEST_F(TestFuture, destoyingIntFuturePropagatesViaThen) {
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


TEST_F(TestFuture, destoyingVoidFuturePropagatesViaThen) {
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

TEST_F(TestFuture, integralFutureIntegralContinuation) {
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

TEST_F(TestFuture, integralFutureVoidContinuation) {
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


TEST_F(TestFuture, voidFutureIntegralContinuation) {
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

TEST_F(TestFuture, voidFutureVoidContinuation) {
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



TEST_F(TestFuture, structFutureErrorContinuation) {
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

    p.setError(Error("testing", 789));

    EXPECT_TRUE(!resolved1);
    EXPECT_TRUE(resolved2);
    EXPECT_TRUE(resolved3);
}

TEST_F(TestFuture, integralFutureErrorContinuation) {
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

    p.setError(Error("testing", 4433));

    EXPECT_TRUE(!resolved1);
    EXPECT_TRUE(resolved2);
    EXPECT_TRUE(resolved3);
}


TEST_F(TestFuture, voidFutureErrorContinuation) {
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

    p.setError(Error("testing", 789));

    EXPECT_TRUE(!resolved1);
    EXPECT_TRUE(resolved2);
    EXPECT_TRUE(resolved3);
}


TEST_F(TestFuture, structFutureErrorResultErrors) {
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

        return Err(Error("testing", -8152));
    })
    .then([&resolved3](SimpleType&& ) {
        resolved3 = true;
    })
    .onError([&resolved4](Error&& e) {
        resolved4 = (e.value() == -8152);
    });

    p.setError(Error("testing", 5355));

    EXPECT_TRUE(!resolved1);
    EXPECT_TRUE(resolved2);
    EXPECT_TRUE(!resolved3);
    EXPECT_TRUE(resolved4);
}

TEST_F(TestFuture, integralFutureErrorResultErrors) {
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

        return Err(Error("testing", -8152));
    })
    .then([&resolved3](int ) {
        resolved3 = true;
    })
    .onError([&resolved4](Error&& e) {
        resolved4 = (e.value() == -8152);
    });

    p.setError(Error("testing", 5355));

    EXPECT_TRUE(!resolved1);
    EXPECT_TRUE(resolved2);
    EXPECT_TRUE(!resolved3);
    EXPECT_TRUE(resolved4);
}

TEST_F(TestFuture, voidFutureErrorResultErrors) {
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

        return Err(Error("testing", -4424));
    })
    .then([&resolved3]() {
        resolved3 = true;
    })
    .onError([&resolved4](Error&& e) {
        resolved4 = (e.value() == -4424);
    });

    p.setError(Error("testing", 7744));

    EXPECT_TRUE(!resolved1);
    EXPECT_TRUE(resolved2);
    EXPECT_TRUE(!resolved3);
    EXPECT_TRUE(resolved4);
}


//------------------------------------------------------------------------------------------------------------------
// Tests for continuations returning Futures
//------------------------------------------------------------------------------------------------------------------

TEST_F(TestFuture, integralFutureIntegralFutureContinuation) {
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

TEST_F(TestFuture, voidFutureIntegralFutureContinuation) {
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

TEST_F(TestFuture, integralFutureVoidFutureContinuation) {
    bool resolved1 = false;
    bool resolved2 = false;

    auto p1 = Promise<int>();
    auto f = p1.getFuture();

    auto p2 = Promise<void>();

    f.then([&resolved1, &p2](int x) {
        resolved1 = (x = 887);

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

TEST_F(TestFuture, voidFutureVoidFutureContinuation) {
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



TEST_F(TestFuture, integralFutureIntegralFutureErrorsContinuation) {
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

    p2.setError(Error("Testing testing", 123));
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(!resolved2);
    EXPECT_TRUE(resolved3);
}

TEST_F(TestFuture, voidFutureIntegralFutureErrorsContinuation) {
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

    p2.setError(Error("Testing testing", 321));
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(!resolved2);
    EXPECT_TRUE(resolved3);
}

TEST_F(TestFuture, integralFutureVoidFutureErrorsContinuation) {
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

    p2.setError(Error("Testing testing", -231));
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(!resolved2);
    EXPECT_TRUE(resolved3);
}

TEST_F(TestFuture, voidFutureVoidFutureErrorsContinuation) {
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

    p2.setError(Error("Testing testing", 543));
    EXPECT_TRUE(resolved1);
    EXPECT_TRUE(!resolved2);
    EXPECT_TRUE(resolved3);
}


//------------------------------------------------------------------------------------------------------------------
// Tests for continuations returning Results
//------------------------------------------------------------------------------------------------------------------

TEST_F(TestFuture, integralFutureIntegralResultContinuation) {
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

TEST_F(TestFuture, voidFutureIntegralResultContinuation) {
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

TEST_F(TestFuture, integralFutureVoidResultContinuation) {
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

TEST_F(TestFuture, voidFutureVoidResultContinuation) {
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



TEST_F(TestFuture, integralFutureIntegralResultErrorsContinuation) {
    bool resolved1 = false;
    bool resolved2 = false;
    bool resolved3 = false;

    auto p1 = Promise<char>();
    auto f = p1.getFuture();

    f.then([&resolved1](char x) -> Result<int, Error> {
        resolved1 = (x == 'n');

        return Err(Error("test", -525));
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

TEST_F(TestFuture, voidFutureIntegralResultErrorsContinuation) {
    bool resolved1 = false;
    bool resolved2 = false;
    bool resolved3 = false;

    auto p1 = Promise<void>();
    auto f = p1.getFuture();

    f.then([&resolved1]() -> Result<int, Error> {
        resolved1 = true;

        return Err(Error("test", -525));
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

TEST_F(TestFuture, integralFutureVoidResultErrorsContinuation) {
    bool resolved1 = false;
    bool resolved2 = false;
    bool resolved3 = false;

    auto p1 = Promise<char>();
    auto f = p1.getFuture();

    f.then([&resolved1](char x) -> Result<void, Error> {
        resolved1 = (x == 'n');

        return Err(Error("test", -525));
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

TEST_F(TestFuture, voidFutureVoidResultErrorsContinuation) {
    bool resolved1 = false;
    bool resolved2 = false;
    bool resolved3 = false;

    Promise<void> p1;
    auto f = p1.getFuture();

    f.then([&resolved1](void) -> Result<void, Error> {
        resolved1 = true;

        return Err(Error("test", 95546));
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



TEST_F(TestFuture, testThenWithStandaloneFunction) {
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









TEST_F(TestFuture, testThenResultContinuation) {
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


TEST_F(TestFuture, testThenVoidResultContinuation) {
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



TEST_F(TestFuture, testThenFutureContinuation) {
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


TEST_F(TestFuture, testOnErrorHandler) {
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


    p1.setError(Solace::Error("Test error"));
    EXPECT_TRUE(!firstCallbackOk);
    EXPECT_TRUE(secondCallbackOk);
}

TEST_F(TestFuture, testOnErrorRestoresTheChain) {
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


    p1.setError(Solace::Error("Test error"));
    EXPECT_TRUE(!firstCallbackOk);
    EXPECT_TRUE(secondCallbackOk);
    EXPECT_TRUE(thirdCallbackOk);
}

TEST_F(TestFuture, testOnErrorSkippedOnSuccess) {
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


TEST_F(TestFuture, testIntegralPromiseThrowsOnDoubleSetValue) {
    Promise<int> promise;

    promise.setValue(123);
    EXPECT_THROW(promise.setValue(-3123), Solace::Exception);
}

TEST_F(TestFuture, testVoidPromiseThrowsOnDoubleSetValue) {
    Promise<void> promise;

    promise.setValue();
    EXPECT_THROW(promise.setValue(), Solace::Exception);
}

TEST_F(TestFuture, testIntegralPromiseThrowsOnDoubleSetError) {
    Promise<int> promise;

    promise.setError(Error("testError", 991));
    EXPECT_THROW(promise.setError(Error("testError", -187)), Solace::Exception);
}

TEST_F(TestFuture, testVoidPromiseThrowsOnDoubleSetError) {
    Promise<void> promise;

    promise.setError(Error("testError", 991));
    EXPECT_THROW(promise.setError(Error("testError", -187)), Solace::Exception);
}


TEST_F(TestFuture, testCollectIntgralWhenAllSuccess) {
    const int bias = -338;
    const uint testGroupSize = 16;

    Array<Promise<int>> promises(testGroupSize);
    std::vector<Future<int>> futures;
    futures.reserve(testGroupSize);
    promises.forEach([&futures](Promise<int>& promise) {
        futures.push_back(promise.getFuture());
    });

    Future<Array<int>> futureArray = collect(futures);

    bool futureArrayReady = false;
    futureArray.then([&futureArrayReady](Array<int>&& values) {
        for (Array<int>::size_type index = 0; index < values.size(); ++index) {
            if (values[index] != (bias + static_cast<int>(index))) {
                return;
            }
        }

        futureArrayReady = true;
    });

    EXPECT_TRUE(!futureArrayReady);

    promises.forEachIndexed([](int index, Promise<int>& promise) {
        promise.setValue(bias + index);
    });

    EXPECT_TRUE(futureArrayReady);
}

TEST_F(TestFuture, testCollectVoidWhenAllSuccess) {
    const uint testGroupSize = 8;

    Array<Promise<void>> promises(testGroupSize);
    std::vector<Future<void>> futures;
    futures.reserve(testGroupSize);
    promises.forEach([&futures](Promise<void>& promise) {
        futures.push_back(promise.getFuture());
    });

    Future<void> futureArray = collect(futures);

    bool futureArrayReady = false;
    futureArray.then([&futureArrayReady]() {
        futureArrayReady = true;
    });

    EXPECT_TRUE(!futureArrayReady);

    promises.forEach([](Promise<void>& promise) {
        promise.setValue();
    });

    EXPECT_TRUE(futureArrayReady);
}


TEST_F(TestFuture, testCollectIntegralWhenOneFailure) {
    const int bias = -338;
    const uint testGroupSize = 16;
    const int failEach = 12;

    Array<Promise<int>> promises(testGroupSize);
    std::vector<Future<int>> futures;
    futures.reserve(testGroupSize);
    promises.forEach([&futures](Promise<int>& promise) {
        futures.push_back(promise.getFuture());
    });

    Future<Array<int>> futureArray = collect(futures);

    bool futureArrayReady = false;
    bool futureArrayErrored = false;
    futureArray
    .then([&futureArrayReady](Array<int>&& values) {
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

    promises.forEachIndexed([](int index, Promise<int>& promise) {
        if ((index % failEach) == 0) {
            promise.setError(Error("failed", 321));
        } else {
            promise.setValue(bias + index);
        }
    });

    EXPECT_TRUE(!futureArrayReady);
    EXPECT_TRUE(futureArrayErrored);
}


TEST_F(TestFuture, testCollectVoidWhenOneFailure) {
    const uint testGroupSize = 16;
    const uint failEach = 12;

    Array<Promise<void>> promises(testGroupSize);
    std::vector<Future<void>> futures;
    futures.reserve(testGroupSize);
    promises.forEach([&futures](Promise<void>& promise) {
        futures.push_back(promise.getFuture());
    });

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

    promises.forEachIndexed([failEach](auto index, Promise<void>& promise) {
        if ((index % failEach) == 0) {
            promise.setError(Error("failed", 321));
        } else {
            promise.setValue();
        }
    });

    EXPECT_TRUE(!futureArrayReady);
    EXPECT_TRUE(futureArrayErrored);
}

TEST_F(TestFuture, testThenFiredDeletesClosure) {
    Promise<int> p;
    auto f = p.getFuture();

    EXPECT_EQ(0, PimitiveType::InstanceCount);
    f.then([t = PimitiveType(132)](int ) {

    });

    EXPECT_EQ(1, PimitiveType::InstanceCount);
    p.setValue(-17);
    EXPECT_EQ(0, PimitiveType::InstanceCount);
}


TEST_F(TestFuture, readyFuture) {
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
