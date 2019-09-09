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
*******************************************************************************/
#include <solace/version.hpp>			// Class being tested
#include <solace/exception.hpp>
#include <solace/vector.hpp>                    // For container req. test

#include <solace/output_utils.hpp>


#include <gtest/gtest.h>


using namespace Solace;


/**
 * Test implementation and contract of Comparable
 */
TEST(TestVersion, testComparable) {
    const Version v1(1, 2, 3);
    const Version v2(1, 2, 3);
    const Version v_different(3, 2, 3);
    const Version v2_same_same(1, 2, 3, "", "build.1321");

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
TEST(TestVersion, testSpecs) {
    {  // 1.0.0 < 2.0.0 < 2.1.0 < 2.1.1
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

    {  // 1.0.0-alpha < 1.0.0
        const Version v1(1, 0, 0);
        const Version v1_alpha(1, 0, 0, "rc.1.alpha");

        EXPECT_TRUE(v1_alpha < v1);
    }

    // 1.0.0-alpha < 1.0.0-alpha.1 < 1.0.0-alpha.beta < 1.0.0-beta < 1.0.0-beta.2 < 1.0.0-beta.11 < 1.0.0-rc.1 < 1.0.0
    {
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


TEST(TestVersion, testSpecs_ignoringMeta) {
    {  // 1.0.0 < 2.0.0 < 2.1.0 < 2.1.1
		const Version v1{1, 0, 0, StringLiteral{}, "Something"};
		const Version v2{2, 0, 0, StringLiteral{}, "3.2.1"};
		const Version v21{2, 1, 0, StringLiteral{}, "3333"};
		const Version v211{2, 1, 1, StringLiteral{}, "180.213"};

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

    {  // 1.0.0-alpha < 1.0.0
        const Version v1(1, 37, 0);
        const Version v1_alpha(1, 37, 0, "alpha", "betta");

        EXPECT_TRUE(v1_alpha < v1);
    }

    // 1.0.0-alpha < 1.0.0-alpha.1 < 1.0.0-alpha.beta < 1.0.0-beta < 1.0.0-beta.2 < 1.0.0-beta.11 < 1.0.0-rc.1 < 1.0.0
    {
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
	* Test implementation and contract of toString
	*/
TEST(TestVersion, testToString) {
    EXPECT_EQ(StringLiteral("3.2.1"), Version(3, 2, 1).toString());

    EXPECT_EQ(StringLiteral("2.0.5-alpha1"),
              Version(2, 0, 5, "alpha1").toString());

    EXPECT_EQ(StringLiteral("41.7.5-alpha1.something-awesome.31"),
              Version(41, 7, 5, "alpha1.something-awesome.31").toString());

    EXPECT_EQ(StringLiteral("33.1.8+20130313144700"),
			  Version(33, 1, 8, StringLiteral{}, "20130313144700").toString());

    EXPECT_EQ(StringLiteral("6.12.77+the.best.version-1"),
			  Version(6, 12, 77, StringLiteral{}, "the.best.version-1").toString());

    EXPECT_EQ(StringLiteral("1.13.7+the.best.version"),
			  Version(1, 13, 7, StringLiteral{}, "the.best.version").toString());

    EXPECT_EQ(StringLiteral("1.4.99-beta+exp.sha.5114f85"),
              Version(1, 4, 99, "beta", "exp.sha.5114f85").toString());

    EXPECT_EQ(StringLiteral("1.4.99-alpha.beta.rc-1+exp.sha.5114f85"),
              Version(1, 4, 99, "alpha.beta.rc-1", "exp.sha.5114f85").toString());
}

/**
	* Test implementation and contract of parsable
	*/
TEST(TestVersion, testParsing) {
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
		EXPECT_EQ(Version(33, 1, 8, StringLiteral{}, "20130313144700"), parseResult.unwrap());
    }

    {
        auto parseResult = Version::parse("6.12.77+the.best.version-rc1");
        EXPECT_TRUE(parseResult.isOk());
		EXPECT_EQ(Version(6, 12, 77, StringLiteral{}, "the.best.version-rc1"), parseResult.unwrap());
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
TEST(TestVersion, testParsing_and_ToString_are_consistent) {
    {
        auto const src = StringLiteral("0.4.9");
        auto parseResult = Version::parse(src);
        EXPECT_TRUE(parseResult.isOk());
        EXPECT_EQ(src, parseResult.unwrap().toString());
    }

    {
        auto const src = StringLiteral("1.4.99-beta+exp.sha.5114f85");
        auto parseResult = Version::parse(src);
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
TEST(TestVersion, testContainerReq) {
    constexpr static uint kTestSetSize = 32;
	auto maybeVec = makeVector<Version>(kTestSetSize);
	ASSERT_TRUE(maybeVec.isOk());

	Vector<Version>& versions = maybeVec.unwrap();

    for (uint i = 0; i < kTestSetSize; ++i) {
        versions.emplace_back(i, i % 3, 2*i + 1);
    }

    for (uint i = 0; i < kTestSetSize; ++i) {
        EXPECT_EQ(Version(i, i % 3, 2*i + 1), versions[i]);
    }

    versions.clear();
    for (uint i = 0; i < kTestSetSize; ++i) {
		versions.emplace_back(2*i + 1, i, i % 3, StringLiteral{}, "Some-tags");
    }

    for (uint i = 0; i < kTestSetSize; ++i) {
		EXPECT_EQ(Version(2*i + 1, i, i % 3, StringLiteral{}, "Some-tags"), versions[i]);
    }
}
