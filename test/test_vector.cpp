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
    Vector<uint32> v{};

    EXPECT_TRUE(v.empty());
	EXPECT_EQ(0U, v.size());
	EXPECT_EQ(0U, v.capacity());
}


TEST(TestVector, testEmptyVectorIsEmpty) {
    // Test pre-condition
    ASSERT_EQ(0, SimpleType::InstanceCount);

    Vector<SimpleType> v;

    EXPECT_TRUE(v.empty());
	EXPECT_EQ(0U, v.size());
	EXPECT_EQ(0U, v.capacity());
    EXPECT_EQ(0, SimpleType::InstanceCount);
}

TEST(TestVector, factoryIntergralVectorWithCapacity) {
	auto maybeVec = makeVector<int32>(10);
	ASSERT_TRUE(maybeVec.isOk());
	auto& v = maybeVec.unwrap();

	EXPECT_EQ(10U, v.capacity());
    EXPECT_TRUE(v.empty());
	EXPECT_EQ(0U, v.size());
}

TEST(TestVector, factoryVectorWithCapacityCreatesNoObjects) {
    ASSERT_EQ(0, SimpleType::InstanceCount);
	auto maybeVec = makeVector<SimpleType>(10);
	ASSERT_TRUE(maybeVec.isOk());
	auto& v = maybeVec.unwrap();

	EXPECT_EQ(10U, v.capacity());
    EXPECT_TRUE(v.empty());
	EXPECT_EQ(0U, v.size());
    EXPECT_EQ(0, SimpleType::InstanceCount);
}


TEST(TestVector, emptyVectorEmplaceFails) {
	ASSERT_TRUE(Vector<int32>().emplace_back(212).isError());
}

TEST(TestVector, emplaceBackResults) {
	ASSERT_EQ(0, SimpleType::InstanceCount);
	{
		auto maybeVec = makeVector<SimpleType>(2);
		ASSERT_TRUE(maybeVec.isOk());
		auto& v = maybeVec.unwrap();

		{
			auto maybeValue = v.emplace_back(3, 2, 1);
			ASSERT_TRUE(maybeValue.isOk());
			EXPECT_EQ(2, maybeValue.unwrap().y);
			EXPECT_EQ(1, maybeValue.unwrap().z);

			EXPECT_FALSE(v.empty());
			EXPECT_EQ(2U, v.capacity());
			EXPECT_EQ(1U, v.size());
			EXPECT_EQ(1, SimpleType::InstanceCount);
		}

		EXPECT_FALSE(v.empty());
		EXPECT_EQ(2U, v.capacity());
		EXPECT_EQ(1U, v.size());
		EXPECT_EQ(1, SimpleType::InstanceCount);
	}

	// Important to make sure all the instances has been correctly destructed after scope exit
	ASSERT_EQ(0, SimpleType::InstanceCount);
}


TEST(TestVector, emplaceBack) {
    ASSERT_EQ(0, SimpleType::InstanceCount);
    {
		auto maybeVec = makeVector<SimpleType>(10);
		ASSERT_TRUE(maybeVec.isOk());
		auto& v = maybeVec.unwrap();

        v.emplace_back(3, 2, 1);
        v.emplace_back(2, 1, 0);
        v.emplace_back(1, 0, -1);

		EXPECT_EQ(10U, v.capacity());
        EXPECT_FALSE(v.empty());
		EXPECT_EQ(3U, v.size());
        EXPECT_EQ(3, SimpleType::InstanceCount);
    }

    // Important to make sure all the instances has been correctly destructed after scope exit
    ASSERT_EQ(0, SimpleType::InstanceCount);
}


TEST(TestVector, emplaceOverflowThrows) {
    ASSERT_EQ(0, SimpleType::InstanceCount);
    {
		auto maybeVec = makeVector<SimpleType>(3);
		ASSERT_TRUE(maybeVec.isOk());
		auto& v = maybeVec.unwrap();

		EXPECT_EQ(3U, v.capacity());

		EXPECT_TRUE(v.emplace_back(3, 2, 1).isOk());
		EXPECT_TRUE(v.emplace_back(2, 1, 0).isOk());
		EXPECT_TRUE(v.emplace_back(1, 0, -1).isOk());

		EXPECT_EQ(3U, v.size());
		EXPECT_FALSE(v.emplace_back(1, 0, -1).isOk());

        EXPECT_EQ(3, SimpleType::InstanceCount);
    }

    // Important to make sure all the instances has been correctly destructed after scope exit
    ASSERT_EQ(0, SimpleType::InstanceCount);
}


TEST(TestVector, movedFromVectorIsEmpty) {
    ASSERT_EQ(0, SimpleType::InstanceCount);
    {
		auto maybeVec = makeVector<SimpleType>(10);
		ASSERT_TRUE(maybeVec.isOk());
		auto& v = maybeVec.unwrap();

        v.emplace_back(3, 2, 1);
        v.emplace_back(2, 1, 0);
        v.emplace_back(1, 0, -1);

		EXPECT_EQ(10U, v.capacity());
        EXPECT_FALSE(v.empty());
		EXPECT_EQ(3U, v.size());
        EXPECT_EQ(3, SimpleType::InstanceCount);

		auto other = mv(v);
		EXPECT_EQ(0U, v.capacity());
        EXPECT_TRUE(v.empty());

		EXPECT_EQ(10U, other.capacity());
        EXPECT_FALSE(other.empty());
		EXPECT_EQ(3U, other.size());
        EXPECT_EQ(3, SimpleType::InstanceCount);
    }

    // Important to make sure all the instances has been correctly destructed after scope exit
    EXPECT_EQ(0, SimpleType::InstanceCount);
}



TEST(TestVector, movingWhenCopyConstructorThrowsIsSafe) {
    SometimesConstructable::BlowUpEveryInstance = 6;
    ASSERT_EQ(0, SometimesConstructable::InstanceCount);

	auto maybeVec = makeVector<SometimesConstructable>(10);
	ASSERT_TRUE(maybeVec.isOk());
	auto& v = maybeVec.unwrap();

    EXPECT_TRUE(v.empty());
	EXPECT_EQ(0U, v.size());
	EXPECT_EQ(10U, v.capacity());
    EXPECT_EQ(0, SometimesConstructable::InstanceCount);

	Vector<SometimesConstructable> const movedInto = mv(v);

    EXPECT_TRUE(v.empty());
	EXPECT_EQ(0U, v.size());
	EXPECT_EQ(0U, v.capacity());
    EXPECT_EQ(0, SometimesConstructable::InstanceCount);
}


TEST(TestVector, constructionFromInitializerList) {
    ASSERT_EQ(0, SimpleType::InstanceCount);
    {
		auto maybeVec = makeVectorOf<SimpleType>({
                                            {3, 2, 1},
                                            {2, 1, 0},
                                            {1, 0, -1}
                                        });
		ASSERT_TRUE(maybeVec.isOk());
		auto& v = maybeVec.unwrap();

        EXPECT_FALSE(v.empty());
		EXPECT_EQ(3U, v.capacity());
		EXPECT_EQ(3U, v.size());
        EXPECT_EQ(3, SimpleType::InstanceCount);
    }

    // Important to make sure all the instances has been correctly destructed after scope exit
    EXPECT_EQ(0, SimpleType::InstanceCount);
}



TEST(TestVector, constructionFromVarArgs) {
    ASSERT_EQ(0, PimitiveType::InstanceCount);
    {
		auto maybeVec = makeVectorOf<PimitiveType>(3, 2, 1);
		ASSERT_TRUE(maybeVec.isOk());
		auto& v = maybeVec.unwrap();

        EXPECT_FALSE(v.empty());
		EXPECT_EQ(3U, v.capacity());
		EXPECT_EQ(3U, v.size());
        EXPECT_EQ(3, PimitiveType::InstanceCount);
    }

    // Important to make sure all the instances has been correctly destructed after scope exit
    EXPECT_EQ(0, PimitiveType::InstanceCount);
}


TEST(TestVector, copy) {
    ASSERT_EQ(0, SimpleType::InstanceCount);
    {
        auto const origin = makeVectorOf<SimpleType>({
                                            {3, 2, 1},
                                            {2, 1, 0},
                                            {1, 0, -1}
                                        });
		ASSERT_TRUE(origin.isOk());
        EXPECT_EQ(3, SimpleType::InstanceCount);

		auto maybeVec = makeVector(*origin);  // Make a copy
		ASSERT_TRUE(maybeVec.isOk());
		auto& v = maybeVec.unwrap();

        EXPECT_FALSE(v.empty());
		EXPECT_EQ(3U, v.capacity());
		EXPECT_EQ(3U, v.size());
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
		auto const maybeVec = makeVectorOf<SimpleType>({
                                            {3, 2, 1},
                                            {2, 1, 0},
                                            {1, 0, -1}
                                        });
		ASSERT_TRUE(maybeVec.isOk());
		auto& v = maybeVec.unwrap();

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
		auto maybeVec = makeVectorOf<SimpleType>({
                                            {3, 2, 1},
                                            {2, 1, 0},
                                            {1, 0, -1}
                                        });
		ASSERT_TRUE(maybeVec.isOk());
		auto& v = maybeVec.unwrap();

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
		auto maybeVec = makeVector<SimpleType>(10);
		ASSERT_TRUE(maybeVec.isOk());
		auto& v = maybeVec.unwrap();

        v.emplace_back(3, 2, 1);
        v.emplace_back(2, 1, 0);
        v.emplace_back(1, 0, -1);
        v.emplace_back(2, 1, 0);
        v.emplace_back(3, 2, 1);

		EXPECT_EQ(10U, v.capacity());
        EXPECT_FALSE(v.empty());
		EXPECT_EQ(5U, v.size());
        EXPECT_EQ(5, SimpleType::InstanceCount);

        EXPECT_NO_THROW(v.pop_back());
		EXPECT_EQ(4U, v.size());
        EXPECT_EQ(4, SimpleType::InstanceCount);
        EXPECT_EQ(SimpleType(2, 1, 0), v[3]);

        EXPECT_NO_THROW(v.pop_back());
		EXPECT_EQ(3U, v.size());
        EXPECT_EQ(3, SimpleType::InstanceCount);
        EXPECT_EQ(SimpleType(1, 0, -1), v[2]);
    }

    // Important to make sure all the instances has been correctly destructed after scope exit
    ASSERT_EQ(0, SimpleType::InstanceCount);
}


TEST(TestVector, clear) {
    ASSERT_EQ(0, SimpleType::InstanceCount);
    {
		auto maybeVec = makeVector<SimpleType>(10);
		ASSERT_TRUE(maybeVec.isOk());
		auto& v = maybeVec.unwrap();

        v.emplace_back(3, 2, 1);
        v.emplace_back(2, 1, 0);
        v.emplace_back(1, 0, -1);
        v.emplace_back(2, 1, 0);
        v.emplace_back(3, 2, 1);

		EXPECT_EQ(10U, v.capacity());
        EXPECT_FALSE(v.empty());
		EXPECT_EQ(5U, v.size());
        EXPECT_EQ(5, SimpleType::InstanceCount);

        EXPECT_NO_THROW(v.clear());
		EXPECT_EQ(0U, v.size());
        EXPECT_EQ(0, SimpleType::InstanceCount);
    }

    // Important to make sure all the instances has been correctly destructed after scope exit
    ASSERT_EQ(0, SimpleType::InstanceCount);
}


TEST(TestVector, moveOnlyTypes) {
    ASSERT_EQ(0, MoveOnlyType::InstanceCount);
    {
		auto maybeVec = makeVector<MoveOnlyType>(10);
		ASSERT_TRUE(maybeVec.isOk());
		auto& v = maybeVec.unwrap();

		EXPECT_EQ(10U, v.capacity());
        EXPECT_TRUE(v.empty());
		EXPECT_EQ(0U, v.size());
        EXPECT_EQ(0, MoveOnlyType::InstanceCount);

        v.emplace_back(3);
        v.emplace_back(2);
        v.emplace_back(1);
        v.emplace_back(0);
        v.emplace_back(-1);

		EXPECT_EQ(10U, v.capacity());
        EXPECT_FALSE(v.empty());
		EXPECT_EQ(5U, v.size());
        EXPECT_EQ(5, MoveOnlyType::InstanceCount);

        int counter = 3;
        for (auto& a : v) {
            EXPECT_EQ(counter, a.x_);
            counter -= 1;
        }

        EXPECT_NO_THROW(v.pop_back());
		EXPECT_EQ(4U, v.size());
        EXPECT_EQ(4, MoveOnlyType::InstanceCount);
        EXPECT_EQ(0, v[3].x_);

        EXPECT_NO_THROW(v.pop_back());
		EXPECT_EQ(3U, v.size());
        EXPECT_EQ(3, MoveOnlyType::InstanceCount);
        EXPECT_EQ(1, v[2].x_);
    }

    // Important to make sure all the instances has been correctly destructed after scope exit
    ASSERT_EQ(0, MoveOnlyType::InstanceCount);
}

TEST(TestVector, toArray_MoveOnlyType) {
    ASSERT_EQ(0, MoveOnlyType::InstanceCount);
    {
		auto maybeVec = makeVector<MoveOnlyType>(4);
		ASSERT_TRUE(maybeVec.isOk());
		auto& v = maybeVec.unwrap();

        v.emplace_back(7);
        v.emplace_back(5);
        v.emplace_back(3);
        v.emplace_back(1);
		EXPECT_EQ(4U, v.capacity());
        EXPECT_FALSE(v.empty());
		EXPECT_EQ(4U, v.size());
        EXPECT_EQ(4, MoveOnlyType::InstanceCount);

        Array<MoveOnlyType> array = v.toArray();
        EXPECT_FALSE(array.empty());
		EXPECT_EQ(4U, array.size());
        EXPECT_EQ(4, MoveOnlyType::InstanceCount);

        // vector 'v' has been moved from - thus should be 'empty'
        EXPECT_TRUE(v.empty());
		EXPECT_EQ(0U, v.capacity());
		EXPECT_EQ(0U, v.size());
    }

    // Important to make sure all the instances has been correctly destructed after scope exit
    ASSERT_EQ(0, MoveOnlyType::InstanceCount);
}

TEST(TestVector, toArray) {
    ASSERT_EQ(0, SometimesConstructable::InstanceCount);
    SometimesConstructable::BlowUpEveryInstance = 4*2 + 1;
    {
		auto maybeVec = makeVectorOf<SometimesConstructable>(7, 5, 3, 1);
		ASSERT_TRUE(maybeVec.isOk());
		auto& v = maybeVec.unwrap();

		EXPECT_EQ(4U, v.capacity());
        EXPECT_FALSE(v.empty());
		EXPECT_EQ(4U, v.size());
        EXPECT_EQ(4, SometimesConstructable::InstanceCount);

        Array<SometimesConstructable> array = v.toArray();
        EXPECT_EQ(4, SometimesConstructable::InstanceCount);
        EXPECT_FALSE(array.empty());
		EXPECT_EQ(4U, array.size());


        // vector 'v' has been moved from - thus should be 'empty'
        EXPECT_TRUE(v.empty());
		EXPECT_EQ(0U, v.capacity());
		EXPECT_EQ(0U, v.size());
    }

    // Important to make sure all the instances has been correctly destructed after scope exit
    ASSERT_EQ(0, SometimesConstructable::InstanceCount);
}
