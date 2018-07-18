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
 * @file: test/process/test_env.cpp
 * @author: soultaker
 *
 * Created on: 20/06/2016
*******************************************************************************/
#include <solace/process/env.hpp>  // Class being tested

#include <solace/exception.hpp>
#include <solace/uuid.hpp>

#include <cppunit/extensions/HelperMacros.h>


using namespace Solace;
using namespace Solace::Process;


class TestProcessEnv: public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestProcessEnv);
        CPPUNIT_TEST(testComplex);
        CPPUNIT_TEST(testSize);
        CPPUNIT_TEST(testSetIllFormatedVar);
        CPPUNIT_TEST(testSetEmptyString);
        CPPUNIT_TEST(testUnset);
        CPPUNIT_TEST(testIteration);
    CPPUNIT_TEST_SUITE_END();

public:

    void testComplex() {
        const auto uid1 = UUID::random();
        const auto uid2 = UUID::random();
        const auto uid3 = UUID::random();

        const auto name = String::join("-", {"test", "env", uid1.toString()});

        auto env = Env();

        CPPUNIT_ASSERT_NO_THROW(env.set(name, uid1.toString()));
        CPPUNIT_ASSERT_EQUAL(false, env.empty());
        CPPUNIT_ASSERT(env.size() > 1);
        CPPUNIT_ASSERT_EQUAL(uid1.toString(), env[name]);

        CPPUNIT_ASSERT_NO_THROW(env.set(name, uid2.toString()));
        CPPUNIT_ASSERT_EQUAL(uid2.toString(), env.get(name).get());
    }


    void testSize() {
        const auto uid = UUID::random();
        const auto name = String::join("-", {"test", "env", uid.toString()});

        auto env = Env();

        CPPUNIT_ASSERT(env.get(name).isNone());
        const auto currentSize = env.size();

        CPPUNIT_ASSERT_NO_THROW(env.set(name, uid.toString()));
        CPPUNIT_ASSERT_EQUAL(currentSize + 1, env.size());
    }


    void testUnset() {
        const auto uid = UUID::random();
        const auto name = String::join("-", {"test", "env", uid.toString()});

        auto env = Env();

        CPPUNIT_ASSERT(env.get(name).isNone());
        const auto currentSize = env.size();

        CPPUNIT_ASSERT_NO_THROW(env.set(name, uid.toString()));
        CPPUNIT_ASSERT_EQUAL(currentSize + 1, env.size());
        CPPUNIT_ASSERT_NO_THROW(env.unset(name));
        CPPUNIT_ASSERT_EQUAL(currentSize, env.size());
    }


    void testSetIllFormatedVar() {
        auto env = Env();

        CPPUNIT_ASSERT_THROW(env.set(String::Empty, "whatever"), Exception);
        CPPUNIT_ASSERT_THROW(env.set("DumbName=", "whatever"), Exception);
        CPPUNIT_ASSERT_THROW(env.unset("DumbName="), Exception);
        CPPUNIT_ASSERT_THROW(env.unset(String::Empty), Exception);
    }


    void testSetEmptyString() {
        auto env = Env();

        const auto uid = UUID::random();
        const auto name = String::join("-", {"test", "env", uid.toString()});

        CPPUNIT_ASSERT(env.get(name).isNone());
        CPPUNIT_ASSERT_NO_THROW(env.set(name, String::Empty));
        CPPUNIT_ASSERT(env.get(name).isSome());
        CPPUNIT_ASSERT_NO_THROW(env.unset(name));
        CPPUNIT_ASSERT(env.get(name).isNone());
    }


    void testIteration() {
        const auto uid = UUID::random();
        const auto name = String::join("-", {"test", "env", uid.toString()});

        auto env = Env();

        CPPUNIT_ASSERT(env.get(name).isNone());
        const auto currentSize = env.size();

        Env::size_type iSize = 0;
        for (auto i : env) {
            CPPUNIT_ASSERT(i.name != name);
            ++iSize;
        }
        CPPUNIT_ASSERT_EQUAL(currentSize, iSize);

        CPPUNIT_ASSERT_NO_THROW(env.set(name, uid.toString()));
        bool found = false;
        iSize = 0;
        for (auto i : env) {
            if (i.name == name) {
                found = true;
                break;
            }
            ++iSize;
        }
        CPPUNIT_ASSERT_EQUAL(true, found);

        CPPUNIT_ASSERT_NO_THROW(env.unset(name));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestProcessEnv);
