/*
*  Copyright 2018 Ivan Ryabov
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
 * dialstring Unit Test Suit
 *	@file test/test_dialstring.cpp
 *	@brief		Test suit for styxe::DialString
 ******************************************************************************/
#include <solace/dialstring.hpp>    // Class being tested.

#include <gtest/gtest.h>

using namespace Solace;


TEST(TestDialString, testParsing_addressOnly) {
    auto res = tryParseDailString("filename");
	ASSERT_TRUE(res.isOk());
	EXPECT_EQ(kProtocolNone, res.unwrap().protocol);
	EXPECT_EQ("filename", res.unwrap().address);
	EXPECT_TRUE(res.unwrap().service.empty());
}


TEST(TestDialString, testParsingServiceOnly) {
	auto res = tryParseDailString("::http");
	ASSERT_TRUE(res.isOk());
	EXPECT_EQ(kProtocolNone, res.unwrap().protocol);
	EXPECT_EQ("", res.unwrap().address);
	EXPECT_EQ("http", res.unwrap().service);
}


TEST(TestDialString, testParsing_addressAndProtocol) {
    auto res = tryParseDailString("sctp:10.3.2.1");
    ASSERT_TRUE(res.isOk());
	EXPECT_EQ(kProtocolSCTP, res.unwrap().protocol);
    EXPECT_EQ("10.3.2.1", res.unwrap().address);
    EXPECT_TRUE(res.unwrap().service.empty());
}

TEST(TestDialString, testParsing) {
    auto res = tryParseDailString("udp:10.3.2.1:54321");
    ASSERT_TRUE(res.isOk());
	EXPECT_EQ(kProtocolUDP, res.unwrap().protocol);
    EXPECT_EQ("10.3.2.1", res.unwrap().address);
    EXPECT_EQ("54321", res.unwrap().service);
}

TEST(TestDialString, testParsingUnix) {
    auto res = tryParseDailString("unix:/dev/null");
    ASSERT_TRUE(res.isOk());
	EXPECT_EQ(kProtocolUnix, res.unwrap().protocol);
    EXPECT_EQ("/dev/null", res.unwrap().address);
    EXPECT_TRUE(res.unwrap().service.empty());
}

TEST(TestDialString, testParsingProtocolOnly) {
    auto res = tryParseDailString("blah:");
    ASSERT_TRUE(res.isOk());
	EXPECT_EQ(atom("blah"), res.unwrap().protocol);
    EXPECT_EQ("", res.unwrap().address);
    EXPECT_TRUE(res.unwrap().service.empty());
}

TEST(TestDialString, testParsingNonAtomProtocol) {
	auto res = tryParseDailString("somelongvalue:87212");
	ASSERT_TRUE(res.isError());
}
