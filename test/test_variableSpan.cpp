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
 *	@file test/test_variableSpan.cpp
 *	@brief		Test set of Solace::VariableSpan
 ******************************************************************************/
#include <solace/variableSpan.hpp>    // Class being tested.

#include <solace/stringView.hpp>
#include <solace/byteWriter.hpp>

#include <gtest/gtest.h>
#include "mockTypes.hpp"

using namespace Solace;


TEST(VariableSpan, emptySpan) {

	byte buffer[1];
	auto data = wrapMemory(buffer);

	VariableSpan<StringView> v{0, data};

	ASSERT_EQ(v.size(), 0);
	ASSERT_EQ(v.empty(), true);
	ASSERT_EQ(v.begin(), v.end());
}


TEST(VariableSpan, singleElementSpan) {

	byte buffer[32];
	auto data = wrapMemory(buffer);
	StringLiteral message{"hello"};
	ByteWriter writer{data};
	writer.write(message.size());
	writer.write(message.view());

	VariableSpan<StringView> v{1, data};

	ASSERT_EQ(v.size(), 1);
	ASSERT_EQ(v.empty(), false);

	auto it = v.begin();
	ASSERT_EQ(*it, message);
}


TEST(VariableSpan, varElementSpan) {

	byte buffer[128];
	auto data = wrapMemory(buffer);

	ByteWriter writer{data};
	StringLiteral messages[] = {{"one"}, {"world"}, {"hello"}};
	for (auto message : messages) {
		writer.write(message.size());
		writer.write(message.view());
	}

	VariableSpan<StringView> v{3, data};

	ASSERT_EQ(v.size(), 3);
	ASSERT_EQ(v.empty(), false);

	uint16 index = 0;
	for (auto i : v) {
		ASSERT_EQ(i, messages[index]);
		index++;
	}
}
