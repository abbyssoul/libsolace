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
#include <solace/io/async/future.hpp>  // Class being tested

#include <cppunit/extensions/HelperMacros.h>

#include "../../mockTypes.hpp"

#include <thread>
#include <chrono>


using namespace Solace;
using namespace Solace::IO;
using namespace Solace::IO::async;


class TestFuture : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestFuture);
        CPPUNIT_TEST(orphanFutureThrows);

        CPPUNIT_TEST(testThenVoidContinuation);
        CPPUNIT_TEST(testVoidThenContinuation);
        CPPUNIT_TEST(testThenValueUnwrapping);
        CPPUNIT_TEST(testThenResultContinuation);
        CPPUNIT_TEST(testThenVoidResultContinuation);
        CPPUNIT_TEST(testThenFutureContinuation);

        CPPUNIT_TEST(testOnErrorHandler);
        CPPUNIT_TEST(testOnErrorSkippedOnSuccess);
        CPPUNIT_TEST(testOnErrorRestoresTheChain);

        CPPUNIT_TEST(voidFutureThenVoidFuture);
    CPPUNIT_TEST_SUITE_END();

protected:
public:


    void orphanFutureThrows() {


        auto makeOrphanFuture = [] () {
            auto p = Promise<int>();

            return p.getFuture();
        };

        auto f = makeOrphanFuture();

        int x = 9;

        CPPUNIT_ASSERT_THROW(f.then([&x](int i) {
            x += i;
        }),
                             Solace::Exception);
    }

    void testThenVoidContinuation() {
        auto p = Promise<int>();
        auto f = p.getFuture();
        bool resolved = false;

        f.then([&resolved](int x) {
            resolved = (x == 120);
        });

        p.setValue(120);

        CPPUNIT_ASSERT(resolved);
    }

    void testVoidThenContinuation() {
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

    void testThenValueUnwrapping() {
        auto p = Promise<int>();
        auto f = p.getFuture();

        bool firstCallbackOk = false;
        bool secondCallbackOk = false;

        f.then([&firstCallbackOk](int x) {
            firstCallbackOk = (x == 120);

            return x / 2;
        })
        .then([&secondCallbackOk](int c) {
            secondCallbackOk = (c == 60);

            return;
        });

        p.setValue(120);

        CPPUNIT_ASSERT(firstCallbackOk);
        CPPUNIT_ASSERT(secondCallbackOk);
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
        }).onError([&secondCallbackOk](Error&& e) -> Result<void, Error> {
            if (e)
                secondCallbackOk = true;
            return Ok();
        }).then([&thirdCallbackOk]() {
            thirdCallbackOk = true;
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
        }).onError([&secondCallbackOk](Error&& e) -> Result<void, Error> {

            if (e)
                secondCallbackOk = true;

            return Ok();
        }).then([&thirdCallbackOk]() {
            thirdCallbackOk = true;
        });


        p1.setValue(120);
        CPPUNIT_ASSERT(firstCallbackOk);
        CPPUNIT_ASSERT(!secondCallbackOk);
        CPPUNIT_ASSERT(thirdCallbackOk);
    }


    void voidFutureThenVoidFuture() {
        Promise<void> p1;
        Promise<void> p2;
        auto f1 = p1.getFuture();

        bool firstCallbackOk = false;
        bool secondCallbackOk = false;
        bool thirdCallbackOk = false;

        auto cont = [&p2, &secondCallbackOk]() {
            secondCallbackOk = true;
            return p2.getFuture();
        };

        f1.then([&firstCallbackOk, &cont]() -> Future<void> {
            firstCallbackOk = true;

            return cont();
        }).then([&thirdCallbackOk]() {
            thirdCallbackOk = true;
        });


        p1.setValue();
        CPPUNIT_ASSERT(firstCallbackOk);
        CPPUNIT_ASSERT(secondCallbackOk);
        CPPUNIT_ASSERT(!thirdCallbackOk);

        p2.setValue();
        CPPUNIT_ASSERT(firstCallbackOk);
        CPPUNIT_ASSERT(secondCallbackOk);
        CPPUNIT_ASSERT(thirdCallbackOk);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestFuture);
