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
 * @file: test/io/test_selector_epoll.cpp
 * @author: soultaker
 *
 * Created on: 10/06/2016
*******************************************************************************/
#include <solace/io/selector.hpp>  // Class being tested

#include <solace/io/pipe.hpp>
#include <cppunit/extensions/HelperMacros.h>


using namespace Solace;
using namespace Solace::IO;


class TestEPollSelector : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestEPollSelector);
        CPPUNIT_TEST(testSubscription);
        CPPUNIT_TEST(testEmptyPolling);
        CPPUNIT_TEST(testRemoval);
        CPPUNIT_TEST(testRemovalOfNotAddedItem);
        CPPUNIT_TEST(testReadPolling);
    CPPUNIT_TEST_SUITE_END();

public:

    void testSubscription() {
        Pipe p;

        auto s = Selector::createEPoll(2);
        s.add(&p.getReadEnd(), Selector::Read);
        s.add(&p.getWriteEnd(), Selector::Write);

        auto i = s.poll(1);
        CPPUNIT_ASSERT(i != i.end());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint>(1), i.size());

        auto ev = *i;
//        CPPUNIT_ASSERT_EQUAL(static_cast<void*>(&p.getWriteEnd()), ev.data);
        CPPUNIT_ASSERT_EQUAL(p.getWriteEnd().getSelectId(), ev.fd);
    }


    void testReadPolling() {
        Pipe p;

        auto s = Selector::createEPoll(1);
        s.add(&p.getReadEnd(), Selector::Read);

        auto i = s.poll(1);

        // Test that poll times out correctly as nothing has been written so far.
        CPPUNIT_ASSERT(i == i.end());

        char msg[] = "message";
        const auto written = p.write(wrapMemory(msg));
        CPPUNIT_ASSERT(written.isOk());

        i = s.poll(1);
        CPPUNIT_ASSERT(i != i.end());

        auto ev = *i;
//        CPPUNIT_ASSERT_EQUAL(static_cast<void*>(&p.getReadEnd()), ev.data);
        CPPUNIT_ASSERT_EQUAL(p.getReadEnd().getSelectId(), ev.fd);

        char buff[100];
        auto m = wrapMemory(buff);
        auto dest = m.slice(0, written.getResult());
        const auto bytesRead = p.read(dest);
        CPPUNIT_ASSERT(bytesRead.isOk());
        CPPUNIT_ASSERT_EQUAL(written.getResult(), bytesRead.getResult());

        // There is no more data in the pipe so the next poll must time-out
        i = s.poll(1);

        // Test that poll times out correctly as nothing has been written so far.
        CPPUNIT_ASSERT(i == i.end());
    }


    void testEmptyPolling() {
        Selector s = Selector::createEPoll(3);

        auto i = s.poll(1);
        CPPUNIT_ASSERT(!(i != i.end()));
        CPPUNIT_ASSERT_EQUAL(static_cast<uint>(0), i.size());
        CPPUNIT_ASSERT_THROW(++i, IndexOutOfRangeException);
    }

    void testRemoval() {
        Pipe p;

        Selector s = Selector::createEPoll(5);
        s.add(&p.getReadEnd(), Selector::Read);
        s.add(&p.getWriteEnd(), Selector::Write);

        {
            auto i = s.poll(1);
            CPPUNIT_ASSERT(i != i.end());
            CPPUNIT_ASSERT_EQUAL(static_cast<uint>(1), i.size());

            auto ev = *i;
            CPPUNIT_ASSERT_EQUAL(p.getWriteEnd().getSelectId(), ev.fd);
        }

        {
            s.remove(&p.getWriteEnd());
            auto i = s.poll(1);
            CPPUNIT_ASSERT(i == i.end());
            CPPUNIT_ASSERT_EQUAL(static_cast<uint>(0), i.size());
        }
    }

    void testRemovalOfNotAddedItem() {
        Pipe p;

        auto s = Selector::createEPoll(5);
        CPPUNIT_ASSERT_NO_THROW(s.remove(&p.getReadEnd()));
        CPPUNIT_ASSERT_NO_THROW(s.remove(&p.getWriteEnd()));
    }


};

CPPUNIT_TEST_SUITE_REGISTRATION(TestEPollSelector);
