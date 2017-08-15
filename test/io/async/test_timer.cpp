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
 * @file: test/io/async/test_timer.cpp
 * @author: soultaker
 *
 * Created on: 10/10/2016
*******************************************************************************/
#include <solace/io/async/timer.hpp>  // Class being tested

#include <cppunit/extensions/HelperMacros.h>

#include <thread>
#include <chrono>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>


using namespace Solace;
using namespace Solace::IO;


class TestAsyncTimer: public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestAsyncTimer);
        CPPUNIT_TEST(testConstructionTimeout);
        CPPUNIT_TEST(testTimeout);
        CPPUNIT_TEST(testPeriodicTimeout);
        CPPUNIT_TEST(testPeriodicTimeoutCancelable);
        CPPUNIT_TEST(testTimeoutCanceledAndRescheduled);
        CPPUNIT_TEST(testGetTimeout);
    CPPUNIT_TEST_SUITE_END();

protected:

public:

    void testConstructionTimeout() {
        auto iocontext = async::EventLoop(2);
        bool eventWasCalled = false;

        async::Timer timer(iocontext, std::chrono::milliseconds(120));

        timer.asyncWait().then([&eventWasCalled](int64_t) {
            eventWasCalled = true;
        });

        CPPUNIT_ASSERT(!eventWasCalled);

        std::thread watchdog([&iocontext]() {
            using namespace std::chrono_literals;

            std::this_thread::sleep_for(400ms);
            iocontext.stop();
        });

        CPPUNIT_ASSERT(!eventWasCalled);

        // Should block untill event is triggered
        iocontext.run();
        watchdog.join();

        CPPUNIT_ASSERT(eventWasCalled);
    }


    void testTimeout() {
        auto iocontext = async::EventLoop(2);
        bool eventWasCalled = false;

        async::Timer timer(iocontext);
        timer.setTimeout(std::chrono::milliseconds(120))
                .asyncWait()
                .then([&eventWasCalled](int64_t ) {
            eventWasCalled = true;
        });

        CPPUNIT_ASSERT(!eventWasCalled);

        std::thread watchdog([&iocontext]() {
            using namespace std::chrono_literals;

            std::this_thread::sleep_for(400ms);
            iocontext.stop();
        });

        CPPUNIT_ASSERT(!eventWasCalled);

        // Should block untill event is triggered
        iocontext.run();
        watchdog.join();

        CPPUNIT_ASSERT(eventWasCalled);
    }

    void testPeriodicTimeout() {
        auto iocontext = async::EventLoop(2);
        int nbTimesCalled = 0;

        async::Timer timer(iocontext);
        timer.setTimeoutInterval(std::chrono::milliseconds(10), std::chrono::milliseconds(25))
                .asyncWait()
                .then([&nbTimesCalled, &iocontext](int64_t numberOfExpirations) {

            nbTimesCalled += numberOfExpirations;
            if (nbTimesCalled >= 4) {
                iocontext.stop();
            }
        });

        CPPUNIT_ASSERT_EQUAL(0, nbTimesCalled);

        std::thread watchdog([&iocontext]() {
            using namespace std::chrono_literals;

            std::this_thread::sleep_for(200ms);
            iocontext.stop();
        });

        // Should block untill event is triggered
        bool runFailes = false;
        try {
            iocontext.run();
        } catch (const std::exception& ) {
            runFailes = true;
        }

        watchdog.join();

        CPPUNIT_ASSERT_EQUAL(4, nbTimesCalled);
        CPPUNIT_ASSERT(!runFailes);
    }


    void testPeriodicTimeoutCancelable() {
        auto iocontext = async::EventLoop(2);
        int nbTimesCalled = 0;
        int nbTimesCalledWhenCanceled = 0;

        async::Timer timer(iocontext);
        timer.setTimeoutInterval(std::chrono::milliseconds(30), std::chrono::milliseconds(60))
                .asyncWait()
                .then([&nbTimesCalled, &iocontext](int64_t numberOfExpirations) {

            nbTimesCalled += numberOfExpirations;
        });

        CPPUNIT_ASSERT_EQUAL(0, nbTimesCalled);

        std::thread watchdog([&iocontext]() {
            using namespace std::chrono_literals;

            std::this_thread::sleep_for(300ms);
            iocontext.stop();
        });

        std::thread canceler([&iocontext, &timer, &nbTimesCalledWhenCanceled, &nbTimesCalled]() {
            using namespace std::chrono_literals;

            std::this_thread::sleep_for(120ms);
            timer.cancel();
            nbTimesCalledWhenCanceled = nbTimesCalled;
        });

        // Should block untill event is triggered
        bool runFailes = false;
        try {
            iocontext.run();
        } catch (const std::exception& ) {
            runFailes = true;
        }

        watchdog.join();
        canceler.join();

        CPPUNIT_ASSERT_EQUAL(nbTimesCalledWhenCanceled, nbTimesCalled);
        CPPUNIT_ASSERT(!runFailes);
    }


    void testTimeoutCanceledAndRescheduled() {
        auto iocontext = async::EventLoop(2);
        int nbTimesCalled = 0;
        int nbTimesCalledWhenCanceled = 0;

        async::Timer timer(iocontext);
        timer.setTimeoutInterval(std::chrono::milliseconds(10), std::chrono::milliseconds(25))
                .asyncWait()
                .then([&nbTimesCalled, &iocontext](int64_t numberOfExpirations) {

            nbTimesCalled += numberOfExpirations;
        });

        CPPUNIT_ASSERT_EQUAL(0, nbTimesCalled);

        std::thread watchdog([&iocontext]() {
            using namespace std::chrono_literals;

            std::this_thread::sleep_for(300ms);
            iocontext.stop();
        });

        std::thread canceler([&iocontext, &timer, &nbTimesCalledWhenCanceled, &nbTimesCalled]() {
            using namespace std::chrono_literals;

            try {
                std::this_thread::sleep_for(120ms);
                timer.cancel();
                nbTimesCalledWhenCanceled = nbTimesCalled;

                std::this_thread::sleep_for(30ms);
                timer.setTimeout(std::chrono::milliseconds(10))
                        .asyncWait()
                        .then([&nbTimesCalled, &iocontext](int64_t numberOfExpirations) {

                    nbTimesCalled += numberOfExpirations;
                });
            } catch (const std::exception& ex) {
                CPPUNIT_FAIL("Unhandeled expection ");
            }
        });

        // Should block untill event is triggered
        bool runFailes = false;
        try {
            iocontext.run();
        } catch (const std::exception& ) {
            runFailes = true;
        }
        canceler.join();

        watchdog.join();
        CPPUNIT_ASSERT_EQUAL(nbTimesCalledWhenCanceled + 1, nbTimesCalled);
        CPPUNIT_ASSERT(!runFailes);
    }


    void testGetTimeout() {
        auto iocontext = async::EventLoop(2);
        int nbTimesCalled = 0;

        async::Timer timer(iocontext);
        timer.setTimeout(std::chrono::milliseconds(20))
                .asyncWait()
                .then([&nbTimesCalled, &iocontext, &timer](int64_t numberOfExpirations) {

            nbTimesCalled += numberOfExpirations;

            const auto timeout = timer.getTimeout();
            // Hopefully it's not flaky!
            CPPUNIT_ASSERT(std::abs(timeout.count()) < 5L);
        });

        CPPUNIT_ASSERT_EQUAL(0, nbTimesCalled);

        const auto timeout = timer.getTimeout();
        CPPUNIT_ASSERT(std::abs(timeout.count() - std::chrono::milliseconds(20).count()) < 20L);

        std::thread watchdog([&iocontext]() {
            using namespace std::chrono_literals;

            std::this_thread::sleep_for(300ms);
            iocontext.stop();
        });

        // Should block untill event is triggered
        iocontext.run();
        watchdog.join();

        CPPUNIT_ASSERT_EQUAL(1, nbTimesCalled);
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestAsyncTimer);
