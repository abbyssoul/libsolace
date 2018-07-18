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

#include <cppunit/extensions/HelperMacros.h>

#include "mockTypes.hpp"

#include <thread>
#include <chrono>


using namespace Solace;


static int resolveVoidFunc() {
    return 99881;
}


class TestFuture : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestFuture);
        CPPUNIT_TEST(orphanIntegralFutureThrows);
        CPPUNIT_TEST(orphanVoidFutureThrows);

        CPPUNIT_TEST(destoyingIntFuturePropagatesViaThen);
        CPPUNIT_TEST(destoyingVoidFuturePropagatesViaThen);

        CPPUNIT_TEST(testThenWithStandaloneFunction);

        CPPUNIT_TEST(integralFutureIntegralContinuation);
        CPPUNIT_TEST(voidFutureIntegralContinuation);
        CPPUNIT_TEST(integralFutureVoidContinuation);
        CPPUNIT_TEST(voidFutureVoidContinuation);

        CPPUNIT_TEST(structFutureErrorContinuation);
        CPPUNIT_TEST(integralFutureErrorContinuation);
        CPPUNIT_TEST(voidFutureErrorContinuation);

        CPPUNIT_TEST(structFutureErrorResultErrors);
        CPPUNIT_TEST(integralFutureErrorResultErrors);
        CPPUNIT_TEST(voidFutureErrorResultErrors);


        CPPUNIT_TEST(integralFutureIntegralFutureContinuation);
        CPPUNIT_TEST(voidFutureIntegralFutureContinuation);
        CPPUNIT_TEST(integralFutureVoidFutureContinuation);
        CPPUNIT_TEST(voidFutureVoidFutureContinuation);

        CPPUNIT_TEST(integralFutureIntegralFutureErrorsContinuation);
        CPPUNIT_TEST(voidFutureIntegralFutureErrorsContinuation);
        CPPUNIT_TEST(integralFutureVoidFutureErrorsContinuation);
        CPPUNIT_TEST(voidFutureVoidFutureErrorsContinuation);


        CPPUNIT_TEST(integralFutureIntegralResultContinuation);
        CPPUNIT_TEST(voidFutureIntegralResultContinuation);
        CPPUNIT_TEST(integralFutureVoidResultContinuation);
        CPPUNIT_TEST(voidFutureVoidResultContinuation);

        CPPUNIT_TEST(integralFutureIntegralResultErrorsContinuation);
        CPPUNIT_TEST(voidFutureIntegralResultErrorsContinuation);
        CPPUNIT_TEST(integralFutureVoidResultErrorsContinuation);
        CPPUNIT_TEST(voidFutureVoidResultErrorsContinuation);



        CPPUNIT_TEST(testThenResultContinuation);
        CPPUNIT_TEST(testThenVoidResultContinuation);
        CPPUNIT_TEST(testThenFutureContinuation);

        CPPUNIT_TEST(testOnErrorHandler);
        CPPUNIT_TEST(testOnErrorSkippedOnSuccess);
        CPPUNIT_TEST(testOnErrorRestoresTheChain);


        CPPUNIT_TEST(testIntegralPromiseThrowsOnDoubleSetValue);
        CPPUNIT_TEST(testVoidPromiseThrowsOnDoubleSetValue);
        CPPUNIT_TEST(testIntegralPromiseThrowsOnDoubleSetError);
        CPPUNIT_TEST(testVoidPromiseThrowsOnDoubleSetError);

        CPPUNIT_TEST(testCollectIntgralWhenAllSuccess);
        CPPUNIT_TEST(testCollectVoidWhenAllSuccess);

        CPPUNIT_TEST(testCollectIntegralWhenOneFailure);
        CPPUNIT_TEST(testCollectVoidWhenOneFailure);

        CPPUNIT_TEST(testThenFiredDeletesClosure);

        CPPUNIT_TEST(readyFuture);

    CPPUNIT_TEST_SUITE_END();

protected:

    template<typename T>
    Future<T> makeOrphant() {
        auto p = Promise<T>();

        return p.getFuture();
    }


public:


    void orphanIntegralFutureThrows() {

        auto f = makeOrphant<int>();

        int x = 9;

        CPPUNIT_ASSERT_THROW(f.then([&x](int i) { x += i;}),
                             Solace::Exception);
        CPPUNIT_ASSERT_THROW(f.onError([&x](Solace::Error&& ) { return x += 12; }),
                             Solace::Exception);
    }

    void orphanVoidFutureThrows() {
        auto f = makeOrphant<void>();

        int x = 9;

        CPPUNIT_ASSERT_THROW(f.then([&x]() { x += 12;}),
                             Solace::Exception);
        CPPUNIT_ASSERT_THROW(f.onError([&x](Solace::Error&& ) { x += 12;}),
                             Solace::Exception);
    }

    void destoyingIntFuturePropagatesViaThen() {
        Promise<int> p;
        bool resolved1 = false;


        {
            p.getFuture().then([&resolved1](int) {
                resolved1 = true;

                return;
            });
        }

        p.setValue(321);

        CPPUNIT_ASSERT(resolved1);
    }


    void destoyingVoidFuturePropagatesViaThen() {
        Promise<void> p;
        bool resolved1 = false;


        {
            p.getFuture().then([&resolved1]() {
                resolved1 = true;
            });
        }

        p.setValue();

        CPPUNIT_ASSERT(resolved1);
    }

    void integralFutureIntegralContinuation() {
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

        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(resolved2);
    }

    void integralFutureVoidContinuation() {
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

        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(resolved2);
    }


    void voidFutureIntegralContinuation() {
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

        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(resolved2);
    }

    void voidFutureVoidContinuation() {
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

        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(resolved2);
    }



    void structFutureErrorContinuation() {
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

        CPPUNIT_ASSERT(!resolved1);
        CPPUNIT_ASSERT(resolved2);
        CPPUNIT_ASSERT(resolved3);
    }

    void integralFutureErrorContinuation() {
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

        CPPUNIT_ASSERT(!resolved1);
        CPPUNIT_ASSERT(resolved2);
        CPPUNIT_ASSERT(resolved3);
    }


    void voidFutureErrorContinuation() {
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

        CPPUNIT_ASSERT(!resolved1);
        CPPUNIT_ASSERT(resolved2);
        CPPUNIT_ASSERT(resolved3);
    }


    void structFutureErrorResultErrors() {
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

        CPPUNIT_ASSERT(!resolved1);
        CPPUNIT_ASSERT(resolved2);
        CPPUNIT_ASSERT(!resolved3);
        CPPUNIT_ASSERT(resolved4);
    }

    void integralFutureErrorResultErrors() {
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

        CPPUNIT_ASSERT(!resolved1);
        CPPUNIT_ASSERT(resolved2);
        CPPUNIT_ASSERT(!resolved3);
        CPPUNIT_ASSERT(resolved4);
    }

    void voidFutureErrorResultErrors() {
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

        CPPUNIT_ASSERT(!resolved1);
        CPPUNIT_ASSERT(resolved2);
        CPPUNIT_ASSERT(!resolved3);
        CPPUNIT_ASSERT(resolved4);
    }


    //------------------------------------------------------------------------------------------------------------------
    // Tests for continuations returning Futures
    //------------------------------------------------------------------------------------------------------------------

    void integralFutureIntegralFutureContinuation() {
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
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(!resolved2);

        p2.setValue('y');
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(resolved2);
    }

    void voidFutureIntegralFutureContinuation() {
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
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(!resolved2);

        p2.setValue(-9817);
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(resolved2);
    }

    void integralFutureVoidFutureContinuation() {
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
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(!resolved2);

        p2.setValue();
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(resolved2);
    }

    void voidFutureVoidFutureContinuation() {
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
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(!resolved2);

        p2.setValue();
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(resolved2);
    }



    void integralFutureIntegralFutureErrorsContinuation() {
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
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(!resolved2);
        CPPUNIT_ASSERT(!resolved3);

        p2.setError(Error("Testing testing", 123));
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(!resolved2);
        CPPUNIT_ASSERT(resolved3);
    }

    void voidFutureIntegralFutureErrorsContinuation() {
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
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(!resolved2);
        CPPUNIT_ASSERT(!resolved3);

        p2.setError(Error("Testing testing", 321));
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(!resolved2);
        CPPUNIT_ASSERT(resolved3);
    }

    void integralFutureVoidFutureErrorsContinuation() {
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
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(!resolved2);
        CPPUNIT_ASSERT(!resolved3);

        p2.setError(Error("Testing testing", -231));
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(!resolved2);
        CPPUNIT_ASSERT(resolved3);
    }

    void voidFutureVoidFutureErrorsContinuation() {
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
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(!resolved2);
        CPPUNIT_ASSERT(!resolved3);

        p2.setError(Error("Testing testing", 543));
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(!resolved2);
        CPPUNIT_ASSERT(resolved3);
    }


    //------------------------------------------------------------------------------------------------------------------
    // Tests for continuations returning Results
    //------------------------------------------------------------------------------------------------------------------

    void integralFutureIntegralResultContinuation() {
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
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(resolved2);
    }

    void voidFutureIntegralResultContinuation() {
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
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(resolved2);
    }

    void integralFutureVoidResultContinuation() {
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
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(resolved2);
    }

    void voidFutureVoidResultContinuation() {
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
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(resolved2);
    }



    void integralFutureIntegralResultErrorsContinuation() {
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
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(!resolved2);
        CPPUNIT_ASSERT(resolved3);
    }

    void voidFutureIntegralResultErrorsContinuation() {
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
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(!resolved2);
        CPPUNIT_ASSERT(resolved3);
    }

    void integralFutureVoidResultErrorsContinuation() {
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
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(!resolved2);
        CPPUNIT_ASSERT(resolved3);
    }

    void voidFutureVoidResultErrorsContinuation() {
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
        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(!resolved2);
        CPPUNIT_ASSERT(resolved3);
    }



    void testThenWithStandaloneFunction() {
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

        CPPUNIT_ASSERT(resolved1);
        CPPUNIT_ASSERT(!errored1);
    }









    void testThenResultContinuation() {
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
            CPPUNIT_ASSERT_EQUAL('Y', c);

            resolved = true;
        });

        p.setValue(120);

        CPPUNIT_ASSERT(firstCallbackOk);
        CPPUNIT_ASSERT(secondCallbackOk);
        CPPUNIT_ASSERT(resolved);
    }


    void testThenVoidResultContinuation() {
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

        CPPUNIT_ASSERT(firstCallbackOk);
        CPPUNIT_ASSERT(secondCallbackOk);
        CPPUNIT_ASSERT(resolved);
    }



    void testThenFutureContinuation() {
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
        CPPUNIT_ASSERT(firstCallbackOk);
        CPPUNIT_ASSERT(!secondCallbackOk);

        p2.setValue(SimpleType(60, 1, 3));
        CPPUNIT_ASSERT(firstCallbackOk);
        CPPUNIT_ASSERT(secondCallbackOk);
    }


    void testOnErrorHandler() {
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
        CPPUNIT_ASSERT(!firstCallbackOk);
        CPPUNIT_ASSERT(secondCallbackOk);
    }

    void testOnErrorRestoresTheChain() {
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
        CPPUNIT_ASSERT(!firstCallbackOk);
        CPPUNIT_ASSERT(secondCallbackOk);
        CPPUNIT_ASSERT(thirdCallbackOk);
    }

    void testOnErrorSkippedOnSuccess() {
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
        CPPUNIT_ASSERT(firstCallbackOk);
        CPPUNIT_ASSERT(!secondCallbackOk);
        CPPUNIT_ASSERT(thirdCallbackOk);
    }


    void testIntegralPromiseThrowsOnDoubleSetValue() {
        Promise<int> promise;

        promise.setValue(123);
        CPPUNIT_ASSERT_THROW(promise.setValue(-3123), Solace::Exception);
    }

    void testVoidPromiseThrowsOnDoubleSetValue() {
        Promise<void> promise;

        promise.setValue();
        CPPUNIT_ASSERT_THROW(promise.setValue(), Solace::Exception);
    }

    void testIntegralPromiseThrowsOnDoubleSetError() {
        Promise<int> promise;

        promise.setError(Error("testError", 991));
        CPPUNIT_ASSERT_THROW(promise.setError(Error("testError", -187)), Solace::Exception);
    }

    void testVoidPromiseThrowsOnDoubleSetError() {
        Promise<void> promise;

        promise.setError(Error("testError", 991));
        CPPUNIT_ASSERT_THROW(promise.setError(Error("testError", -187)), Solace::Exception);
    }


    void testCollectIntgralWhenAllSuccess() {
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

        CPPUNIT_ASSERT(!futureArrayReady);

        promises.forEachIndexed([](int index, Promise<int>& promise) {
            promise.setValue(bias + index);
        });

        CPPUNIT_ASSERT(futureArrayReady);
    }

    void testCollectVoidWhenAllSuccess() {
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

        CPPUNIT_ASSERT(!futureArrayReady);

        promises.forEach([](Promise<void>& promise) {
            promise.setValue();
        });

        CPPUNIT_ASSERT(futureArrayReady);
    }


    void testCollectIntegralWhenOneFailure() {
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

        CPPUNIT_ASSERT(!futureArrayReady);
        CPPUNIT_ASSERT(!futureArrayErrored);

        promises.forEachIndexed([](int index, Promise<int>& promise) {
            if ((index % failEach) == 0) {
                promise.setError(Error("failed", 321));
            } else {
                promise.setValue(bias + index);
            }
        });

        CPPUNIT_ASSERT(!futureArrayReady);
        CPPUNIT_ASSERT(futureArrayErrored);
    }


    void testCollectVoidWhenOneFailure() {
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

        CPPUNIT_ASSERT(!futureArrayReady);
        CPPUNIT_ASSERT(!futureArrayErrored);

        promises.forEachIndexed([failEach](auto index, Promise<void>& promise) {
            if ((index % failEach) == 0) {
                promise.setError(Error("failed", 321));
            } else {
                promise.setValue();
            }
        });

        CPPUNIT_ASSERT(!futureArrayReady);
        CPPUNIT_ASSERT(futureArrayErrored);
    }

    void testThenFiredDeletesClosure() {
        Promise<int> p;
        auto f = p.getFuture();

        CPPUNIT_ASSERT_EQUAL(0, PimitiveType::InstanceCount);
        f.then([t = PimitiveType(132)](int ) {

        });

        CPPUNIT_ASSERT_EQUAL(1, PimitiveType::InstanceCount);
        p.setValue(-17);
        CPPUNIT_ASSERT_EQUAL(0, PimitiveType::InstanceCount);
    }


    void readyFuture() {
        bool thenFired = false;
        bool futureErrored = false;

        Future<PimitiveType> f = makeFuture(PimitiveType(817));

        CPPUNIT_ASSERT_EQUAL(1, PimitiveType::InstanceCount);

        f.then([&thenFired](PimitiveType&& value) {
            thenFired = (value.x == 817);
        })
        .onError([&futureErrored](Error&& ) {
            futureErrored = true;
        });


        CPPUNIT_ASSERT(thenFired);
        CPPUNIT_ASSERT(!futureErrored);
        CPPUNIT_ASSERT_EQUAL(0, PimitiveType::InstanceCount);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestFuture);
