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
 * @author: soultaker
 *
 * Created on: 21 Jan 2016
*******************************************************************************/
#include <solace/path.hpp>			// Class being tested
#include <solace/exception.hpp>     // Checked expcetions


#include <ostream>


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


#include "mockTypes.hpp"
#include <gtest/gtest.h>
#include <cstring>

using namespace Solace;


TEST(TestPath, testRootIsSingleComponent) {
    EXPECT_EQ(1, Path::Root.getComponentsCount());
}

TEST(TestPath, testRootAbsolute) {
    EXPECT_TRUE(Path::Root.isAbsolute());
}

TEST(TestPath, testEmpty) {
    EXPECT_TRUE(Path().empty());
    EXPECT_EQ(0, Path().length());
    EXPECT_EQ(0, Path().getComponentsCount());

    EXPECT_TRUE(!makePath("file").empty());
    EXPECT_TRUE(!makePath("some", "path", "to", "file").empty());
}


TEST(TestPath, testLength) {
    EXPECT_EQ(static_cast<String::size_type>(0), Path().length());
    EXPECT_EQ(static_cast<String::size_type>(4), makePath("file").length());

    // Special case of a root path
    EXPECT_EQ(static_cast<String::size_type>(1), makePath("").length());
    EXPECT_EQ(static_cast<String::size_type>(5), makePath("").length("[]lll"));

    EXPECT_EQ(static_cast<String::size_type>(std::strlen("some/path/to/file")),
                            makePath("some", "path", "to", "file").length());

    EXPECT_EQ(static_cast<String::size_type>(std::strlen("/file")),
                            makePath("", "file").length());

    EXPECT_EQ(static_cast<String::size_type>(std::strlen("/some/path/to/file")),
                            makePath("", "some", "path", "to", "file").length());

    EXPECT_EQ(static_cast<String::size_type>(std::strlen("[-:]some[-:]path[-:]to[-:]file")),
                            makePath("", "some", "path", "to", "file").length("[-:]"));
}

/**
    * Test implementation and contract of IComparable
    */
TEST(TestPath, testComparable) {
    auto const p1 = makePath("1", "2", "3", "4", "file");
    auto const p2 = makePath("1", "2", "3", "4", "file");
    auto const p_different = makePath("something", "2", "3", "file");

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

    // Verify compareTo lexicographical order
    EXPECT_LT(makePath("aa").compareTo(makePath("ab")), 0);
    EXPECT_LT(makePath("a", "a").compareTo(makePath("a", "b")), 0);
    EXPECT_LT(makePath("a", "a").compareTo(makePath("a", "c")), 0);
    EXPECT_LT(makePath("a", "b", "c").compareTo(makePath("a", "b", "c", "d")), 0);
    EXPECT_LT(makePath("a", "b", "c").compareTo(makePath("a", "b", "c", "d", "e")), 0);
    EXPECT_LT(makePath("a", "a", "c").compareTo(makePath("a", "c", "c", "d", "e")), 0);

    EXPECT_GT(makePath("ab").compareTo(makePath("aa")), 0);
    EXPECT_GT(makePath("a", "b").compareTo(makePath("a", "a")), 0);
    EXPECT_GT(makePath("a", "c").compareTo(makePath("a", "a")), 0);
    EXPECT_GT(makePath("a", "b", "c", "d").compareTo(makePath("a", "b", "c")), 0);
    EXPECT_GT(makePath("a", "b", "c", "d", "e").compareTo(makePath("a", "b", "c")), 0);
    EXPECT_GT(makePath("a", "c", "c", "d", "e").compareTo(makePath("a", "a", "c")), 0);
}


TEST(TestPath, testStartsWith) {
    {
        auto const p = makePath("some", "path", "to", "a", "file");
        EXPECT_TRUE(p.startsWith(p));
        EXPECT_TRUE(p.startsWith("som"));
        EXPECT_TRUE(p.startsWith("some"));

        EXPECT_EQ(true, p.startsWith(makePath("some", "pa")));
        EXPECT_EQ(true, p.startsWith(makePath("some", "path")));
        EXPECT_EQ(true, p.startsWith(makePath("some", "path", "t")));

        EXPECT_EQ(false, p.startsWith(makePath("so", "pa")));
        EXPECT_EQ(false, p.startsWith(makePath("some", "pa", "to")));
    }

    {
        auto const p1 = makePath("1", "2", "3", "4", "file");
        auto const p2 = makePath("1", "2", "3");
        auto const p3 = makePath("2", "3", "4");

        EXPECT_TRUE(p1.startsWith(p1));
        EXPECT_TRUE(p1.startsWith(makePath("1")));
        EXPECT_TRUE(p1.startsWith(p2));

        EXPECT_EQ(false, p1.startsWith(p3));
        EXPECT_EQ(false, p2.startsWith(p3));
        EXPECT_EQ(false, p3.startsWith(p1));
        EXPECT_EQ(false, p3.startsWith(makePath("2", "3", "4", "")));
    }
}


TEST(TestPath, testEndsWith) {
    {
        auto const p = makePath("some", "path", "to", "awesome", "file.awe");
        EXPECT_TRUE(p.endsWith(p));
        EXPECT_TRUE(p.endsWith("awe"));
        EXPECT_TRUE(p.endsWith("file.awe"));

        EXPECT_EQ(true, p.endsWith(makePath("some", "file.awe")));
        EXPECT_EQ(true, p.endsWith(makePath("awesome", "file.awe")));
        EXPECT_EQ(true, p.endsWith(makePath("to", "awesome", "file.awe")));

        EXPECT_EQ(false, p.endsWith(makePath("to", "awe", "file.awe")));
        EXPECT_EQ(false, p.endsWith(makePath("to", "some", "file.awe")));
    }

    {
        auto const p1 = makePath("1", "2", "3", "4", "file");
        auto const p2 = makePath("3", "4", "file");
        auto const p3 = makePath("2", "3", "4");

        EXPECT_TRUE(p1.endsWith(p1));
        EXPECT_TRUE(p1.endsWith(makePath("file")));
        EXPECT_TRUE(p1.endsWith(p2));

        EXPECT_EQ(false, p1.endsWith(p3));
        EXPECT_EQ(false, p2.endsWith(p3));
        EXPECT_EQ(false, p3.endsWith(p1));
        EXPECT_EQ(false, p3.endsWith(makePath("", "1", "2", "3", "4")));
    }
}


TEST(TestPath, testContains) {
    auto const p1 = makePath("1", "2", "3", "4", "file");
    auto const p2 = makePath("2", "3", "4");
    auto const p3 = makePath("4", "3", "file");

    EXPECT_TRUE(p1.contains(p1));
    EXPECT_TRUE(p1.contains("file"));
    EXPECT_TRUE(p1.contains("2"));
    EXPECT_TRUE(p1.contains(p2));

    EXPECT_EQ(false, p1.contains(p3));
    EXPECT_EQ(false, p2.contains(p3));

    // Shorter path can not contain a longer one!
    EXPECT_EQ(false, p2.contains(p1));

    EXPECT_EQ(false, p1.contains(makePath("1", "2", "5")));
}

TEST(TestPath, testGetParent) {
    {
        EXPECT_EQ(makePath("1", "2", "3", "4"),
                  makePath("1", "2", "3", "4", "file").getParent());

        auto const p3 = makePath("file");
        EXPECT_EQ(p3, p3.getParent());

        auto const root = makePath("");
        EXPECT_EQ(root, root.getParent());
        EXPECT_EQ(root, makePath("", "file").getParent());
    }
    {
        Path root{};
        EXPECT_TRUE(root.empty());

        Path const p = root.getParent();
        EXPECT_TRUE(p.empty());

        auto somePath = makePath("abc");
        root = std::move(somePath);
        EXPECT_TRUE(somePath.empty());
        EXPECT_TRUE(!root.empty());
    }
}


TEST(TestPath, testBasename) {
    EXPECT_EQ(StringView(), Path{}.getBasename());
    EXPECT_EQ(Path::Delimiter, makePath("").getBasename());
    EXPECT_EQ(StringView("file"), makePath("file").getBasename());
    EXPECT_EQ(StringView("file"), makePath("file").getBasename());
    EXPECT_EQ(StringView("file"), makePath("", "file").getBasename());

    EXPECT_EQ(StringView("."), makePath(".").getBasename());
    EXPECT_EQ(StringView(".."), makePath("..").getBasename());
    EXPECT_EQ(StringView("."), makePath("", ".").getBasename());
    EXPECT_EQ(StringView(".."), makePath("", "..").getBasename());

    EXPECT_EQ(StringView("etc"), makePath("", "etc").getBasename());
    EXPECT_EQ(StringView(""), makePath("", "etc", "").getBasename());
    EXPECT_EQ(StringView("file"), makePath("", "etc", "file").getBasename());
    EXPECT_EQ(StringView(".."), makePath("", "etc", "..").getBasename());
    EXPECT_EQ(StringView("."), makePath("etc", "..", ".").getBasename());

    EXPECT_EQ(StringView("file"), makePath("1", "2", "3", "4", "file").getBasename());
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
    auto const p = makePath(components[0], components[1], components[2], components[3], components[4]);

    EXPECT_EQ(5, p.getComponentsCount());
    for (Path::size_type i = 0; i < p.getComponentsCount(); ++i) {
        EXPECT_EQ(components[i], p.getComponent(i));
    }
}


TEST(TestPath, testSubpath) {
    EXPECT_EQ(makePath("1", "2", "3"), makePath("1", "2", "3", "4", "file").subpath(0, 3));
    EXPECT_EQ(makePath("3", "4", "file"), makePath("1", "2", "3", "4", "file").subpath(2, 5));
    EXPECT_EQ(makePath("2", "3"), makePath("1", "2", "3", "4", "file").subpath(1, 3));

    // Error modes:

    // End index outside of path lenght
    EXPECT_EQ(4, makePath("1", "2", "3", "4", "file").subpath(1, 23).getComponentsCount());
    // Start index oustide of path lenght
    EXPECT_TRUE(makePath("1", "2", "3", "4", "file").subpath(17, 18).empty());
    // Start greater then end index
    EXPECT_TRUE(makePath("1", "2", "3", "4", "file").subpath(3, 1).empty());
}


TEST(TestPath, testJoin) {
    EXPECT_EQ(makePath("etc", "file"),
              makePath(makePath("etc"), makePath("file")));
    EXPECT_EQ(makePath("etc", "file"),
              makePath(makePath("etc"), StringView("file")));
    EXPECT_EQ(makePath("etc", "file"),
              makePath(makePath("etc"), "file"));

    EXPECT_EQ(makePath("etc", "some", "long", "path"),
              makePath(makePath("etc"), makePath("some"), makePath("long"), makePath("path")));
    EXPECT_EQ(makePath("etc", "some", "long", "path"),
              makePath(StringView("etc"), StringView("some"), StringView("long"), StringView("path")));
    EXPECT_EQ(makePath("etc", "some", "long", "path"),
              Path::parse("etc/some/long/path").unwrap());
}

TEST(TestPath, testIterable) {
    auto const p = makePath("e", "so", "lon", "path", "foilx");

    String::size_type i = 0;
    for (auto& v : p) {
        ++i;
        EXPECT_EQ(i, v.length());
    }
}

TEST(TestPath, testForEach) {
    std::vector<int> counts;
    makePath("e", "so", "long", "pat", "fx", "x").forEach([&counts] (const String& component){
        counts.push_back(component.length());
    });

    EXPECT_EQ(std::vector<int>({1, 2, 4, 3, 2, 1}), counts);
}


TEST(TestPath, testIsAbsolute) {
    EXPECT_FALSE(makePath("etc").isAbsolute());
    EXPECT_FALSE(makePath("etc", "2", "file").isAbsolute());
    EXPECT_TRUE(makePath("", "etc", "dir", "file").isAbsolute());
    EXPECT_TRUE(makePath("", "2", "f", "").isAbsolute());
}


TEST(TestPath, testIsRelative) {
    EXPECT_TRUE(makePath("etc").isRelative());
    EXPECT_TRUE(makePath("1", "2", "f").isRelative());
    EXPECT_TRUE(makePath("1", "2", "f", "").isRelative());
    EXPECT_FALSE(makePath("", "1", "2", "f").isRelative());
}


TEST(TestPath, testNormalize) {

    EXPECT_EQ(Path::Root,
              makePath("").normalize());

    EXPECT_EQ(makePath("file"),
              makePath("file").normalize());

    EXPECT_EQ(makePath(".."),
              makePath("..").normalize());

    EXPECT_EQ(makePath("1", "2", "f"),
              makePath("1", ".", "2", "f").normalize());

    EXPECT_EQ(makePath("1", "f"),
              makePath("1", "2", "..", "f").normalize());

    EXPECT_EQ(makePath("1", "3"),
              makePath(".", "1", "2", "..", "3", ".", "f", "..").normalize());

    EXPECT_EQ(makePath("..", "2", "fixt"),
              makePath("..", "2", "fixt").normalize());
}

/**
    * Test implementation and contract of IFormattable
    */
TEST(TestPath, testToString) {
    EXPECT_EQ(StringLiteral("/"), makePath("").toString());
    EXPECT_EQ(StringLiteral("[:]"), makePath("").toString("[:]"));
    EXPECT_EQ(StringLiteral("filename"), makePath("filename").toString());
    EXPECT_EQ(StringLiteral("filename"), makePath("filename").toString("[:]"));

    {
        auto const p = makePath("3", "2", "1");
        EXPECT_EQ(StringLiteral("3/2/1"),   p.toString());
        EXPECT_EQ(StringLiteral("3|:2|:1"), p.toString("|:"));
    }

    {
        auto const p = makePath("", "etc", "something", "1");
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
                  Path::parse("some-long_path").unwrap());
    }
    {
        EXPECT_EQ(makePath("", "etc"),
                  Path::parse("/etc").unwrap());
    }
    {
        EXPECT_EQ(Path::Root, Path::parse("").unwrap());
        EXPECT_EQ(Path::Root, Path::parse("/").unwrap());
        EXPECT_EQ(Path::Root, Path::parse(Path::Delimiter).unwrap());
    }
    {
        EXPECT_EQ(makePath("some", "file", "path.321"),
                  Path::parse("some/file/path.321").unwrap());
    }
    {
        EXPECT_EQ(makePath("some", "file", "path.321"),
                  Path::parse("some/file/path.321/").unwrap());
    }
    {
        EXPECT_EQ(makePath("some", "file", "", "path.321"),
                  Path::parse("some/file//path.321/").unwrap());
    }
    {
        EXPECT_EQ(makePath("", "!)", "$@#&@#", "some", "file", "path"),
                  Path::parse("/!)/$@#&@#/some/file/path").unwrap());
    }
    {
        EXPECT_EQ(makePath("some", "file", "path"),
                  Path::parse("some.file.path", ".").unwrap());
    }
    {
        EXPECT_EQ(makePath("some", "file", "", "path"),
                  Path::parse("some.file..path", ".").unwrap());
    }
    {
        EXPECT_EQ(makePath("", "some", "file", "path"),
                  Path::parse("{?some{?file{?path{?", "{?").unwrap());
    }
    {
        EXPECT_EQ(makePath("", "some", "", "file", "path"),
                  Path::parse("{?some{?{?file{?path{?", "{?").unwrap());
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
