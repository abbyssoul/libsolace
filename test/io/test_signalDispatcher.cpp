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
 * @file: test/test_writeBuffer.cpp
 * @author: soultaker
 *
 * Created on: 28 Apr 2016
*******************************************************************************/
#include <solace/io/signalDispatcher.hpp>  // Class being tested

#include <cppunit/extensions/HelperMacros.h>

#include <unistd.h>
#include <signal.h>


using namespace Solace;
using namespace Solace::IO;


class TestIOSignalDispatcher : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestIOSignalDispatcher);
        CPPUNIT_TEST(testSubscription);
    CPPUNIT_TEST_SUITE_END();

public:

    void testSubscription() {
        bool signaled = false;

        SignalDispatcher::getInstance().attachHandler(SIGALRM, [&signaled](int signalId) {
            signaled = (signalId == SIGALRM);
        });

        // Generate ALARM signal, in 1 sec
        alarm(1);
        sleep(2);
        CPPUNIT_ASSERT(signaled);

        // Reset
        signaled = false;
        int count = 0;

        SignalDispatcher::getInstance().attachHandler(SIGALRM, [&count](int) {
            count++;
        });
        SignalDispatcher::getInstance().attachHandler(SIGALRM, [&count](int) {
            count++;
        });
        SignalDispatcher::getInstance().attachHandler(SIGALRM, [&count](int) {
            count++;
        });

        alarm(1);
        sleep(2);
        CPPUNIT_ASSERT(signaled);
        CPPUNIT_ASSERT_EQUAL(3, count);
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestIOSignalDispatcher);

