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
 * @file: test/io/async/test_signalSet.cpp
 * @author: soultaker
 *
 * Created on: 10/10/2016
*******************************************************************************/
#include <solace/io/async/signalSet.hpp>  // Class being tested

#include <cppunit/extensions/HelperMacros.h>

#include <thread>
#include <chrono>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>


using namespace Solace;
using namespace Solace::IO;


class TestAsyncSignalSet : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestAsyncSignalSet);
        CPPUNIT_TEST(testEPoll_subscription);
        CPPUNIT_TEST(testEPoll_subscription2);
        CPPUNIT_TEST(testPoll_subscription);
        CPPUNIT_TEST(testPoll_subscription2);
        CPPUNIT_TEST(testSubscriptionNonLeakage);
    CPPUNIT_TEST_SUITE_END();

protected:

    void testSingleSubscription(async::EventLoop& iocontext) {
        async::SignalSet signalSet(iocontext, {SIGUSR1});

        bool eventWasCalled = false;
        signalSet.asyncWait().then([&eventWasCalled](int signalId){

            CPPUNIT_ASSERT_EQUAL(SIGUSR1, signalId);

            eventWasCalled = true;
        });

        CPPUNIT_ASSERT(!eventWasCalled);

        std::thread t([&iocontext]() {
            using namespace std::chrono_literals;

            std::this_thread::sleep_for(400ms);
            iocontext.stop();
        });

        raise(SIGUSR1);

        // Should block untill event is triggered
        iocontext.run();
        t.join();

        CPPUNIT_ASSERT(eventWasCalled);
    }


    void testSingleSubscription2(async::EventLoop& iocontext) {
        async::SignalSet signalSet(iocontext, {SIGUSR1, SIGUSR2});

        bool eventWasCalled = false;
        signalSet.asyncWait().then([&eventWasCalled](int signalId){

            CPPUNIT_ASSERT((signalId == SIGUSR1) || (signalId == SIGUSR2));

            eventWasCalled = true;
        });

        CPPUNIT_ASSERT(!eventWasCalled);

        std::thread t([&iocontext]() {
            using namespace std::chrono_literals;

            std::this_thread::sleep_for(200ms);
            iocontext.stop();
        });

        raise(SIGUSR2);
        CPPUNIT_ASSERT(!eventWasCalled);


        // Should block untill event is triggered
        iocontext.run();
        t.join();

        CPPUNIT_ASSERT(eventWasCalled);
    }

    void subscriptionNonLeakage(async::EventLoop& iocontext) {
        async::SignalSet signalSet1(iocontext, {SIGUSR1});
        async::SignalSet signalSet2(iocontext, {SIGUSR2});

        bool event1_wasCalled = false;
        bool event2_wasCalled = false;
        signalSet1.asyncWait().then([&event1_wasCalled](int signalId){
            CPPUNIT_ASSERT_EQUAL(SIGUSR1, signalId);

            event1_wasCalled = true;
        });

        signalSet2.asyncWait().then([&event2_wasCalled](int signalId){
            CPPUNIT_ASSERT_EQUAL(SIGUSR2, signalId);

            event2_wasCalled = true;
        });

        CPPUNIT_ASSERT(!event1_wasCalled);
        CPPUNIT_ASSERT(!event2_wasCalled);

        std::thread t([&iocontext]() {
            using namespace std::chrono_literals;

            std::this_thread::sleep_for(200ms);
            iocontext.stop();
        });

        raise(SIGUSR2);
        // Should block untill event is triggered
        iocontext.run();
        t.join();

        CPPUNIT_ASSERT(!event1_wasCalled);
        CPPUNIT_ASSERT(event2_wasCalled);
    }


public:

    void testEPoll_subscription() {
        auto iocontext = async::EventLoop(2, Selector::createEPoll(3));
        testSingleSubscription(iocontext);
    }

    void testEPoll_subscription2() {
        auto iocontext = async::EventLoop(2, Selector::createEPoll(3));
        testSingleSubscription2(iocontext);
    }

    void testPoll_subscription() {
        auto iocontext = async::EventLoop(2, Selector::createPoll(3));

        testSingleSubscription(iocontext);
    }

    void testPoll_subscription2() {
        auto iocontext = async::EventLoop(2, Selector::createPoll(3));

        testSingleSubscription2(iocontext);
    }


    void testSubscriptionNonLeakage() {
        {
            auto iocontext = async::EventLoop(2, Selector::createPoll(3));
            subscriptionNonLeakage(iocontext);
        }

        {
            auto iocontext = async::EventLoop(2, Selector::createEPoll(3));
            subscriptionNonLeakage(iocontext);
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestAsyncSignalSet);
