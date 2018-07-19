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
 * @file: test/test_version.cpp
 * @author: soultaker
 *
 * Created on: 22 Apr 2015
*******************************************************************************/
#include <solace/version.hpp>			// Class being tested
#include <solace/exception.hpp>

#include <gtest/gtest.h>

#include <vector>

using namespace Solace;


class TestVersion : public ::testing::Test {

public:

    void setUp() {
	}

    void tearDown() {
	}
};
	/**
	 * Test implementation and contract of IComparable
	 */
TEST_F(TestVersion, testComparable) {
	const Version v1(1, 2, 3);
	const Version v2(1, 2, 3);
	const Version v_different(3, 2, 3);
	const Version v2_same_same(1, 2, 3, String::Empty, "build.1321");

	EXPECT_TRUE(v1.equals(v2));
	EXPECT_TRUE(v2.equals(v1));
	EXPECT_EQ(v1, v2);
	EXPECT_TRUE(v2 == v2);
	EXPECT_TRUE(v2 == v1);
	EXPECT_TRUE(v2 != v_different);

	EXPECT_EQ(false, v1.equals(v_different));
	EXPECT_EQ(false, v2.equals(v_different));
	EXPECT_EQ(false, v_different.equals(v1));
	EXPECT_EQ(false, v_different.equals(v2));

	EXPECT_TRUE(v1.equals(v2_same_same));
	EXPECT_TRUE(v2.equals(v2_same_same));

}

/**
	* Test version comparison rules as defined in specs
	*/
TEST_F(TestVersion, testSpecs) {
	{
		// 1.0.0 < 2.0.0 < 2.1.0 < 2.1.1
		const Version v1(1, 0, 0);
		const Version v2(2, 0, 0);
		const Version v21(2, 1, 0);
		const Version v211(2, 1, 1);

		EXPECT_TRUE(v1 < v2);
		EXPECT_TRUE(v2 < v21);
		EXPECT_TRUE(v21 < v211);
		// transitive
		EXPECT_TRUE(v1 < v211);
		EXPECT_TRUE(v2 < v211);

		EXPECT_TRUE(v2 > v1);
		EXPECT_TRUE(v21 > v2);
		EXPECT_TRUE(v211 > v21);
		// transitive
		EXPECT_TRUE(v211 > v2);
		EXPECT_TRUE(v211 > v1);
	}

	{
	// 1.0.0-alpha < 1.0.0
		const Version v1(1, 0, 0);
		const Version v1_alpha(1, 0, 0, "rc.1.alpha");

		EXPECT_TRUE(v1_alpha < v1);
	}

	{
	// 1.0.0-alpha < 1.0.0-alpha.1 < 1.0.0-alpha.beta < 1.0.0-beta < 1.0.0-beta.2 < 1.0.0-beta.11 < 1.0.0-rc.1 < 1.0.0
		const Version v1(1, 0, 7);
		const Version v1_alpha(1, 0, 7, "alpha");
		const Version v1_alpha1(1, 0, 7, "alpha.1");
		const Version v1_alpha_beta(1, 0, 7, "alpha.beta");
		const Version v1_beta(1, 0, 7, "beta");
		const Version v1_beta2(1, 0, 7, "beta.2");
		const Version v1_beta11(1, 0, 7, "beta.11");
		const Version v1_rc1(1, 0, 7, "rc.1");

		EXPECT_TRUE(v1_alpha < v1_alpha1);
		EXPECT_TRUE(v1_alpha1 < v1_alpha_beta);
		EXPECT_TRUE(v1_beta < v1_beta2);
		EXPECT_TRUE(v1_beta2 < v1_beta11);
		EXPECT_TRUE(v1_beta11 < v1_rc1);
		EXPECT_TRUE(v1_rc1 < v1);
	}
}


TEST_F(TestVersion, testSpecs_ignoringMeta) {
	{
		// 1.0.0 < 2.0.0 < 2.1.0 < 2.1.1
		const Version v1(1, 0, 0, String::Empty, "Something");
		const Version v2(2, 0, 0, String::Empty, "3.2.1");
		const Version v21(2, 1, 0, String::Empty, "3333");
		const Version v211(2, 1, 1, String::Empty, "180.213");

		EXPECT_TRUE(v1 < v2);
		EXPECT_TRUE(v2 < v21);
		EXPECT_TRUE(v21 < v211);
		// transitive
		EXPECT_TRUE(v2 < v211);
		EXPECT_TRUE(v1 < v211);

		EXPECT_TRUE(v2 > v1);
		EXPECT_TRUE(v21 > v2);
		EXPECT_TRUE(v211 > v21);
		// transitive
		EXPECT_TRUE(v211 > v2);
		EXPECT_TRUE(v211 > v1);
	}

	{
	// 1.0.0-alpha < 1.0.0
		const Version v1(1, 37, 0);
		const Version v1_alpha(1, 37, 0, "alpha", "betta");

		EXPECT_TRUE(v1_alpha < v1);
	}

	{
	// 1.0.0-alpha < 1.0.0-alpha.1 < 1.0.0-alpha.beta < 1.0.0-beta < 1.0.0-beta.2 < 1.0.0-beta.11 < 1.0.0-rc.1 < 1.0.0.
		const Version v1(1, 0, 0, "how.to.mess.it.123");
		const Version v1_alpha(1, 0, 0, "alpha");
		const Version v1_alpha1(1, 0, 0, "alpha.1");
		const Version v1_alpha_beta(1, 0, 0, "alpha.beta");
		const Version v1_beta(1, 0, 0, "beta", "12.asdf");
		const Version v1_beta2(1, 0, 0, "beta.2");
		const Version v1_beta11(1, 0, 0, "beta.11", "12.asdf");
		const Version v1_rc1(1, 0, 0, "rc.1");

		EXPECT_TRUE(v1_alpha < v1_alpha1);
		EXPECT_TRUE(v1_alpha1 < v1_alpha_beta);
		EXPECT_TRUE(v1_beta < v1_beta2);
		EXPECT_TRUE(v1_beta2 < v1_beta11);
		EXPECT_TRUE(v1_beta11 < v1_rc1);
		EXPECT_TRUE(v1_rc1 < v1);
	}
}

/**
	* Test implementation and contract of IFormattable
	*/
TEST_F(TestVersion, testToString) {
	{
		const Version v(3, 2, 1);
		EXPECT_EQ(String("3.2.1"), v.toString());
	}

	{
		const Version v(2, 0, 5, "alpha1");
		EXPECT_EQ(String("2.0.5-alpha1"), v.toString());
	}

	{
		const Version v(41, 7, 5, "alpha1.something-awesome.31");
		EXPECT_EQ(String("41.7.5-alpha1.something-awesome.31"), v.toString());
	}

	{
		const Version v(33, 1, 8, String::Empty, "20130313144700");
		EXPECT_EQ(String("33.1.8+20130313144700"), v.toString());
	}
	{
		const Version v(6, 12, 77, String::Empty, "the.best.version-1");
		EXPECT_EQ(String("6.12.77+the.best.version-1"), v.toString());
	}

	{
		const Version v(1, 13, 7, String::Empty, "the.best.version");
		EXPECT_EQ(String("1.13.7+the.best.version"), v.toString());
	}

	{
		const Version v(1, 4, 99, "beta", "exp.sha.5114f85");
		EXPECT_EQ(String("1.4.99-beta+exp.sha.5114f85"), v.toString());
	}
	{
		const Version v(1, 4, 99, "alpha.beta.rc-1", "exp.sha.5114f85");
		EXPECT_EQ(String("1.4.99-alpha.beta.rc-1+exp.sha.5114f85"), v.toString());
	}
}

/**
	* Test implementation and contract of parsable
	*/
TEST_F(TestVersion, testParsing) {
	{
		auto parseResult = Version::parse("3.231.1");
		EXPECT_TRUE(parseResult.isOk());
		EXPECT_EQ(Version(3, 231, 1), parseResult.unwrap());
	}

	{
		auto parseResult = Version::parse("2.0.5-alpha1");
		EXPECT_TRUE(parseResult.isOk());
		EXPECT_EQ(Version(2, 0, 5, "alpha1"), parseResult.unwrap());
	}

	{
		auto parseResult = Version::parse("233.1076.532-alpha1.something-awesome");
		EXPECT_TRUE(parseResult.isOk());
		EXPECT_EQ(Version(233, 1076, 532, "alpha1.something-awesome"), parseResult.unwrap());
	}

	{
		auto parseResult = Version::parse("33.1.8+20130313144700");
		EXPECT_TRUE(parseResult.isOk());
		EXPECT_EQ(Version(33, 1, 8, String::Empty, "20130313144700"), parseResult.unwrap());
	}
	{
		auto parseResult = Version::parse("6.12.77+the.best.version-rc1");
		EXPECT_TRUE(parseResult.isOk());
		EXPECT_EQ(Version(6, 12, 77, String::Empty, "the.best.version-rc1"), parseResult.unwrap());
	}
	{
		auto parseResult = Version::parse("1.4.99-beta+exp.sha-5114f85");
		EXPECT_TRUE(parseResult.isOk());
		EXPECT_EQ(Version(1, 4, 99, "beta", "exp.sha-5114f85"), parseResult.unwrap());
	}
	{
		auto parseResult = Version::parse("1.13.7-alpha1.betta+the-best.version");
		EXPECT_TRUE(parseResult.isOk());
		EXPECT_EQ(Version(1, 13, 7, "alpha1.betta", "the-best.version"), parseResult.unwrap());
	}

	{
		EXPECT_TRUE(Version::parse("x3+Bingo").isError());
		EXPECT_TRUE(Version::parse("3.+Bingo").isError());
		EXPECT_TRUE(Version::parse("3.1-+Bingo").isError());
	}

}

/**
	* Test consistency of parsing and toString implementation
	*/
TEST_F(TestVersion, testParsing_and_ToString_are_consistent) {
	{
		const String src("0.4.9");
		auto parseResult = Version::parse(src.view());
		EXPECT_TRUE(parseResult.isOk());
		EXPECT_EQ(src, parseResult.unwrap().toString());
	}

	{
		const String src("1.4.99-beta+exp.sha.5114f85");
		auto parseResult = Version::parse(src.view());
		EXPECT_TRUE(parseResult.isOk());
		EXPECT_EQ(src, parseResult.unwrap().toString());
	}

	{
		const Version v(37, 4, 9, "something.pre.els", "2475-betta.soon");
		const String src = v.toString();
		auto parseResult = Version::parse(src.view());
		EXPECT_TRUE(parseResult.isOk());
		EXPECT_EQ(v, parseResult.unwrap());
	}
}

/**
	* Test consistency of parsing and toString implementation
*/
TEST_F(TestVersion, testContainerReq) {
	std::vector<Version> versions;

	for (uint i = 0; i < 32; ++i) {
		versions.emplace_back(i, i % 3, 2*i + 1);
	}

	for (uint i = 0; i < 32; ++i) {
		EXPECT_EQ(Version(i, i % 3, 2*i + 1), versions[i]);
	}

	for (uint i = 0; i < 32; ++i) {
		versions[i] = Version(2*i + 1, i, i % 3, String::Empty, "Some-tags");
	}

	for (uint i = 0; i < 32; ++i) {
		EXPECT_EQ(Version(2*i + 1, i, i % 3, String::Empty, "Some-tags"), versions[i]);
	}
}
