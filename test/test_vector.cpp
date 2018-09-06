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


TEST(TestVector, emptyVectorEmplaceFails) {
    EXPECT_ANY_THROW(Vector<int32>().emplace_back(212));
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


TEST(TestVector, constructionFromInitializerList) {
    ASSERT_EQ(0, SimpleType::InstanceCount);
    {
        auto v = makeVector<SimpleType>({
                                            {3, 2, 1},
                                            {2, 1, 0},
                                            {1, 0, -1}
                                        });
        EXPECT_FALSE(v.empty());
        EXPECT_EQ(3, v.capacity());
        EXPECT_EQ(3, v.size());
        EXPECT_EQ(3, SimpleType::InstanceCount);
    }

    // Important to make sure all the instances has been correctly destructed after scope exit
    EXPECT_EQ(0, SimpleType::InstanceCount);
}

TEST(TestVector, copy) {
    ASSERT_EQ(0, SimpleType::InstanceCount);
    {
        auto const origin = makeVector<SimpleType>({
                                            {3, 2, 1},
                                            {2, 1, 0},
                                            {1, 0, -1}
                                        });
        EXPECT_EQ(3, SimpleType::InstanceCount);

        auto v = makeVector(origin);  // Make a copy
        EXPECT_FALSE(v.empty());
        EXPECT_EQ(3, v.capacity());
        EXPECT_EQ(3, v.size());
        EXPECT_EQ(6, SimpleType::InstanceCount);
    }

    // Important to make sure all the instances has been correctly destructed after scope exit
    EXPECT_EQ(0, SimpleType::InstanceCount);
}

TEST(TestVector, interatingOverEmptyVector) {
    EXPECT_EQ(0, SimpleType::InstanceCount);

    Vector<SimpleType> v;

    int acc = 0;
    int counter = 0;
    for (auto& i : v) {
        acc += i.x;
        counter += 1;
    }

    EXPECT_EQ(0, acc);
    EXPECT_EQ(0, counter);
}



TEST(TestVector, interationNoMutation) {
    ASSERT_EQ(0, SimpleType::InstanceCount);
    {
        auto const v = makeVector<SimpleType>({
                                            {3, 2, 1},
                                            {2, 1, 0},
                                            {1, 0, -1}
                                        });
        int counter = 3;
        for (auto const& i : v) {
            EXPECT_EQ(counter - 0, i.x);
            EXPECT_EQ(counter - 1, i.y);
            EXPECT_EQ(counter - 2, i.z);
            counter -= 1;
        }
    }

    // Important to make sure all the instances has been correctly destructed after scope exit
    EXPECT_EQ(0, SimpleType::InstanceCount);
}

TEST(TestVector, interationMutation) {
    ASSERT_EQ(0, SimpleType::InstanceCount);
    {
        auto v = makeVector<SimpleType>({
                                            {3, 2, 1},
                                            {2, 1, 0},
                                            {1, 0, -1}
                                        });
        for (auto& i : v) {
            i.z = i.x + i.y;
        }

        int counter = 5;
        for (auto const& i : v) {
            EXPECT_EQ(counter, i.z);
            counter -= 2;
        }
    }

    // Important to make sure all the instances has been correctly destructed after scope exit
    EXPECT_EQ(0, SimpleType::InstanceCount);
}


TEST(TestVector, popBack) {
    ASSERT_EQ(0, SimpleType::InstanceCount);
    {
        auto v = makeVector<SimpleType>(10);
        v.emplace_back(3, 2, 1);
        v.emplace_back(2, 1, 0);
        v.emplace_back(1, 0, -1);
        v.emplace_back(2, 1, 0);
        v.emplace_back(3, 2, 1);

        EXPECT_EQ(10, v.capacity());
        EXPECT_FALSE(v.empty());
        EXPECT_EQ(5, v.size());
        EXPECT_EQ(5, SimpleType::InstanceCount);

        EXPECT_NO_THROW(v.pop_back());
        EXPECT_EQ(4, v.size());
        EXPECT_EQ(4, SimpleType::InstanceCount);
        EXPECT_EQ(SimpleType(2, 1, 0), v[3]);

        EXPECT_NO_THROW(v.pop_back());
        EXPECT_EQ(3, v.size());
        EXPECT_EQ(3, SimpleType::InstanceCount);
        EXPECT_EQ(SimpleType(1, 0, -1), v[2]);
    }

    // Important to make sure all the instances has been correctly destructed after scope exit
    ASSERT_EQ(0, SimpleType::InstanceCount);
}



TEST(TestVector, moveOnlyTypes) {
    ASSERT_EQ(0, MoveOnlyType::InstanceCount);
    {
        auto v = makeVector<MoveOnlyType>(10);
        EXPECT_EQ(10, v.capacity());
        EXPECT_TRUE(v.empty());
        EXPECT_EQ(0, v.size());
        EXPECT_EQ(0, MoveOnlyType::InstanceCount);

        v.emplace_back(3);
        v.emplace_back(2);
        v.emplace_back(1);
        v.emplace_back(0);
        v.emplace_back(-1);

        EXPECT_EQ(10, v.capacity());
        EXPECT_FALSE(v.empty());
        EXPECT_EQ(5, v.size());
        EXPECT_EQ(5, MoveOnlyType::InstanceCount);

        int counter = 3;
        for (auto& a : v) {
            EXPECT_EQ(counter, a.x_);
            counter -= 1;
        }

        EXPECT_NO_THROW(v.pop_back());
        EXPECT_EQ(4, v.size());
        EXPECT_EQ(4, MoveOnlyType::InstanceCount);
        EXPECT_EQ(0, v[3].x_);

        EXPECT_NO_THROW(v.pop_back());
        EXPECT_EQ(3, v.size());
        EXPECT_EQ(3, MoveOnlyType::InstanceCount);
        EXPECT_EQ(1, v[2].x_);
    }

    // Important to make sure all the instances has been correctly destructed after scope exit
    ASSERT_EQ(0, MoveOnlyType::InstanceCount);
}

TEST(TestVector, toArray_MoveOnlyType) {
    ASSERT_EQ(0, MoveOnlyType::InstanceCount);
    {
        auto v = makeVector<MoveOnlyType>(4);
        v.emplace_back(7);
        v.emplace_back(5);
        v.emplace_back(3);
        v.emplace_back(1);
        EXPECT_EQ(4, v.capacity());
        EXPECT_FALSE(v.empty());
        EXPECT_EQ(4, v.size());
        EXPECT_EQ(4, MoveOnlyType::InstanceCount);

        Array<MoveOnlyType> array = v.toArray();
        EXPECT_FALSE(array.empty());
        EXPECT_EQ(4, array.size());
        EXPECT_EQ(4, MoveOnlyType::InstanceCount);

        // vector 'v' has been moved from - thus should be 'empty'
        EXPECT_TRUE(v.empty());
        EXPECT_EQ(0, v.capacity());
        EXPECT_EQ(0, v.size());
    }

    // Important to make sure all the instances has been correctly destructed after scope exit
    ASSERT_EQ(0, MoveOnlyType::InstanceCount);
}

TEST(TestVector, toArray) {
    ASSERT_EQ(0, SometimesConstructable::InstanceCount);
    SometimesConstructable::BlowUpEveryInstance = 5;
    {
        auto v = makeVector<SometimesConstructable>({7, 5, 3, 1});
        EXPECT_EQ(4, v.capacity());
        EXPECT_FALSE(v.empty());
        EXPECT_EQ(4, v.size());
        EXPECT_EQ(4, SometimesConstructable::InstanceCount);

        Array<SometimesConstructable> array = v.toArray();
        EXPECT_FALSE(array.empty());
        EXPECT_EQ(4, array.size());
        EXPECT_EQ(4, SometimesConstructable::InstanceCount);


        // vector 'v' has been moved from - thus should be 'empty'
        EXPECT_TRUE(v.empty());
        EXPECT_EQ(0, v.capacity());
        EXPECT_EQ(0, v.size());
    }

    // Important to make sure all the instances has been correctly destructed after scope exit
    ASSERT_EQ(0, SometimesConstructable::InstanceCount);
}
