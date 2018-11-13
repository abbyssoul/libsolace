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
 *	@file test/test_error.cpp
 *	@brief		Test suit for Solace::Error
 ******************************************************************************/
#include <solace/error.hpp>    // Class being tested.

#include <gtest/gtest.h>
#include "mockTypes.hpp"

using namespace Solace;


TEST(TestError, constructor) {
    AtomValue testCategory = atom("test");
    Error v{testCategory, 1, "Test"};

    EXPECT_TRUE(v);
    EXPECT_EQ(testCategory, v.domain());
    EXPECT_EQ(1, v.value());
    EXPECT_EQ(StringLiteral("Test"), v.tag());
}
