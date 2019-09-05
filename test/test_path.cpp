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
 * @file: test/test_path.cpp
*******************************************************************************/
#include <solace/path.hpp>			// Class being tested
#include <solace/exception.hpp>     // Checked expcetions


#include "mockTypes.hpp"
#include <gtest/gtest.h>
#include <cstring>

#include <ostream>

using namespace Solace;


std::ostream& operator<< (std::ostream& ostr, Solace::Path const& v) {
    ostr << "Path:" << v.getComponentsCount() << ":{";
    for (auto const& c : v) {
        auto const componentView = c.view();
        ostr.write(componentView.data(), componentView.size());
        ostr << " ";
    }

    ostr << "}";
    return ostr;
}


template<typename...Args>
Path makePathSafe(const char* p, Args&&...args) {
	auto maybePath = makePath(p, std::forward<Args>(args)...);
	return maybePath.moveResult();
}

template<typename...Args>
Path makePathSafe(Path const& p, Args&&...args) {
	auto maybePath = makePath(p, std::forward<Args>(args)...);
	return maybePath.moveResult();
}


TEST(TestPath, testRootIsSingleComponent) {
    EXPECT_EQ(1, Path::Root.getComponentsCount());
}

TEST(TestPath, testRootAbsolute) {
    EXPECT_TRUE(Path::Root.isAbsolute());
}

TEST(TestPath, defaultConstructedPathIsEmpty) {
	EXPECT_TRUE(Path{}.empty());
	EXPECT_EQ(0, Path{}.length());
	EXPECT_EQ(0, Path{}.getComponentsCount());
}

TEST(TestPath, nonEmptyPathIsNotEmpty) {
	auto p = makePath("file");
	ASSERT_TRUE(p.isOk());
	EXPECT_FALSE(p.unwrap().empty());

	auto p2 = makePath("some", "path", "to", "file");
	ASSERT_TRUE(p2.isOk());
	EXPECT_TRUE(!p2.unwrap().empty());
}


TEST(TestPath, testLength) {
	EXPECT_EQ(0, Path{}.length());
	EXPECT_EQ(4, makePathSafe("file").length());

    // Special case of a root path
	EXPECT_EQ(1, makePathSafe("").length());
	EXPECT_EQ(5, makePathSafe("").length("[]lll"));

    EXPECT_EQ(std::strlen("some/path/to/file"),
							makePathSafe("some", "path", "to", "file").length());

    EXPECT_EQ(std::strlen("/file"),
							makePathSafe("", "file").length());

    EXPECT_EQ(std::strlen("/some/path/to/file"),
							makePathSafe("", "some", "path", "to", "file").length());

    EXPECT_EQ(std::strlen("[-:]some[-:]path[-:]to[-:]file"),
							makePathSafe("", "some", "path", "to", "file").length("[-:]"));
}

/**
	* Test implementation and contract of comparable
    */
TEST(TestPath, testComparable) {
	auto maybeP1 = makePath("1", "2", "3", "4", "file");
	ASSERT_TRUE(maybeP1.isOk());

	auto maybeP2 = makePath("1", "2", "3", "4", "file");
	ASSERT_TRUE(maybeP2.isOk());

	auto maybeP3 = makePath("something", "2", "3", "file");
	ASSERT_TRUE(maybeP3.isOk());

	auto const& p1 = maybeP1.unwrap();
	auto const& p2 = maybeP2.unwrap();
	auto const& p_different = maybeP3.unwrap();

    EXPECT_TRUE(p1.equals(p2));
    EXPECT_TRUE(p2.equals(p1));
    EXPECT_EQ(p1, p2);
    EXPECT_EQ(0, p1.compareTo(p1));
    EXPECT_EQ(0, p2.compareTo(p2));
    EXPECT_EQ(0, p1.compareTo(p2));
    EXPECT_EQ(0, p2.compareTo(p1));

    EXPECT_EQ(false, p1.equals(p_different));
    EXPECT_EQ(false, p2.equals(p_different));
    EXPECT_EQ(false, p_different.equals(p1));
    EXPECT_EQ(false, p_different.equals(p2));
}


TEST(TestPath, compareTo) {
    // Verify compareTo lexicographical order
	EXPECT_LT(makePathSafe("aa").compareTo(makePathSafe("ab")), 0);
	EXPECT_LT(makePathSafe("a", "a").compareTo(makePathSafe("a", "b")), 0);
	EXPECT_LT(makePathSafe("a", "a").compareTo(makePathSafe("a", "c")), 0);
	EXPECT_LT(makePathSafe("a", "b", "c").compareTo(makePathSafe("a", "b", "c", "d")), 0);
	EXPECT_LT(makePathSafe("a", "b", "c").compareTo(makePathSafe("a", "b", "c", "d", "e")), 0);
	EXPECT_LT(makePathSafe("a", "a", "c").compareTo(makePathSafe("a", "c", "c", "d", "e")), 0);

	EXPECT_GT(makePathSafe("ab").compareTo(makePathSafe("aa")), 0);
	EXPECT_GT(makePathSafe("a", "b").compareTo(makePathSafe("a", "a")), 0);
	EXPECT_GT(makePathSafe("a", "c").compareTo(makePathSafe("a", "a")), 0);
	EXPECT_GT(makePathSafe("a", "b", "c", "d").compareTo(makePathSafe("a", "b", "c")), 0);
	EXPECT_GT(makePathSafe("a", "b", "c", "d", "e").compareTo(makePathSafe("a", "b", "c")), 0);
	EXPECT_GT(makePathSafe("a", "c", "c", "d", "e").compareTo(makePathSafe("a", "a", "c")), 0);
}


TEST(TestPath, testStartsWith) {
    {
		auto maybeP = makePath("some", "path", "to", "a", "file");
		ASSERT_TRUE(maybeP.isOk());
		auto const& p = maybeP.unwrap();

        EXPECT_TRUE(p.startsWith(p));
        EXPECT_TRUE(p.startsWith("som"));
        EXPECT_TRUE(p.startsWith("some"));

		EXPECT_EQ(true, p.startsWith(makePathSafe("some", "pa")));
		EXPECT_EQ(true, p.startsWith(makePathSafe("some", "path")));
		EXPECT_EQ(true, p.startsWith(makePathSafe("some", "path", "t")));

		EXPECT_EQ(false, p.startsWith(makePathSafe("so", "pa")));
		EXPECT_EQ(false, p.startsWith(makePathSafe("some", "pa", "to")));
    }

    {
		auto maybeP1 = makePath("1", "2", "3", "4", "file");
		auto maybeP2 = makePath("1", "2", "3");
		auto maybeP3 = makePath("2", "3", "4");
		auto const& p1 = maybeP1.unwrap();
		auto const& p2 = maybeP2.unwrap();
		auto const& p3 = maybeP3.unwrap();

        EXPECT_TRUE(p1.startsWith(p1));
		EXPECT_TRUE(p1.startsWith(makePathSafe("1")));
        EXPECT_TRUE(p1.startsWith(p2));

        EXPECT_EQ(false, p1.startsWith(p3));
        EXPECT_EQ(false, p2.startsWith(p3));
        EXPECT_EQ(false, p3.startsWith(p1));
		EXPECT_EQ(false, p3.startsWith(makePathSafe("2", "3", "4", "")));
    }
}


TEST(TestPath, testEndsWith) {
    {
		auto maybeP = makePath("some", "path", "to", "awesome", "file.awe");
		auto const& p = maybeP.unwrap();

        EXPECT_TRUE(p.endsWith(p));
        EXPECT_TRUE(p.endsWith("awe"));
        EXPECT_TRUE(p.endsWith("file.awe"));

		EXPECT_EQ(true, p.endsWith(makePathSafe("some", "file.awe")));
		EXPECT_EQ(true, p.endsWith(makePathSafe("awesome", "file.awe")));
		EXPECT_EQ(true, p.endsWith(makePathSafe("to", "awesome", "file.awe")));

		EXPECT_EQ(false, p.endsWith(makePathSafe("to", "awe", "file.awe")));
		EXPECT_EQ(false, p.endsWith(makePathSafe("to", "some", "file.awe")));
    }

    {
		auto maybeP1 = makePath("1", "2", "3", "4", "file");
		auto maybeP2 = makePath("3", "4", "file");
		auto maybeP3 = makePath("2", "3", "4");
		auto const& p1 = maybeP1.unwrap();
		auto const& p2 = maybeP2.unwrap();
		auto const& p3 = maybeP3.unwrap();

        EXPECT_TRUE(p1.endsWith(p1));
		EXPECT_TRUE(p1.endsWith(makePathSafe("file")));
        EXPECT_TRUE(p1.endsWith(p2));

        EXPECT_EQ(false, p1.endsWith(p3));
        EXPECT_EQ(false, p2.endsWith(p3));
        EXPECT_EQ(false, p3.endsWith(p1));
		EXPECT_EQ(false, p3.endsWith(makePathSafe("", "1", "2", "3", "4")));
    }
}


TEST(TestPath, testContains) {
	auto maybeP1 = makePath("1", "2", "3", "4", "file");
	auto maybeP2 = makePath("2", "3", "4");
	auto maybeP3 = makePath("4", "3", "file");
	auto const& p1 = maybeP1.unwrap();
	auto const& p2 = maybeP2.unwrap();
	auto const& p3 = maybeP3.unwrap();

    EXPECT_TRUE(p1.contains(p1));
    EXPECT_TRUE(p1.contains("file"));
    EXPECT_TRUE(p1.contains("2"));
    EXPECT_TRUE(p1.contains(p2));

    EXPECT_EQ(false, p1.contains(p3));
    EXPECT_EQ(false, p2.contains(p3));

    // Shorter path can not contain a longer one!
    EXPECT_EQ(false, p2.contains(p1));

	EXPECT_EQ(false, p1.contains(makePathSafe("1", "2", "5")));
}


TEST(TestPath, testGetParent) {
    {
		EXPECT_EQ(makePathSafe("1", "2", "3", "4"),
				  makePathSafe("1", "2", "3", "4", "file").getParent());

		auto maybeP = makePath("file");
		ASSERT_TRUE(maybeP.isOk());
		EXPECT_EQ(*maybeP, maybeP.unwrap().getParent());

		auto const maybeRoot = makePath("");
		ASSERT_TRUE(maybeRoot.isOk());
		EXPECT_EQ(*maybeRoot, maybeRoot.unwrap().getParent());
		EXPECT_EQ(*maybeRoot, makePathSafe("", "file").getParent());
    }
    {
        Path root{};
        EXPECT_TRUE(root.empty());

        Path const p = root.getParent();
        EXPECT_TRUE(p.empty());

		auto somePath = makePathSafe("abc");
		root = mv(somePath);
        EXPECT_TRUE(somePath.empty());
        EXPECT_TRUE(!root.empty());
    }
}


TEST(TestPath, testBasename) {
    EXPECT_EQ(StringView(), Path{}.getBasename());
	EXPECT_EQ(Path::Delimiter, makePathSafe("").getBasename());
	EXPECT_EQ(StringView("file"), makePathSafe("file").getBasename());
	EXPECT_EQ(StringView("file"), makePathSafe("file").getBasename());
	EXPECT_EQ(StringView("file"), makePathSafe("", "file").getBasename());

	EXPECT_EQ(StringView("."), makePathSafe(".").getBasename());
	EXPECT_EQ(StringView(".."), makePathSafe("..").getBasename());
	EXPECT_EQ(StringView("."), makePathSafe("", ".").getBasename());
	EXPECT_EQ(StringView(".."), makePathSafe("", "..").getBasename());

	EXPECT_EQ(StringView("etc"), makePathSafe("", "etc").getBasename());
	EXPECT_EQ(StringView(""), makePathSafe("", "etc", "").getBasename());
	EXPECT_EQ(StringView("file"), makePathSafe("", "etc", "file").getBasename());
	EXPECT_EQ(StringView(".."), makePathSafe("", "etc", "..").getBasename());
	EXPECT_EQ(StringView("."), makePathSafe("etc", "..", ".").getBasename());

	EXPECT_EQ(StringView("file"), makePathSafe("1", "2", "3", "4", "file").getBasename());
}


TEST(TestPath, testUnixBasename) {
    EXPECT_EQ(StringView("lib"),
                            Path::parse("/usr/lib").unwrap().getBasename());

    // FIXME(abbyssoul): This is directly from basename spec which we don't comply with atm :'(
    EXPECT_EQ(StringView("usr"),
                            Path::parse("/usr/").unwrap().getBasename());

    EXPECT_EQ(StringView("/"),
                            Path::parse("/").unwrap().getBasename());
}


TEST(TestPath, testComponents) {
    const StringLiteral components[] = {"1", "2", "3", "4", "file"};
	auto maybeP = makePath(components[0], components[1], components[2], components[3], components[4]);
	ASSERT_TRUE(maybeP.isOk());
	auto const& p = maybeP.unwrap();

    EXPECT_EQ(5, p.getComponentsCount());
    for (Path::size_type i = 0; i < p.getComponentsCount(); ++i) {
        EXPECT_EQ(components[i], p.getComponent(i));
    }
}


TEST(TestPath, testSubpath) {
	EXPECT_EQ(makePathSafe("1", "2", "3"), makePathSafe("1", "2", "3", "4", "file").subpath(0, 3));
	EXPECT_EQ(makePathSafe("3", "4", "file"), makePathSafe("1", "2", "3", "4", "file").subpath(2, 5));
	EXPECT_EQ(makePathSafe("2", "3"), makePathSafe("1", "2", "3", "4", "file").subpath(1, 3));

    // Error modes:

    // End index outside of path lenght
	EXPECT_EQ(4, makePathSafe("1", "2", "3", "4", "file").subpath(1, 23).getComponentsCount());
    // Start index oustide of path lenght
	EXPECT_TRUE(makePathSafe("1", "2", "3", "4", "file").subpath(17, 18).empty());
    // Start greater then end index
	EXPECT_TRUE(makePathSafe("1", "2", "3", "4", "file").subpath(3, 1).empty());
}


TEST(TestPath, testJoin) {
	EXPECT_EQ(makePath("etc", "file"),
			  makePath(makePathSafe("etc"), makePathSafe("file")));
	EXPECT_EQ(makePath("etc", "file"),
			  makePath(makePathSafe("etc"), StringView("file")));
	EXPECT_EQ(makePath("etc", "file"),
			  makePath(makePathSafe("etc"), "file"));

	EXPECT_EQ(makePath("etc", "some", "long", "path"),
			  makePath(makePathSafe("etc"), makePathSafe("some"), makePathSafe("long"), makePathSafe("path")));
	EXPECT_EQ(makePath("etc", "some", "long", "path"),
			  makePath(StringView("etc"), StringView("some"), StringView("long"), StringView("path")));
	EXPECT_EQ(makePath("etc", "some", "long", "path"),
			  Path::parse("etc/some/long/path"));
}


TEST(TestPath, testIterable) {
	auto maybeP = makePath("e", "so", "lon", "path", "foilx");
	ASSERT_TRUE(maybeP.isOk());
	auto const& p = maybeP.unwrap();

    String::size_type i = 0;
    for (auto& v : p) {
        ++i;
        EXPECT_EQ(i, v.length());
    }
}

TEST(TestPath, testForEach) {
    std::vector<int> counts;
	makePathSafe("e", "so", "long", "pat", "fx", "x").forEach([&counts] (const String& component){
        counts.push_back(component.length());
    });

    EXPECT_EQ(std::vector<int>({1, 2, 4, 3, 2, 1}), counts);
}


TEST(TestPath, testIsAbsolute) {
	EXPECT_FALSE(makePathSafe("etc").isAbsolute());
	EXPECT_FALSE(makePathSafe("etc", "2", "file").isAbsolute());
	EXPECT_TRUE(makePathSafe("", "etc", "dir", "file").isAbsolute());
	EXPECT_TRUE(makePathSafe("", "2", "f", "").isAbsolute());
}


TEST(TestPath, testIsRelative) {
	EXPECT_TRUE(makePathSafe("etc").isRelative());
	EXPECT_TRUE(makePathSafe("1", "2", "f").isRelative());
	EXPECT_TRUE(makePathSafe("1", "2", "f", "").isRelative());
	EXPECT_FALSE(makePathSafe("", "1", "2", "f").isRelative());
}


TEST(TestPath, testNormalize) {

    EXPECT_EQ(Path::Root,
			  makePathSafe("").normalize());

	EXPECT_EQ(makePathSafe("file"),
			  makePathSafe("file").normalize());

	EXPECT_EQ(makePathSafe(".."),
			  makePathSafe("..").normalize());

	EXPECT_EQ(makePathSafe("1", "2", "f"),
			  makePathSafe("1", ".", "2", "f").normalize());

	EXPECT_EQ(makePathSafe("1", "f"),
			  makePathSafe("1", "2", "..", "f").normalize());

	EXPECT_EQ(makePathSafe("1", "3"),
			  makePathSafe(".", "1", "2", "..", "3", ".", "f", "..").normalize());

	EXPECT_EQ(makePathSafe("..", "2", "fixt"),
			  makePathSafe("..", "2", "fixt").normalize());
}

/**
    * Test implementation and contract of toString
    */
TEST(TestPath, testToString) {
	EXPECT_EQ(StringLiteral("/"), makePathSafe("").toString());
	EXPECT_EQ(StringLiteral("[:]"), makePathSafe("").toString("[:]"));
	EXPECT_EQ(StringLiteral("filename"), makePathSafe("filename").toString());
	EXPECT_EQ(StringLiteral("filename"), makePathSafe("filename").toString("[:]"));

    {
		auto maybeP = makePath("3", "2", "1");
		ASSERT_TRUE(maybeP.isOk());
		auto const& p = maybeP.unwrap();

        EXPECT_EQ(StringLiteral("3/2/1"),   p.toString());
        EXPECT_EQ(StringLiteral("3|:2|:1"), p.toString("|:"));
    }

    {
		auto maybeP = makePath("", "etc", "something", "1");
		ASSERT_TRUE(maybeP.isOk());
		auto const& p = maybeP.unwrap();

		EXPECT_EQ(StringLiteral("/etc/something/1"), p.toString());
        EXPECT_EQ(StringLiteral("|:etc|:something|:1"), p.toString("|:"));
    }
}


/**
    * Test implementation and contract of parsable
    */
TEST(TestPath, testParsing) {
    {
        EXPECT_EQ(makePath("some-long_path"),
				  Path::parse("some-long_path"));
    }
    {
        EXPECT_EQ(makePath("", "etc"),
				  Path::parse("/etc"));
    }
    {
        EXPECT_EQ(Path::Root, Path::parse("").unwrap());
        EXPECT_EQ(Path::Root, Path::parse("/").unwrap());
        EXPECT_EQ(Path::Root, Path::parse(Path::Delimiter).unwrap());
    }
    {
        EXPECT_EQ(makePath("some", "file", "path.321"),
				  Path::parse("some/file/path.321"));
    }
    {
        EXPECT_EQ(makePath("some", "file", "path.321"),
				  Path::parse("some/file/path.321/"));
    }
    {
        EXPECT_EQ(makePath("some", "file", "", "path.321"),
				  Path::parse("some/file//path.321/"));
    }
    {
        EXPECT_EQ(makePath("", "!)", "$@#&@#", "some", "file", "path"),
				  Path::parse("/!)/$@#&@#/some/file/path"));
    }
    {
        EXPECT_EQ(makePath("some", "file", "path"),
				  Path::parse("some.file.path", "."));
    }
    {
        EXPECT_EQ(makePath("some", "file", "", "path"),
				  Path::parse("some.file..path", "."));
    }
    {
        EXPECT_EQ(makePath("", "some", "file", "path"),
				  Path::parse("{?some{?file{?path{?", "{?"));
    }
    {
        EXPECT_EQ(makePath("", "some", "", "file", "path"),
				  Path::parse("{?some{?{?file{?path{?", "{?"));
    }
}

/**
    * Test consistency of parsing and toString implementation
    */
TEST(TestPath, testParsing_and_ToString_are_consistent) {
    {
        auto const src = StringLiteral("some-long_path");
        auto const v = Path::parse(src).unwrap();
        EXPECT_EQ(src, v.toString());
    }
    {
        auto const src = StringLiteral("some/file/path.321");
        auto const v = Path::parse(src).unwrap();
        EXPECT_EQ(src, v.toString());
    }
    {
        EXPECT_EQ(StringLiteral("some/file/path.321"),
                                Path::parse("some/file/path.321/")
                                .unwrap()
                                .toString());
    }
    {
        auto const src = StringLiteral("/!)/$@#&@#/some/file/path");
        auto const v = Path::parse(src).unwrap();
        EXPECT_EQ(src, v.toString());
    }
    {
        auto const src = StringLiteral("some.file.path");
        auto const v = Path::parse(src, "\\.").unwrap();
        EXPECT_EQ(src, v.toString("."));
    }
    {
        auto const src = StringLiteral("some.file..path");
        auto const v = Path::parse(src, "\\.").unwrap();
        EXPECT_EQ(src, v.toString("."));
    }
    {
        EXPECT_EQ(StringLiteral("{?some{?file{?path"),
                                Path::parse("{?some{?file{?path{?", "{?")
                                .unwrap()
                                .toString("{?"));
    }
}
