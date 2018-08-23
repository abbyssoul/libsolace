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
 *	@file test/test_vector.cpp
 *	@brief		Test suit for Solace::Vector
 ******************************************************************************/
#include <solace/vector.hpp>    // Class being tested.

#include <gtest/gtest.h>
#include "mockTypes.hpp"

using namespace Solace;

TEST(TestVector, testEmptyIntegralVectorIsEmpty) {
    Vector<uint32> v;

    EXPECT_TRUE(v.empty());
    EXPECT_EQ(0, v.size());
    EXPECT_EQ(0, v.capacity());
}


TEST(TestVector, testEmptyVectorIsEmpty) {
    // Test pre-condition
    ASSERT_EQ(0, SimpleType::InstanceCount);

    Vector<SimpleType> v;

    EXPECT_TRUE(v.empty());
    EXPECT_EQ(0, v.size());
    EXPECT_EQ(0, v.capacity());
    EXPECT_EQ(0, SimpleType::InstanceCount);
}

TEST(TestVector, factoryIntergralVectorWithCapacity) {
    auto v = makeVector<int32>(10);

    EXPECT_EQ(10, v.capacity());
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(0, v.size());
}

TEST(TestVector, factoryVectorWithCapacityCreatesNoObjects) {
    ASSERT_EQ(0, SimpleType::InstanceCount);
    auto v = makeVector<SimpleType>(10);

    EXPECT_EQ(10, v.capacity());
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(0, v.size());
    EXPECT_EQ(0, SimpleType::InstanceCount);
}

TEST(TestVector, emplaceBack) {
    ASSERT_EQ(0, SimpleType::InstanceCount);
    {
        auto v = makeVector<SimpleType>(10);
        v.emplace_back(3, 2, 1);
        v.emplace_back(2, 1, 0);
        v.emplace_back(1, 0, -1);

        EXPECT_EQ(10, v.capacity());
        EXPECT_FALSE(v.empty());
        EXPECT_EQ(3, v.size());
        EXPECT_EQ(3, SimpleType::InstanceCount);
    }

    // Important to make sure all the instances has been correctly destructed after scope exit
    ASSERT_EQ(0, SimpleType::InstanceCount);
}


TEST(TestVector, emptyVectorEmplaceFails) {
    EXPECT_ANY_THROW(Vector<int32>().emplace_back(212));
}

TEST(TestVector, emplaceOverflowThrows) {
    ASSERT_EQ(0, SimpleType::InstanceCount);
    {
        auto v = makeVector<SimpleType>(3);
        EXPECT_EQ(3, v.capacity());

        v.emplace_back(3, 2, 1);
        v.emplace_back(2, 1, 0);
        v.emplace_back(1, 0, -1);

        EXPECT_EQ(3, v.size());
        EXPECT_ANY_THROW(v.emplace_back(1, 0, -1));

        EXPECT_EQ(3, SimpleType::InstanceCount);
    }

    // Important to make sure all the instances has been correctly destructed after scope exit
    ASSERT_EQ(0, SimpleType::InstanceCount);
}


TEST(TestVector, movedFromVectorIsEmpty) {
    ASSERT_EQ(0, SimpleType::InstanceCount);
    {
        auto v = makeVector<SimpleType>(10);
        v.emplace_back(3, 2, 1);
        v.emplace_back(2, 1, 0);
        v.emplace_back(1, 0, -1);

        EXPECT_EQ(10, v.capacity());
        EXPECT_FALSE(v.empty());
        EXPECT_EQ(3, v.size());
        EXPECT_EQ(3, SimpleType::InstanceCount);

        auto other = std::move(v);
        EXPECT_EQ(0, v.capacity());
        EXPECT_TRUE(v.empty());

        EXPECT_EQ(10, other.capacity());
        EXPECT_FALSE(other.empty());
        EXPECT_EQ(3, other.size());
        EXPECT_EQ(3, SimpleType::InstanceCount);
    }

    // Important to make sure all the instances has been correctly destructed after scope exit
    EXPECT_EQ(0, SimpleType::InstanceCount);
}



TEST(TestVector, movingWhenCopyConstructorThrowsIsSafe) {
    SometimesConstructable::BlowUpEveryInstance = 6;
    ASSERT_EQ(0, SometimesConstructable::InstanceCount);

    auto v = makeVector<SometimesConstructable>(10);

    EXPECT_TRUE(v.empty());
    EXPECT_EQ(0, v.size());
    EXPECT_EQ(10, v.capacity());
    EXPECT_EQ(0, SometimesConstructable::InstanceCount);

    auto movedInto = std::move(v);

    EXPECT_TRUE(v.empty());
    EXPECT_EQ(0, v.size());
    EXPECT_EQ(0, v.capacity());
    EXPECT_EQ(0, SometimesConstructable::InstanceCount);
}
