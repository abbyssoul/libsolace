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

#include <gtest/gtest.h>

using namespace Solace;
using namespace Solace::Process;

class TestProcessEnv: public ::testing::Test {

public:

    void setUp() {
	}

    void tearDown() {
	}
};

TEST_F(TestProcessEnv, testComplex) {
    const auto uid1 = UUID::random();
    const auto uid2 = UUID::random();
    const auto uid3 = UUID::random();

    const auto name = String::join("-", {"test", "env", uid1.toString()});

    auto env = Env();

    EXPECT_NO_THROW(env.set(name, uid1.toString()));
    EXPECT_EQ(false, env.empty());
    EXPECT_GT(env.size(), 1);
    EXPECT_EQ(uid1.toString(), env[name]);

    EXPECT_NO_THROW(env.set(name, uid2.toString()));
    EXPECT_EQ(uid2.toString(), env.get(name).get());
}

TEST_F(TestProcessEnv, testSize) {
    const auto uid = UUID::random();
    const auto name = String::join("-", {"test", "env", uid.toString()});

    auto env = Env();

    EXPECT_TRUE(env.get(name).isNone());
    const auto currentSize = env.size();

    EXPECT_NO_THROW(env.set(name, uid.toString()));
    EXPECT_EQ(currentSize + 1, env.size());
}

TEST_F(TestProcessEnv, testUnset) {
    const auto uid = UUID::random();
    const auto name = String::join("-", {"test", "env", uid.toString()});

    auto env = Env();

    EXPECT_TRUE(env.get(name).isNone());
    const auto currentSize = env.size();

    EXPECT_NO_THROW(env.set(name, uid.toString()));
    EXPECT_EQ(currentSize + 1, env.size());
    EXPECT_NO_THROW(env.unset(name));
    EXPECT_EQ(currentSize, env.size());
}

TEST_F(TestProcessEnv, testSetIllFormatedVar) {
    auto env = Env();

    EXPECT_THROW(env.set(String::Empty, "whatever"), Exception);
    EXPECT_THROW(env.set("DumbName=", "whatever"), Exception);
    EXPECT_THROW(env.unset("DumbName="), Exception);
    EXPECT_THROW(env.unset(String::Empty), Exception);
}

TEST_F(TestProcessEnv, testSetEmptyString) {
    auto env = Env();

    const auto uid = UUID::random();
    const auto name = String::join("-", {"test", "env", uid.toString()});

    EXPECT_TRUE(env.get(name).isNone());
    EXPECT_NO_THROW(env.set(name, String::Empty));
    EXPECT_TRUE(env.get(name).isSome());
    EXPECT_NO_THROW(env.unset(name));
    EXPECT_TRUE(env.get(name).isNone());
}

TEST_F(TestProcessEnv, testIteration) {
    const auto uid = UUID::random();
    const auto name = String::join("-", {"test", "env", uid.toString()});

    auto env = Env();

    EXPECT_TRUE(env.get(name).isNone());
    const auto currentSize = env.size();

    Env::size_type iSize = 0;
    for (auto i : env) {
        EXPECT_TRUE(i.name != name);
        ++iSize;
    }
    EXPECT_EQ(currentSize, iSize);

    EXPECT_NO_THROW(env.set(name, uid.toString()));
    bool found = false;
    iSize = 0;
    for (auto i : env) {
        if (i.name == name) {
            found = true;
            break;
        }
        ++iSize;
    }
    EXPECT_EQ(true, found);

    EXPECT_NO_THROW(env.unset(name));
}
