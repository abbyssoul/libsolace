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
 *	@file test/test_dictionary.cpp
 *	@brief		Test suit for Solace::Dictionary
 ******************************************************************************/
#include <solace/dictionary.hpp>    // Class being tested.

#include <gtest/gtest.h>
#include "mockTypes.hpp"

using namespace Solace;


TEST(TestDictionary, testEmptyIntegralDictionaryIsEmpty) {
    Dictionary<int32, int32> v;

    EXPECT_TRUE(v.empty());
    EXPECT_EQ(0, v.size());
    EXPECT_EQ(0, v.capacity());
}


TEST(TestDictionary, testEmptyDictionaryIsEmpty) {
    // Test pre-condition
    ASSERT_EQ(0, SimpleType::InstanceCount);

    Dictionary<int32, SimpleType> v;

    EXPECT_TRUE(v.empty());
    EXPECT_EQ(0, v.size());
    EXPECT_EQ(0, v.capacity());
    EXPECT_EQ(0, SimpleType::InstanceCount);
}

TEST(TestDictionary, factoryIntergralWithCapacity) {
    auto v = makeDictionary<int32, int32>(10);

    EXPECT_EQ(10, v.capacity());
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(0, v.size());
}

TEST(TestDictionary, factoryWithCapacityCreatesNoObjects) {
    ASSERT_EQ(0, SimpleType::InstanceCount);
    auto v = makeDictionary<int32, SimpleType>(10);

    EXPECT_EQ(10, v.capacity());
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(0, v.size());
    EXPECT_EQ(0, SimpleType::InstanceCount);
}


TEST(TestDictionary, addIntoEmptyCollectionFails) {
    Dictionary<int32, int32> integral;
    EXPECT_ANY_THROW(integral.put(212, 8288));

    Dictionary<int32, SimpleType> custom;
    EXPECT_ANY_THROW(custom.put(212, {1, 2, 3}));
}


TEST(TestDictionary, addIntoIntegralNonEmptyCollection) {
    auto v = makeDictionary<int32, int32>(10);

    EXPECT_EQ(10, v.capacity());
    EXPECT_TRUE(v.empty());

    v.put(321, 99888);
    EXPECT_FALSE(v.empty());
    EXPECT_EQ(1, v.size());

    v.put(132, 99888);
    EXPECT_FALSE(v.empty());
    EXPECT_EQ(2, v.size());
}

TEST(TestDictionary, addIntoNonEmptyCollection) {
    ASSERT_EQ(0, SimpleType::InstanceCount);
    {
        auto v = makeDictionary<int32, SimpleType>(10);

        EXPECT_EQ(10, v.capacity());
        EXPECT_TRUE(v.empty());

        v.put(321, 99888, 2, 3);
        EXPECT_FALSE(v.empty());
        EXPECT_EQ(1, v.size());

        v.put(132, 99888, 3, 2);
        EXPECT_FALSE(v.empty());
        EXPECT_EQ(2, v.size());
    }

    ASSERT_EQ(0, SimpleType::InstanceCount);
}


TEST(TestDictionary, containsDataType) {
    ASSERT_EQ(0, SimpleType::InstanceCount);
    {
        auto v = makeDictionary<int32, SimpleType>(
                                                {0, {99888, 2, 3}},
                                                Dictionary<int32, SimpleType>::Entry{321, {1, 2, 3}},
                                                Dictionary<int32, SimpleType>::Entry{17, {3, 0, 0}});
        EXPECT_TRUE(v.contains(321));
        EXPECT_TRUE(v.contains(17));
        EXPECT_FALSE(v.contains(18));
    }

    ASSERT_EQ(0, SimpleType::InstanceCount);
}


TEST(TestDictionary, containsUsingCustomKey) {
    ASSERT_EQ(0, SimpleType::InstanceCount);
    {
        auto v = makeDictionary<int32, SimpleType>(
                                                {0, {99888, 2, 3}},
                                                Dictionary<int32, SimpleType>::Entry{321, {1, 2, 3}},
                                                Dictionary<int32, SimpleType>::Entry{17, {3, 0, 0}});
        EXPECT_TRUE(v.contains(321));
        EXPECT_TRUE(v.contains(17));

        EXPECT_FALSE(v.contains(18));
    }

    ASSERT_EQ(0, SimpleType::InstanceCount);
}


TEST(TestDictionary, forEachValue) {
    ASSERT_EQ(0, SimpleType::InstanceCount);
    {
        auto v = makeDictionary<int32, SimpleType>(
                                                {-1,  {1, 2, 3}},
                                                Dictionary<int32, SimpleType>::Entry{13, {2, 3, 4}},
                                                Dictionary<int32, SimpleType>::Entry{17, {3, 4, 5}});

        int32 acc = 0;
        int32 counter = 1;
        for (auto const& i : v.values()) {
            EXPECT_EQ(counter, i.x);
            acc += i.z;
            counter += 1;
        }

        EXPECT_EQ(12, acc);

        acc = 0;
        for (auto const& i : v.keys()) {
            acc += i;
        }

        EXPECT_EQ(29, acc);
    }

    ASSERT_EQ(0, SimpleType::InstanceCount);
}



TEST(TestDictionary, constructionThrow) {
    ASSERT_EQ(0, SimpleType::InstanceCount);
    ASSERT_EQ(0, SometimesConstructable::InstanceCount);

    SometimesConstructable::BlowUpEveryInstance = 4;
    {
        EXPECT_ANY_THROW( (makeDictionary<SimpleType, SometimesConstructable>(
                             Dictionary<SimpleType, SometimesConstructable>::Entry{{81, 2, 3}, 1},
                             Dictionary<SimpleType, SometimesConstructable>::Entry{{12, 7, 3}, 2},
                             Dictionary<SimpleType, SometimesConstructable>::Entry{{-3, 0, 0}, 3}
                         )) );

        EXPECT_EQ(0, SimpleType::InstanceCount);
        EXPECT_EQ(0, SometimesConstructable::InstanceCount);
    }
}
