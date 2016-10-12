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
 * @file: test/io/async/test_event.cpp
 * @author: soultaker
 *
 * Created on: 10/10/2016
*******************************************************************************/
#include <solace/io/async/event.hpp>  // Class being tested

#include <cppunit/extensions/HelperMacros.h>

#include <thread>
#include <chrono>

#include <unistd.h>
#include <fcntl.h>

#include <iostream>

using namespace Solace;
using namespace Solace::IO;


class TestAsyncEvent : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestAsyncEvent);
        CPPUNIT_TEST(testSubscriptionEPoll);
        CPPUNIT_TEST(testSubscriptionPoll);
    CPPUNIT_TEST_SUITE_END();

protected:

    void testSubscription(async::EventLoop& iocontext) {
        async::Event event(iocontext);

        bool eventWasCalled = false;
        event.asyncWait().then([&eventWasCalled, &iocontext](){
            eventWasCalled = true;
        });

        CPPUNIT_ASSERT(!eventWasCalled);

        std::thread t([&event](){
            using namespace std::chrono_literals;

            std::this_thread::sleep_for(200ms);
            event.notify();
        });

        // Should block untill event is triggered
        try {
            iocontext.run();

            t.join();
        } catch(Solace::Exception& e) {
            if (t.joinable())
                t.join();

            throw;
        }

        CPPUNIT_ASSERT(eventWasCalled);
    }

public:

    void testSubscriptionEPoll() {
        auto iocontext = async::EventLoop(2, Selector::createEPoll(3));
        testSubscription(iocontext);
    }

    void testSubscriptionPoll() {
        auto iocontext = async::EventLoop(2, Selector::createPoll(3));
        testSubscription(iocontext);
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestAsyncEvent);
