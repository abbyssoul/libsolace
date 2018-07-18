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
 * @file: test/io/test_signalDispatcher.cpp
 * @author: soultaker
 *
 * Created on: 12 Jun 2016
*******************************************************************************/
#include <solace/io/signalDispatcher.hpp>  // Class being tested

#include <gtest/gtest.h>

#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

using namespace Solace;
using namespace Solace::IO;

class TestIOSignalDispatcher : public ::testing::Test {

public:

    void setUp() {
	}

    void tearDown() {
	}
};

TEST_F(TestIOSignalDispatcher, testSubscription) {
    bool signaled = false;

    SignalDispatcher::getInstance().attachHandler(SIGALRM, [&signaled](int signalId) {
        signaled = (signalId == SIGALRM);
    });

    // Generate ALARM signal, in 1 sec
    itimerval timeToSleep;
    timeToSleep.it_interval.tv_sec = 0;
    timeToSleep.it_interval.tv_usec = 0;
    timeToSleep.it_value.tv_sec = 0;
    timeToSleep.it_value.tv_usec = 250 * 1000;
    EXPECT_EQ(0, setitimer(ITIMER_REAL, &timeToSleep, nullptr));

    usleep(400 * 1000);
    EXPECT_TRUE(signaled);

    // Reset
    signaled = false;
    int count = 0;

    SignalDispatcher::getInstance().attachHandler(SIGALRM, [&count](int signalId) {
        count += (signalId == SIGALRM) ? 1 : 0;
    });
    SignalDispatcher::getInstance().attachHandler(SIGALRM, [&count](int signalId) {
        count += (signalId == SIGALRM) ? 1 : 0;
    });
    SignalDispatcher::getInstance().attachHandler(SIGALRM, [&count](int signalId) {
        count += (signalId == SIGALRM) ? 1 : 0;
    });

    timeToSleep.it_value.tv_usec = 300 * 1000;
    EXPECT_EQ(0, setitimer(ITIMER_REAL, &timeToSleep, nullptr));

    usleep(600 * 1000);

    EXPECT_TRUE(signaled);
    EXPECT_EQ(3, count);
}
