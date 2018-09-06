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
    EXPECT_EQ(static_cast<Path::size_type>(1), Path::Root.getComponentsCount());
}

TEST(TestPath, testRootAbsolute) {
    EXPECT_TRUE(Path::Root.isAbsolute());
}

TEST(TestPath, testEmpty) {
    EXPECT_TRUE(Path().empty());
    EXPECT_TRUE(!allocPath("file").empty());
    EXPECT_TRUE(!allocPath({"some", "path", "to", "file"}).empty());
}


TEST(TestPath, testLength) {
    EXPECT_EQ(static_cast<String::size_type>(0), Path().length());
    EXPECT_EQ(static_cast<String::size_type>(4), allocPath("file").length());

    // Special case of a root path
    EXPECT_EQ(static_cast<String::size_type>(1), allocPath({""}).length());
    EXPECT_EQ(static_cast<String::size_type>(5), allocPath({""}).length("[]lll"));

    EXPECT_EQ(static_cast<String::size_type>(std::strlen("some/path/to/file")),
                            allocPath({"some", "path", "to", "file"}).length());

    EXPECT_EQ(static_cast<String::size_type>(std::strlen("/file")),
                            allocPath({"", "file"}).length());

    EXPECT_EQ(static_cast<String::size_type>(std::strlen("/some/path/to/file")),
                            allocPath({"", "some", "path", "to", "file"}).length());

    EXPECT_EQ(static_cast<String::size_type>(std::strlen("[-:]some[-:]path[-:]to[-:]file")),
                            allocPath({"", "some", "path", "to", "file"}).length("[-:]"));
}

/**
    * Test implementation and contract of IComparable
    */
TEST(TestPath, testComparable) {
    auto const p1 = allocPath({"1", "2", "3", "4", "file"});
    auto const p2 = allocPath({"1", "2", "3", "4", "file"});
    auto const p_different = allocPath({"something", "2", "3", "file"});

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

    // Verify compareTo
    EXPECT_LT(allocPath("aa").compareTo(allocPath("ab")), 0);
    EXPECT_LT(allocPath({"a", "a"}).compareTo(allocPath({"a", "b"})), 0);
    EXPECT_LT(allocPath({"a", "a"}).compareTo(allocPath({"a", "c"})), 0);
    EXPECT_LT(allocPath({"a", "b", "c"}).compareTo(allocPath({"a", "b", "c", "d"})), 0);
    EXPECT_LT(allocPath({"a", "b", "c"}).compareTo(allocPath({"a", "b", "c", "d", "e"})), 0);
    EXPECT_LT(allocPath({"a", "a", "c"}).compareTo(allocPath({"a", "c", "c", "d", "e"})), 0);

    EXPECT_GT(allocPath("ab").compareTo(allocPath("aa")), 0);
    EXPECT_GT(allocPath({"a", "b"}).compareTo(allocPath({"a", "a"})), 0);
    EXPECT_GT(allocPath({"a", "c"}).compareTo(allocPath({"a", "a"})), 0);
    EXPECT_GT(allocPath({"a", "b", "c", "d"}).compareTo(allocPath({"a", "b", "c"})), 0);
    EXPECT_GT(allocPath({"a", "b", "c", "d", "e"}).compareTo(allocPath({"a", "b", "c"})), 0);
    EXPECT_GT(allocPath({"a", "c", "c", "d", "e"}).compareTo(allocPath({"a", "a", "c"})), 0);
}


TEST(TestPath, testStartsWith) {
    {
        auto const p = allocPath({"some", "path", "to", "a", "file"});
        EXPECT_TRUE(p.startsWith(p));
        EXPECT_TRUE(p.startsWith("som"));
        EXPECT_TRUE(p.startsWith("some"));

        EXPECT_EQ(true, p.startsWith(allocPath({"some", "pa"})));
        EXPECT_EQ(true, p.startsWith(allocPath({"some", "path"})));
        EXPECT_EQ(true, p.startsWith(allocPath({"some", "path", "t"})));

        EXPECT_EQ(false, p.startsWith(allocPath({"so", "pa"})));
        EXPECT_EQ(false, p.startsWith(allocPath({"some", "pa", "to"})));
    }

    {
        auto const p1 = allocPath({"1", "2", "3", "4", "file"});
        auto const p2 = allocPath({"1", "2", "3"});
        auto const p3 = allocPath({"2", "3", "4"});

        EXPECT_TRUE(p1.startsWith(p1));
        EXPECT_TRUE(p1.startsWith(allocPath("1")));
        EXPECT_TRUE(p1.startsWith(p2));

        EXPECT_EQ(false, p1.startsWith(p3));
        EXPECT_EQ(false, p2.startsWith(p3));
        EXPECT_EQ(false, p3.startsWith(p1));
        EXPECT_EQ(false, p3.startsWith(allocPath({"2", "3", "4", ""})));
    }
}


TEST(TestPath, testEndsWith) {
    {
        auto const p = allocPath({"some", "path", "to", "awesome", "file.awe"});
        EXPECT_TRUE(p.endsWith(p));
        EXPECT_TRUE(p.endsWith("awe"));
        EXPECT_TRUE(p.endsWith("file.awe"));

        EXPECT_EQ(true, p.endsWith(allocPath({"some", "file.awe"})));
        EXPECT_EQ(true, p.endsWith(allocPath({"awesome", "file.awe"})));
        EXPECT_EQ(true, p.endsWith(allocPath({"to", "awesome", "file.awe"})));

        EXPECT_EQ(false, p.endsWith(allocPath({"to", "awe", "file.awe"})));
        EXPECT_EQ(false, p.endsWith(allocPath({"to", "some", "file.awe"})));
    }

    {
        auto const p1 = allocPath({"1", "2", "3", "4", "file"});
        auto const p2 = allocPath({"3", "4", "file"});
        auto const p3 = allocPath({"2", "3", "4"});

        EXPECT_TRUE(p1.endsWith(p1));
        EXPECT_TRUE(p1.endsWith(allocPath("file")));
        EXPECT_TRUE(p1.endsWith(p2));

        EXPECT_EQ(false, p1.endsWith(p3));
        EXPECT_EQ(false, p2.endsWith(p3));
        EXPECT_EQ(false, p3.endsWith(p1));
        EXPECT_EQ(false, p3.endsWith(allocPath({"", "1", "2", "3", "4"})));
    }
}


TEST(TestPath, testContains) {
    auto const p1 = allocPath({"1", "2", "3", "4", "file"});
    auto const p2 = allocPath({"2", "3", "4"});
    auto const p3 = allocPath({"4", "3", "file"});

    EXPECT_TRUE(p1.contains(p1));
    EXPECT_TRUE(p1.contains("file"));
    EXPECT_TRUE(p1.contains("2"));
    EXPECT_TRUE(p1.contains(p2));

    EXPECT_EQ(false, p1.contains(p3));
    EXPECT_EQ(false, p2.contains(p3));

    // Shorter path can not contain a longer one!
    EXPECT_EQ(false, p2.contains(p1));

    EXPECT_EQ(false, p1.contains(allocPath({"1", "2", "5"})));
}

TEST(TestPath, testGetParent) {
    {
        EXPECT_EQ(allocPath({"1", "2", "3", "4"}),
                  allocPath({"1", "2", "3", "4", "file"}).getParent());

        auto const p3 = allocPath("file");
        EXPECT_EQ(p3, p3.getParent());

        auto const root = allocPath("");
        EXPECT_EQ(root, root.getParent());
        EXPECT_EQ(root, allocPath({"", "file"}).getParent());
    }
    {
        Path root{};
        EXPECT_TRUE(root.empty());

        Path const p = root.getParent();
        EXPECT_TRUE(p.empty());

        auto somePath = allocPath("abc");
        root = std::move(somePath);
        EXPECT_TRUE(somePath.empty());
        EXPECT_TRUE(!root.empty());
    }
}


TEST(TestPath, testBasename) {
    EXPECT_EQ(StringView(), Path{}.getBasename());
    EXPECT_EQ(Path::Delimiter, allocPath("").getBasename());
    EXPECT_EQ(StringView("file"), allocPath("file").getBasename());
    EXPECT_EQ(StringView("file"), allocPath({"file"}).getBasename());
    EXPECT_EQ(StringView("file"), allocPath({"", "file"}).getBasename());

    EXPECT_EQ(StringView("."), allocPath(".").getBasename());
    EXPECT_EQ(StringView(".."), allocPath("..").getBasename());
    EXPECT_EQ(StringView("."), allocPath({"", "."}).getBasename());
    EXPECT_EQ(StringView(".."), allocPath({"", ".."}).getBasename());

    EXPECT_EQ(StringView("etc"), allocPath({"", "etc"}).getBasename());
    EXPECT_EQ(StringView(""), allocPath({"", "etc", ""}).getBasename());
    EXPECT_EQ(StringView("file"), allocPath({"", "etc", "file"}).getBasename());
    EXPECT_EQ(StringView(".."), allocPath({"", "etc", ".."}).getBasename());
    EXPECT_EQ(StringView("."), allocPath({"etc", "..", "."}).getBasename());

    EXPECT_EQ(StringView("file"), allocPath({"1", "2", "3", "4", "file"}).getBasename());
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
    const String components[] = {"1", "2", "3", "4", "file"};
    auto const p = allocPath({components[0], components[1], components[2], components[3], components[4]});

    EXPECT_EQ(static_cast<Path::size_type>(5), p.getComponentsCount());
    for (Path::size_type i = 0; i < p.getComponentsCount(); ++i) {
        EXPECT_EQ(components[i], p.getComponent(i));
    }
}


TEST(TestPath, testSubpath) {
    EXPECT_EQ(allocPath({"1", "2", "3"}), allocPath({"1", "2", "3", "4", "file"}).subpath(0, 3));
    EXPECT_EQ(allocPath({"3", "4", "file"}), allocPath({"1", "2", "3", "4", "file"}).subpath(2, 5));
    EXPECT_EQ(allocPath({"2", "3"}), allocPath({"1", "2", "3", "4", "file"}).subpath(1, 3));

    // Error modes:

    // End index outside of path lenght
    EXPECT_THROW(allocPath({"1", "2", "3", "4", "file"}).subpath(1, 23), IndexOutOfRangeException);
    // Start index oustide of path lenght
    EXPECT_THROW(allocPath({"1", "2", "3", "4", "file"}).subpath(17, 18), IndexOutOfRangeException);
    // Start greater then end index
    EXPECT_THROW(allocPath({"1", "2", "3", "4", "file"}).subpath(3, 1), IndexOutOfRangeException);
}


TEST(TestPath, testJoin) {
    EXPECT_EQ(allocPath({"etc", "file"}),
              allocPath(allocPath("etc"), allocPath("file")));
    EXPECT_EQ(allocPath({"etc", "file"}),
              allocPath(allocPath("etc"), StringView("file")));
    EXPECT_EQ(allocPath({"etc", "file"}),
              allocPath(allocPath("etc"), String("file")));
    EXPECT_EQ(allocPath({"etc", "file"}),
              allocPath(allocPath("etc"), "file"));

    EXPECT_EQ(allocPath({"etc", "some", "long", "path"}),
              allocPath({allocPath("etc"), allocPath("some"), allocPath("long"), allocPath("path")}));
    EXPECT_EQ(allocPath({"etc", "some", "long", "path"}),
              allocPath({StringView("etc"), StringView("some"), StringView("long"), StringView("path")}));
    EXPECT_EQ(allocPath({"etc", "some", "long", "path"}),
              allocPath({String("etc"), String("some"), String("long"), String("path")}));
}

TEST(TestPath, testIterable) {
    auto const p = allocPath({"e", "so", "lon", "path", "foilx"});

    String::size_type i = 0;
    for (auto& v : p) {
        ++i;
        EXPECT_EQ(i, v.length());
    }
}

TEST(TestPath, testForEach) {
    std::vector<int> counts;
    allocPath({"e", "so", "long", "pat", "fx", "x"}).forEach([&counts] (const String& component){
        counts.push_back(component.length());
    });

    EXPECT_EQ(std::vector<int>({1, 2, 4, 3, 2, 1}), counts);
}


TEST(TestPath, testIsAbsolute) {
    EXPECT_FALSE(allocPath("etc").isAbsolute());
    EXPECT_FALSE(allocPath({"etc", "2", "file"}).isAbsolute());
    EXPECT_TRUE(allocPath({"", "etc", "dir", "file"}).isAbsolute());
    EXPECT_TRUE(allocPath({"", "2", "f", ""}).isAbsolute());
}


TEST(TestPath, testIsRelative) {
    EXPECT_TRUE(allocPath("etc").isRelative());
    EXPECT_TRUE(allocPath({"1", "2", "f"}).isRelative());
    EXPECT_TRUE(allocPath({"1", "2", "f", ""}).isRelative());
    EXPECT_FALSE(allocPath({"", "1", "2", "f"}).isRelative());
}


TEST(TestPath, testNormalize) {

    EXPECT_EQ(Path::Root,
              allocPath("").normalize());

    EXPECT_EQ(allocPath("file"),
              allocPath("file").normalize());

    EXPECT_EQ(allocPath(".."),
              allocPath("..").normalize());

    EXPECT_EQ(allocPath({"1", "2", "f"}),
              allocPath({"1", ".", "2", "f"}).normalize());

    EXPECT_EQ(allocPath({"1", "f"}),
              allocPath({"1", "2", "..", "f"}).normalize());

    EXPECT_EQ(allocPath({"1", "3"}),
              allocPath({".", "1", "2", "..", "3", ".", "f", ".."}).normalize());

    EXPECT_EQ(allocPath({"..", "2", "fixt"}),
              allocPath({"..", "2", "fixt"}).normalize());
}

/**
    * Test implementation and contract of IFormattable
    */
TEST(TestPath, testToString) {
    EXPECT_EQ(StringLiteral("/"), allocPath("").toString());
    EXPECT_EQ(StringLiteral("[:]"), allocPath("").toString("[:]"));
    EXPECT_EQ(StringLiteral("filename"), allocPath("filename").toString());
    EXPECT_EQ(StringLiteral("filename"), allocPath("filename").toString("[:]"));

    {
        auto const p = allocPath({"3", "2", "1"});
        EXPECT_EQ(StringLiteral("3/2/1"),   p.toString());
        EXPECT_EQ(StringLiteral("3|:2|:1"), p.toString("|:"));
    }

    {
        auto const p = allocPath({"", "etc", "something", "1"});
        EXPECT_EQ(StringLiteral("/etc/something/1"), p.toString());
        EXPECT_EQ(StringLiteral("|:etc|:something|:1"), p.toString("|:"));
    }
}


/**
    * Test implementation and contract of parsable
    */
TEST(TestPath, testParsing) {
    {
        EXPECT_EQ(allocPath("some-long_path"),
                  Path::parse("some-long_path").unwrap());
    }
    {
        EXPECT_EQ(allocPath({"", "etc"}),
                  Path::parse("/etc").unwrap());
    }
    {
        EXPECT_EQ(Path::Root, Path::parse("").unwrap());
        EXPECT_EQ(Path::Root, Path::parse("/").unwrap());
        EXPECT_EQ(Path::Root, Path::parse(Path::Delimiter).unwrap());
    }
    {
        EXPECT_EQ(allocPath({"some", "file", "path.321"}),
                  Path::parse("some/file/path.321").unwrap());
    }
    {
        EXPECT_EQ(allocPath({"some", "file", "path.321"}),
                  Path::parse("some/file/path.321/").unwrap());
    }
    {
        EXPECT_EQ(allocPath({"some", "file", "", "path.321"}),
                  Path::parse("some/file//path.321/").unwrap());
    }
    {
        EXPECT_EQ(allocPath({"", "!)", "$@#&@#", "some", "file", "path"}),
                  Path::parse("/!)/$@#&@#/some/file/path").unwrap());
    }
    {
        EXPECT_EQ(allocPath({"some", "file", "path"}),
                  Path::parse("some.file.path", ".").unwrap());
    }
    {
        EXPECT_EQ(allocPath({"some", "file", "", "path"}),
                  Path::parse("some.file..path", ".").unwrap());
    }
    {
        EXPECT_EQ(allocPath({"", "some", "file", "path"}),
                  Path::parse("{?some{?file{?path{?", "{?").unwrap());
    }
    {
        EXPECT_EQ(allocPath({"", "some", "", "file", "path"}),
                  Path::parse("{?some{?{?file{?path{?", "{?").unwrap());
    }
}

/**
    * Test consistency of parsing and toString implementation
    */
TEST(TestPath, testParsing_and_ToString_are_consistent) {
    {
        const String src("some-long_path");
        const auto v = Path::parse(src.view()).unwrap();
        EXPECT_EQ(src, v.toString());
    }
    {
        const String src("some/file/path.321");
        const auto v = Path::parse(src.view()).unwrap();
        EXPECT_EQ(src, v.toString());
    }
    {
        EXPECT_EQ(String("some/file/path.321"),
                                Path::parse("some/file/path.321/")
                                .unwrap()
                                .toString());
    }
    {
        const String src("/!)/$@#&@#/some/file/path");
        const auto v = Path::parse(src.view()).unwrap();
        EXPECT_EQ(src, v.toString());
    }
    {
        const String src("some.file.path");
        const auto v = Path::parse(src.view(), "\\.").unwrap();
        EXPECT_EQ(src, v.toString("."));
    }
    {
        const String src("some.file..path");
        const auto v = Path::parse(src.view(), "\\.").unwrap();
        EXPECT_EQ(src, v.toString("."));
    }
    {
        EXPECT_EQ(String("{?some{?file{?path"),
                                Path::parse("{?some{?file{?path{?", "{?")
                                .unwrap()
                                .toString("{?"));
    }
}
