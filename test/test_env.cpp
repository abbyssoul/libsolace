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
#include <solace/env.hpp>  // Class being tested

#include <solace/uuid.hpp>  // For random names

#include <gtest/gtest.h>

using namespace Solace;


StringView randomName() {
//    return String::join("-", {"test-env-", UUID::random().toString()});

    static char buffer[9 + UUID::StringSize + 1];
    memset(buffer, 0, sizeof(buffer));
    strncpy(buffer, "test-env-", sizeof(buffer));

    return makeRandomUUID().toString(wrapMemory(buffer + 9, UUID::StringSize));

}

StringView randomValue() {
    static char buffer[UUID::StringSize + 1];
    memset(buffer, 0, sizeof(buffer));

    return makeRandomUUID().toString(wrapMemory(buffer));
}



TEST(TestProcessEnv, testComplex) {
    auto const name = randomName();

    auto env = Env{};
    {
        auto const uid1 = randomValue();
        EXPECT_TRUE(env.set(name, uid1).isOk());
        EXPECT_FALSE(env.empty());
        EXPECT_GT(env.size(), 1);

        auto const var = env.get(name);
        EXPECT_TRUE(var.isSome());
        EXPECT_EQ(uid1, *var);
    }

    {
        auto const uid2 = randomValue();
        EXPECT_TRUE(env.set(name, uid2).isOk());

        auto const var2 = env.get(name);
        EXPECT_EQ(uid2, *var2);
    }
}


TEST(TestProcessEnv, testSize) {
    auto const name = randomName();

    auto env = Env{};
    EXPECT_TRUE(env.get(name).isNone());

    auto const currentSize = env.size();
    EXPECT_TRUE(env.set(name, randomValue()).isOk());
    EXPECT_EQ(currentSize + 1, env.size());
}

TEST(TestProcessEnv, testUnset) {
    auto const name = randomName();

    auto env = Env{};

    EXPECT_TRUE(env.get(name).isNone());
    auto const currentSize = env.size();

    EXPECT_TRUE(env.set(name, randomValue()).isOk());
    EXPECT_EQ(currentSize + 1, env.size());
    EXPECT_TRUE(env.unset(name).isOk());
    EXPECT_EQ(currentSize, env.size());
}

TEST(TestProcessEnv, testSetIllFormatedVar) {
    auto env = Env{};

    EXPECT_TRUE(env.set(StringView{}, randomValue()).isError());
    EXPECT_TRUE(env.set("DumbName=", randomValue()).isError());
    EXPECT_TRUE(env.set("Dumb=Name", "").isError());
    EXPECT_TRUE(env.unset("DumbName=").isError());
    EXPECT_TRUE(env.unset(StringView{}).isError());
}

TEST(TestProcessEnv, testSetEmptyString) {
    auto const name = randomName();

    auto env = Env{};

    EXPECT_TRUE(env.get(name).isNone());
    EXPECT_TRUE(env.set(name, StringView{}).isOk());
    EXPECT_TRUE(env.get(name).isSome());
    EXPECT_TRUE(env.unset(name).isOk());
    EXPECT_TRUE(env.get(name).isNone());
}

TEST(TestProcessEnv, testIteration) {
    auto const name = randomName();

    auto env = Env{};

    EXPECT_TRUE(env.get(name).isNone());
    auto const currentSize = env.size();

    Env::size_type iSize = 0;
    for (auto i : env) {
        EXPECT_NE(i.name, name);
        ++iSize;
    }
    EXPECT_EQ(currentSize, iSize);

    EXPECT_TRUE(env.set(name, randomValue()).isOk());
    bool found = false;
    iSize = 0;
    for (auto i : env) {
        if (i.name == name) {
            found = true;
            break;
        }
        ++iSize;
    }
    EXPECT_TRUE(found);

    EXPECT_TRUE(env.unset(name).isOk());
}


TEST(TestProcessEnv, forEachCantMutate) {
    auto const name = randomName();

    auto env = Env{};

    EXPECT_TRUE(env.get(name).isNone());

    auto const initialValue = randomValue();
    EXPECT_TRUE(env.set(name, initialValue).isOk());

    env.forEach([&](Env::Var var) {
        if (var.name == name) {
            var.value = "Blarg!";  // Such name, very random
        }
    });

    EXPECT_EQ(initialValue, *env.get(name));
    EXPECT_TRUE(env.unset(name).isOk());
}
