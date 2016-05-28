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
#include <cppunit/extensions/HelperMacros.h>

#include <solace/exception.hpp>
#include <vector>   // FIXME: We should test with Solace::Array

using namespace Solace;


class TestVersion : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestVersion);
        CPPUNIT_TEST(testComparable);
        CPPUNIT_TEST(testSpecs);
        CPPUNIT_TEST(testSpecs_ignoringMeta);
        CPPUNIT_TEST(testToString);
        CPPUNIT_TEST(testParsing);
        CPPUNIT_TEST(testParsing_and_ToString_are_consistent);
        CPPUNIT_TEST(testContainerReq);
	CPPUNIT_TEST_SUITE_END();

public:

	/**
	 * Test implementation and contract of IComparable
	 */
	void testComparable() {
        const Version v1(1, 2, 3);
        const Version v2(1, 2, 3);
        const Version v_different(3, 2, 3);
        const Version v2_same_same(1, 2, 3, String::Empty, "build.1321");

        CPPUNIT_ASSERT(v1.equals(v2));
        CPPUNIT_ASSERT(v2.equals(v1));
		CPPUNIT_ASSERT_EQUAL(v1, v2);
        CPPUNIT_ASSERT(v2 == v2);
        CPPUNIT_ASSERT(v2 == v1);
        CPPUNIT_ASSERT(v2 != v_different);

        CPPUNIT_ASSERT_EQUAL(false, v1.equals(v_different));
        CPPUNIT_ASSERT_EQUAL(false, v2.equals(v_different));
        CPPUNIT_ASSERT_EQUAL(false, v_different.equals(v1));
        CPPUNIT_ASSERT_EQUAL(false, v_different.equals(v2));

        CPPUNIT_ASSERT(v1.equals(v2_same_same));
        CPPUNIT_ASSERT(v2.equals(v2_same_same));

	}

	/**
	 * Test version comparison rules as defined in specs
	 */
	void testSpecs() {
		{
            // 1.0.0 < 2.0.0 < 2.1.0 < 2.1.1
			const Version v1(1, 0, 0);
			const Version v2(2, 0, 0);
			const Version v21(2, 1, 0);
			const Version v211(2, 1, 1);

            CPPUNIT_ASSERT(v1 < v2);
            CPPUNIT_ASSERT(v2 < v21);
            CPPUNIT_ASSERT(v21 < v211);
			// transitive
            CPPUNIT_ASSERT(v1 < v211);
            CPPUNIT_ASSERT(v2 < v211);

            CPPUNIT_ASSERT(v2 > v1);
            CPPUNIT_ASSERT(v21 > v2);
            CPPUNIT_ASSERT(v211 > v21);
			// transitive
            CPPUNIT_ASSERT(v211 > v2);
            CPPUNIT_ASSERT(v211 > v1);
		}

		{
		// 1.0.0-alpha < 1.0.0
			const Version v1(1, 0, 0);
			const Version v1_alpha(1, 0, 0, "rc.1.alpha");

            CPPUNIT_ASSERT(v1_alpha < v1);
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

            CPPUNIT_ASSERT(v1_alpha < v1_alpha1);
            CPPUNIT_ASSERT(v1_alpha1 < v1_alpha_beta);
            CPPUNIT_ASSERT(v1_beta < v1_beta2);
            CPPUNIT_ASSERT(v1_beta2 < v1_beta11);
            CPPUNIT_ASSERT(v1_beta11 < v1_rc1);
            CPPUNIT_ASSERT(v1_rc1 < v1);
		}
	}


	void testSpecs_ignoringMeta() {
		{
            // 1.0.0 < 2.0.0 < 2.1.0 < 2.1.1
			const Version v1(1, 0, 0, String::Empty, "Something");
			const Version v2(2, 0, 0, String::Empty, "3.2.1");
			const Version v21(2, 1, 0, String::Empty, "3333");
			const Version v211(2, 1, 1, String::Empty, "180.213");

            CPPUNIT_ASSERT(v1 < v2);
            CPPUNIT_ASSERT(v2 < v21);
            CPPUNIT_ASSERT(v21 < v211);
			// transitive
            CPPUNIT_ASSERT(v2 < v211);
            CPPUNIT_ASSERT(v1 < v211);

            CPPUNIT_ASSERT(v2 > v1);
            CPPUNIT_ASSERT(v21 > v2);
            CPPUNIT_ASSERT(v211 > v21);
			// transitive
            CPPUNIT_ASSERT(v211 > v2);
            CPPUNIT_ASSERT(v211 > v1);
		}

		{
		// 1.0.0-alpha < 1.0.0
			const Version v1(1, 37, 0);
			const Version v1_alpha(1, 37, 0, "alpha", "betta");

            CPPUNIT_ASSERT(v1_alpha < v1);
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

            CPPUNIT_ASSERT(v1_alpha < v1_alpha1);
            CPPUNIT_ASSERT(v1_alpha1 < v1_alpha_beta);
            CPPUNIT_ASSERT(v1_beta < v1_beta2);
            CPPUNIT_ASSERT(v1_beta2 < v1_beta11);
            CPPUNIT_ASSERT(v1_beta11 < v1_rc1);
            CPPUNIT_ASSERT(v1_rc1 < v1);
		}
	}

	/**
	 * Test implementation and contract of IFormattable
	 */
	void testToString() {
		{
			const Version v(3, 2, 1);
            CPPUNIT_ASSERT_EQUAL(String("3.2.1"), v.toString());
		}

		{
			const Version v(2, 0, 5, "alpha1");
            CPPUNIT_ASSERT_EQUAL(String("2.0.5-alpha1"), v.toString());
		}

		{
			const Version v(41, 7, 5, "alpha1.something-awesome.31");
            CPPUNIT_ASSERT_EQUAL(String("41.7.5-alpha1.something-awesome.31"), v.toString());
		}

		{
			const Version v(33, 1, 8, String::Empty, "20130313144700");
            CPPUNIT_ASSERT_EQUAL(String("33.1.8+20130313144700"), v.toString());
		}
		{
			const Version v(6, 12, 77, String::Empty, "the.best.version-1");
            CPPUNIT_ASSERT_EQUAL(String("6.12.77+the.best.version-1"), v.toString());
		}

		{
			const Version v(1, 13, 7, String::Empty, "the.best.version");
            CPPUNIT_ASSERT_EQUAL(String("1.13.7+the.best.version"), v.toString());
		}

		{
			const Version v(1, 4, 99, "beta", "exp.sha.5114f85");
            CPPUNIT_ASSERT_EQUAL(String("1.4.99-beta+exp.sha.5114f85"), v.toString());
		}
		{
			const Version v(1, 4, 99, "alpha.beta.rc-1", "exp.sha.5114f85");
            CPPUNIT_ASSERT_EQUAL(String("1.4.99-alpha.beta.rc-1+exp.sha.5114f85"), v.toString());
		}
	}

	/**
	 * Test implementation and contract of parsable
	 */
	void testParsing() {
		{
			const Version v = Version::parse("3.231.1");
            CPPUNIT_ASSERT_EQUAL(Version(3, 231, 1), v);
		}

		{
			const Version v = Version::parse("2.0.5-alpha1");
            CPPUNIT_ASSERT_EQUAL(Version(2, 0, 5, "alpha1"), v);
		}

		{
			const Version v = Version::parse("233.1076.532-alpha1.something-awesome");
            CPPUNIT_ASSERT_EQUAL(Version(233, 1076, 532, "alpha1.something-awesome"), v);
		}

		{
			const Version v = Version::parse("33.1.8+20130313144700");
			CPPUNIT_ASSERT_EQUAL(Version(33, 1, 8, String::Empty, "20130313144700"), v);
		}
		{
			const Version v = Version::parse("6.12.77+the.best.version-rc1");
			CPPUNIT_ASSERT_EQUAL(Version(6, 12, 77, String::Empty, "the.best.version-rc1"), v);
		}
		{
			const Version v = Version::parse("1.4.99-beta+exp.sha-5114f85");
			CPPUNIT_ASSERT_EQUAL(Version(1, 4, 99, "beta", "exp.sha-5114f85"), v);
		}
		{
			const Version v = Version::parse("1.13.7-alpha1.betta+the-best.version");
			CPPUNIT_ASSERT_EQUAL(Version(1, 13, 7, "alpha1.betta", "the-best.version"), v);
		}
	}

	/**
	 * Test consistency of parsing and toString implementation
	 */
	void testParsing_and_ToString_are_consistent() {
		{
			const String src("0.4.9");
			const Version v = Version::parse(src);
			CPPUNIT_ASSERT_EQUAL(src, v.toString());
		}

		{
			const String src("1.4.99-beta+exp.sha.5114f85");
			const Version v = Version::parse(src);
			CPPUNIT_ASSERT_EQUAL(src, v.toString());
		}

		{
            const Version v(37, 4, 9, "something.pre.els", "2475-betta.soon");
			const String src = v.toString();
			CPPUNIT_ASSERT_EQUAL(v, Version::parse(src));
		}
	}

	/**
     * Test consistency of parsing and toString implementation
    */
	void testContainerReq() {
        std::vector<Version> versions;

        for (uint i = 0; i < 32; ++i) {
            versions.push_back({i, i % 3, 2*i + 1});
        }

        for (uint i = 0; i < 32; ++i) {
            CPPUNIT_ASSERT_EQUAL(Version(i, i % 3, 2*i + 1), versions[i]);
        }

//        versions.clear();
        for (uint i = 0; i < 32; ++i) {
            versions[i] = Version(2*i + 1, i, i % 3, String::Empty, "Some-tags");
        }

        for (uint i = 0; i < 32; ++i) {
            CPPUNIT_ASSERT_EQUAL(Version(2*i + 1, i, i % 3, String::Empty, "Some-tags"), versions[i]);
        }

    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestVersion);
