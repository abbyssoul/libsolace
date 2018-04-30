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

#include "mockTypes.hpp"
#include <cppunit/extensions/HelperMacros.h>
#include <cstring>

using namespace Solace;


class TestPath : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestPath);
        CPPUNIT_TEST(testRootIsSingleComponent);
        CPPUNIT_TEST(testRootAbsolute);

        CPPUNIT_TEST(testEmpty);
        CPPUNIT_TEST(testLength);

        CPPUNIT_TEST(testComparable);
        CPPUNIT_TEST(testStartsWith);
        CPPUNIT_TEST(testEndsWith);
        CPPUNIT_TEST(testContains);

        CPPUNIT_TEST(testNormalize);
        CPPUNIT_TEST(testGetParent);
        CPPUNIT_TEST(testComponents);
        CPPUNIT_TEST(testSubpath);
        CPPUNIT_TEST(testJoin);
        CPPUNIT_TEST(testIterable);
        CPPUNIT_TEST(testForEach);

        CPPUNIT_TEST(testBasename);
        CPPUNIT_TEST(testUnixBasename);

        CPPUNIT_TEST(testIsAbsolute);
        CPPUNIT_TEST(testIsRelative);
        CPPUNIT_TEST(testFirst);
        CPPUNIT_TEST(testLast);

        CPPUNIT_TEST(testToString);
        CPPUNIT_TEST(testParsing);
        CPPUNIT_TEST(testParsing_and_ToString_are_consistent);
    CPPUNIT_TEST_SUITE_END();

private:

    Path moveMe() {
        return Path("abc");
    }

public:

    void setUp() override {
	}

    void tearDown() override {
	}

    void testRootIsSingleComponent() {
        CPPUNIT_ASSERT_EQUAL(static_cast<Path::size_type>(1), Path::Root.getComponentsCount());
    }

    void testRootAbsolute() {
        CPPUNIT_ASSERT(Path::Root.isAbsolute());
    }

    void testEmpty() {
        Path emptyPath;
        CPPUNIT_ASSERT(emptyPath.empty());

        Path notEmptyPath("file");
        CPPUNIT_ASSERT(!notEmptyPath.empty());

        Path notEmptyPath2({"some", "path", "to", "file"});
        CPPUNIT_ASSERT(!notEmptyPath.empty());
    }

    void testLength() {
        CPPUNIT_ASSERT_EQUAL(static_cast<String::size_type>(0), Path().length());

        CPPUNIT_ASSERT_EQUAL(static_cast<String::size_type>(4), Path("file").length());

        // Special case of a root path
        CPPUNIT_ASSERT_EQUAL(static_cast<String::size_type>(1), Path({""}).length());
        CPPUNIT_ASSERT_EQUAL(static_cast<String::size_type>(5), Path({""}).length("[]lll"));

        CPPUNIT_ASSERT_EQUAL(static_cast<String::size_type>(std::strlen("some/path/to/file")),
                             Path({"some", "path", "to", "file"}).length());

        CPPUNIT_ASSERT_EQUAL(static_cast<String::size_type>(std::strlen("/file")),
                             Path({"", "file"}).length());

        CPPUNIT_ASSERT_EQUAL(static_cast<String::size_type>(std::strlen("/some/path/to/file")),
                             Path({"", "some", "path", "to", "file"}).length());

        CPPUNIT_ASSERT_EQUAL(static_cast<String::size_type>(std::strlen("[-:]some[-:]path[-:]to[-:]file")),
                             Path({"", "some", "path", "to", "file"}).length("[-:]"));
    }

	/**
	 * Test implementation and contract of IComparable
	 */
	void testComparable() {
        const Path p1({"1", "2", "3", "4", "file"});
        const Path p2({"1", "2", "3", "4", "file"});
        const Path p_different({"something", "2", "3", "file"});

        CPPUNIT_ASSERT(p1.equals(p2));
        CPPUNIT_ASSERT(p2.equals(p1));
        CPPUNIT_ASSERT_EQUAL(p1, p2);
        CPPUNIT_ASSERT_EQUAL(0, p1.compareTo(p1));
        CPPUNIT_ASSERT_EQUAL(0, p2.compareTo(p2));
        CPPUNIT_ASSERT_EQUAL(0, p1.compareTo(p2));
        CPPUNIT_ASSERT_EQUAL(0, p2.compareTo(p1));

        CPPUNIT_ASSERT_EQUAL(false, p1.equals(p_different));
        CPPUNIT_ASSERT_EQUAL(false, p2.equals(p_different));
        CPPUNIT_ASSERT_EQUAL(false, p_different.equals(p1));
        CPPUNIT_ASSERT_EQUAL(false, p_different.equals(p2));

        // Verify compareTo
        CPPUNIT_ASSERT(Path("aa").compareTo(Path("ab")) < 0);
        CPPUNIT_ASSERT(Path({"a", "a"}).compareTo({"a", "b"}) < 0);
        CPPUNIT_ASSERT(Path({"a", "a"}).compareTo({"a", "c"}) < 0);
        CPPUNIT_ASSERT(Path({"a", "b", "c"}).compareTo({"a", "b", "c", "d"}) < 0);
        CPPUNIT_ASSERT(Path({"a", "b", "c"}).compareTo({"a", "b", "c", "d", "e"}) < 0);
        CPPUNIT_ASSERT(Path({"a", "a", "c"}).compareTo({"a", "c", "c", "d", "e"}) < 0);

        CPPUNIT_ASSERT(Path("ab").compareTo(Path("aa")) > 0);
        CPPUNIT_ASSERT(Path({"a", "b"}).compareTo({"a", "a"}) > 0);
        CPPUNIT_ASSERT(Path({"a", "c"}).compareTo({"a", "a"}) > 0);
        CPPUNIT_ASSERT(Path({"a", "b", "c", "d"}).compareTo({"a", "b", "c"}) > 0);
        CPPUNIT_ASSERT(Path({"a", "b", "c", "d", "e"}).compareTo({"a", "b", "c"}) > 0);
        CPPUNIT_ASSERT(Path({"a", "c", "c", "d", "e"}).compareTo({"a", "a", "c"}) > 0);
    }

    void testStartsWith() {
        {
            const Path p({"some", "path", "to", "a", "file"});
            CPPUNIT_ASSERT(p.startsWith(p));
            CPPUNIT_ASSERT(p.startsWith("som"));
            CPPUNIT_ASSERT(p.startsWith("some"));

            CPPUNIT_ASSERT_EQUAL(true, p.startsWith({"some", "pa"}));
            CPPUNIT_ASSERT_EQUAL(true, p.startsWith({"some", "path"}));
            CPPUNIT_ASSERT_EQUAL(true, p.startsWith({"some", "path", "t"}));

            CPPUNIT_ASSERT_EQUAL(false, p.startsWith({"so", "pa"}));
            CPPUNIT_ASSERT_EQUAL(false, p.startsWith({"some", "pa", "to"}));
        }

        {
            const Path p1({"1", "2", "3", "4", "file"});
            const Path p2({"1", "2", "3"});
            const Path p3({"2", "3", "4"});

            CPPUNIT_ASSERT(p1.startsWith(p1));
            CPPUNIT_ASSERT(p1.startsWith(Path("1")));
            CPPUNIT_ASSERT(p1.startsWith(p2));

            CPPUNIT_ASSERT_EQUAL(false, p1.startsWith(p3));
            CPPUNIT_ASSERT_EQUAL(false, p2.startsWith(p3));
            CPPUNIT_ASSERT_EQUAL(false, p3.startsWith(p1));
            CPPUNIT_ASSERT_EQUAL(false, p3.startsWith(Path({"2", "3", "4", ""})));
        }
    }

    void testEndsWith() {
        {
            const Path p({"some", "path", "to", "awesome", "file.awe"});
            CPPUNIT_ASSERT(p.endsWith(p));
            CPPUNIT_ASSERT(p.endsWith("awe"));
            CPPUNIT_ASSERT(p.endsWith("file.awe"));

            CPPUNIT_ASSERT_EQUAL(true, p.endsWith({"some", "file.awe"}));
            CPPUNIT_ASSERT_EQUAL(true, p.endsWith({"awesome", "file.awe"}));
            CPPUNIT_ASSERT_EQUAL(true, p.endsWith({"to", "awesome", "file.awe"}));

            CPPUNIT_ASSERT_EQUAL(false, p.endsWith({"to", "awe", "file.awe"}));
            CPPUNIT_ASSERT_EQUAL(false, p.endsWith({"to", "some", "file.awe"}));
        }

        {
            const Path p1({"1", "2", "3", "4", "file"});
            const Path p2({"3", "4", "file"});
            const Path p3({"2", "3", "4"});

            CPPUNIT_ASSERT(p1.endsWith(p1));
            CPPUNIT_ASSERT(p1.endsWith(Path("file")));
            CPPUNIT_ASSERT(p1.endsWith(p2));

            CPPUNIT_ASSERT_EQUAL(false, p1.endsWith(p3));
            CPPUNIT_ASSERT_EQUAL(false, p2.endsWith(p3));
            CPPUNIT_ASSERT_EQUAL(false, p3.endsWith(p1));
            CPPUNIT_ASSERT_EQUAL(false, p3.endsWith(Path{"", "1", "2", "3", "4"}));
        }
    }

    void testContains() {
        const Path p1({"1", "2", "3", "4", "file"});
        const Path p2({"2", "3", "4"});
        const Path p3({"4", "3", "file"});

        CPPUNIT_ASSERT(p1.contains(p1));
        CPPUNIT_ASSERT(p1.contains("file"));
        CPPUNIT_ASSERT(p1.contains("2"));
        CPPUNIT_ASSERT(p1.contains(p2));

        CPPUNIT_ASSERT_EQUAL(false, p1.contains(p3));
        CPPUNIT_ASSERT_EQUAL(false, p2.contains(p3));

        // Shorter path can not contain a longer one!
        CPPUNIT_ASSERT_EQUAL(false, p2.contains(p1));

        CPPUNIT_ASSERT_EQUAL(false, p1.contains(Path({"1", "2", "5"})));
    }

    void testGetParent() {
        {
            const Path root("");
            CPPUNIT_ASSERT_EQUAL(root, root.getParent());

            const Path p3("file");

            CPPUNIT_ASSERT_EQUAL(Path({"1", "2", "3", "4"}), Path({"1", "2", "3", "4", "file"}).getParent());
            CPPUNIT_ASSERT_EQUAL(p3, p3.getParent());
            CPPUNIT_ASSERT_EQUAL(root, Path({"", "file"}).getParent());
        }
        {
            Path root;
            CPPUNIT_ASSERT(root.empty());

            Path p = root.getParent();
            CPPUNIT_ASSERT(p.empty());

            root = moveMe();
            CPPUNIT_ASSERT(!root.empty());
            CPPUNIT_ASSERT(p.empty());
        }
    }

    void testBasename() {
        CPPUNIT_ASSERT_EQUAL(String::Empty, Path().getBasename());
        CPPUNIT_ASSERT_EQUAL(Path::Delimiter, Path({""}).getBasename());
        CPPUNIT_ASSERT_EQUAL(String("file"), Path("file").getBasename());
        CPPUNIT_ASSERT_EQUAL(String("file"), Path({"file"}).getBasename());
        CPPUNIT_ASSERT_EQUAL(String("file"), Path({"", "file"}).getBasename());

        CPPUNIT_ASSERT_EQUAL(String("."), Path({"."}).getBasename());
        CPPUNIT_ASSERT_EQUAL(String(".."), Path({".."}).getBasename());
        CPPUNIT_ASSERT_EQUAL(String("."), Path({"", "."}).getBasename());
        CPPUNIT_ASSERT_EQUAL(String(".."), Path({"", ".."}).getBasename());

        CPPUNIT_ASSERT_EQUAL(String("etc"), Path({"", "etc"}).getBasename());
        CPPUNIT_ASSERT_EQUAL(String(""), Path({"", "etc", ""}).getBasename());
        CPPUNIT_ASSERT_EQUAL(String("file"), Path({"", "etc", "file"}).getBasename());
        CPPUNIT_ASSERT_EQUAL(String(".."), Path({"", "etc", ".."}).getBasename());
        CPPUNIT_ASSERT_EQUAL(String("."), Path({"etc", "..", "."}).getBasename());

        CPPUNIT_ASSERT_EQUAL(String("file"), Path({"1", "2", "3", "4", "file"}).getBasename());
    }


    void testUnixBasename() {
        CPPUNIT_ASSERT_EQUAL(String("lib"), Path::parse("/usr/lib").getBasename());

        // FIXME(abbyssoul): This is directly from basename spec which we don't comply with atm :'(
        CPPUNIT_ASSERT_EQUAL(String("usr"), Path::parse("/usr/").getBasename());

        CPPUNIT_ASSERT_EQUAL(String("/"),   Path::parse("/").getBasename());
    }


    void testComponents() {
        const String components[] = {"1", "2", "3", "4", "file"};
        const Path p({components[0], components[1], components[2], components[3], components[4]});

        CPPUNIT_ASSERT_EQUAL(static_cast<Path::size_type>(5), p.getComponentsCount());
        for (Path::size_type i = 0; i < p.getComponentsCount(); ++i) {
            CPPUNIT_ASSERT_EQUAL(components[i], p.getComponent(i));
        }
    }

    void testFirst() {
        CPPUNIT_ASSERT_EQUAL(String::Empty, Path().first());
        CPPUNIT_ASSERT_EQUAL(String::Empty, Path({""}).first());

        CPPUNIT_ASSERT_EQUAL(String("file"), Path({"file"}).first());
        CPPUNIT_ASSERT_EQUAL(String("etc"), Path({"etc", "file"}).first());
        CPPUNIT_ASSERT_EQUAL(String(""), Path({"", "etc", "file"}).first());
    }

    void testLast() {
        CPPUNIT_ASSERT_EQUAL(String::Empty, Path().last());
        CPPUNIT_ASSERT_EQUAL(String::Empty, Path({""}).last());

        CPPUNIT_ASSERT_EQUAL(String("file"), Path({"file"}).last());
        CPPUNIT_ASSERT_EQUAL(String("file"), Path({"etc", "file"}).last());
        CPPUNIT_ASSERT_EQUAL(String("file"), Path({"", "etc", "file"}).last());
    }

    void testSubpath() {
        CPPUNIT_ASSERT_EQUAL(Path({"1", "2", "3"}), Path({"1", "2", "3", "4", "file"}).subpath(0, 3));
        CPPUNIT_ASSERT_EQUAL(Path({"3", "4", "file"}), Path({"1", "2", "3", "4", "file"}).subpath(2, 5));
        CPPUNIT_ASSERT_EQUAL(Path({"2", "3"}), Path({"1", "2", "3", "4", "file"}).subpath(1, 3));

        // Error modes:

        // End index outside of path lenght
        CPPUNIT_ASSERT_THROW(Path({"1", "2", "3", "4", "file"}).subpath(1, 23), IndexOutOfRangeException);
        // Start index oustide of path lenght
        CPPUNIT_ASSERT_THROW(Path({"1", "2", "3", "4", "file"}).subpath(17, 18), IndexOutOfRangeException);
        // Start greater then end index
        CPPUNIT_ASSERT_THROW(Path({"1", "2", "3", "4", "file"}).subpath(3, 1), IndexOutOfRangeException);
    }


    void testJoin() {
        CPPUNIT_ASSERT_EQUAL(Path({"etc", "file"}), Path("etc").join(Path("file")));
        CPPUNIT_ASSERT_EQUAL(Path({"etc", "file"}), Path("etc").join("file"));

        CPPUNIT_ASSERT_EQUAL(Path({"etc", "file"}), Path("etc") / Path("file"));
        CPPUNIT_ASSERT_EQUAL(Path({"etc", "file"}), Path("etc") / "file");

        CPPUNIT_ASSERT_EQUAL(Path({"etc", "file"}), Path::join(Path("etc"), Path("file")));
        CPPUNIT_ASSERT_EQUAL(Path({"etc", "file"}), Path::join(Path("etc"), "file"));

        CPPUNIT_ASSERT_EQUAL(Path({"etc", "some", "long", "path"}),
                             Path::join({"etc", "some", "long", "path"}));
        CPPUNIT_ASSERT_EQUAL(Path({"etc", "some", "long", "path"}),
                             Path::join({Path({"etc", "some"}), Path("long"), Path("path")}));
    }

    void testIterable() {
        const Path p({"e", "so", "lon", "path", "foilx"});

        String::size_type i = 0;
        for (auto& v : p) {
            ++i;
            CPPUNIT_ASSERT_EQUAL(i, v.length());
        }
    }

    void testForEach() {
        int index = 0;
        Array<int> counts(6);

        Path({"e", "so", "long", "pat", "fx", "x"}).forEach([&index, &counts] (const String& component){
            counts[index++] = component.length();
        });

        CPPUNIT_ASSERT_EQUAL(Array<int>({1, 2, 4, 3, 2, 1}), (counts));
    }


    void testIsAbsolute() {
        CPPUNIT_ASSERT(!Path("etc").isAbsolute());
        CPPUNIT_ASSERT(!Path({"etc", "2", "file"}).isAbsolute());
        CPPUNIT_ASSERT(Path({"", "etc", "dir", "file"}).isAbsolute());
        CPPUNIT_ASSERT(Path({"", "2", "f", ""}).isAbsolute());
    }


    void testIsRelative() {
        CPPUNIT_ASSERT(Path("etc").isRelative());
        CPPUNIT_ASSERT(Path({"1", "2", "f"}).isRelative());
        CPPUNIT_ASSERT(!Path({"", "1", "2", "f"}).isRelative());
        CPPUNIT_ASSERT(Path({"1", "2", "f", ""}).isRelative());
    }


    void testNormalize() {

        CPPUNIT_ASSERT_EQUAL(Path({"1", "2", "f"}),
                             Path({"1", ".", "2", "f"}).normalize());

        CPPUNIT_ASSERT_EQUAL(Path({"1", "f"}),
                             Path({"1", "2", "..", "f"}).normalize());

        CPPUNIT_ASSERT_EQUAL(Path({"1", "3"}),
                             Path({".", "1", "2", "..", "3", ".", "f", ".."}).normalize());
    }

	/**
	 * Test implementation and contract of IFormattable
	 */
	void testToString() {
        CPPUNIT_ASSERT_EQUAL(String("/"), Path("").toString());
        CPPUNIT_ASSERT_EQUAL(String("[:]"), Path("").toString("[:]"));
        CPPUNIT_ASSERT_EQUAL(String("filename"), Path("filename").toString());
        CPPUNIT_ASSERT_EQUAL(String("filename"), Path("filename").toString("[:]"));

        {
			const Path p({"3", "2", "1"});
            CPPUNIT_ASSERT_EQUAL(String("3/2/1"), p.toString());

            CPPUNIT_ASSERT_EQUAL(String("3|:2|:1"), p.toString("|:"));
		}

        {
            const Path p({"", "etc", "something", "1"});
            CPPUNIT_ASSERT_EQUAL(String("/etc/something/1"), p.toString());

            CPPUNIT_ASSERT_EQUAL(String("|:etc|:something|:1"), p.toString("|:"));
        }
    }


    /**
     * Test implementation and contract of parsable
     */
    void testParsing() {
        {
            CPPUNIT_ASSERT_EQUAL(Path("some-long_path"),
                                 Path::parse("some-long_path"));
        }
        {
            CPPUNIT_ASSERT_EQUAL(Path({"", "etc"}),
                                 Path::parse("/etc"));
        }
        {
            CPPUNIT_ASSERT_EQUAL(Path::Root, Path::parse(""));
            CPPUNIT_ASSERT_EQUAL(Path::Root, Path::parse("/"));
            CPPUNIT_ASSERT_EQUAL(Path::Root, Path::parse(Path::Delimiter));
        }
        {
            CPPUNIT_ASSERT_EQUAL(Path({"some", "file", "path.321"}),
                                 Path::parse("some/file/path.321"));
        }
        {
            CPPUNIT_ASSERT_EQUAL(Path({"some", "file", "path.321"}),
                                 Path::parse("some/file/path.321/"));
        }
        {
            CPPUNIT_ASSERT_EQUAL(Path({"some", "file", "", "path.321"}),
                                 Path::parse("some/file//path.321/"));
        }
        {
            const auto v = Path::parse("/!)/$@#&@#/some/file/path");
            CPPUNIT_ASSERT_EQUAL(Path({"", "!)", "$@#&@#", "some", "file", "path"}), v);
        }
        {
            const auto v = Path::parse("some.file.path", "\\.");
            CPPUNIT_ASSERT_EQUAL(Path({"some", "file", "path"}), v);
        }
        {
            const auto v = Path::parse("some.file..path", "\\.");
            CPPUNIT_ASSERT_EQUAL(Path({"some", "file", "", "path"}), v);
        }
        {
            CPPUNIT_ASSERT_EQUAL(Path({"", "some", "file", "path"}),
                                 Path::parse("{?some{?file{?path{?", "\\{\\?"));
        }
        {
            CPPUNIT_ASSERT_EQUAL(Path({"", "some", "", "file", "path"}),
                                 Path::parse("{?some{?{?file{?path{?", "\\{\\?"));
        }
    }

    /**
     * Test consistency of parsing and toString implementation
     */
    void testParsing_and_ToString_are_consistent() {
        {
            const String src("some-long_path");
            const auto v = Path::parse(src);
            CPPUNIT_ASSERT_EQUAL(src, v.toString());
        }
        {
            const String src("some/file/path.321");
            const auto v = Path::parse(src);
            CPPUNIT_ASSERT_EQUAL(src, v.toString());
        }
        {
            CPPUNIT_ASSERT_EQUAL(String("some/file/path.321"),
                                 Path::parse("some/file/path.321/").toString());
        }
        {
            const String src("/!)/$@#&@#/some/file/path");
            const auto v = Path::parse(src);
            CPPUNIT_ASSERT_EQUAL(src, v.toString());
        }
        {
            const String src("some.file.path");
            const auto v = Path::parse(src, "\\.");
            CPPUNIT_ASSERT_EQUAL(src, v.toString("."));
        }
        {
            const String src("some.file..path");
            const auto v = Path::parse(src, "\\.");
            CPPUNIT_ASSERT_EQUAL(src, v.toString("."));
        }
        {
            CPPUNIT_ASSERT_EQUAL(String("{?some{?file{?path"),
                                 Path::parse("{?some{?file{?path{?", "\\{\\?").toString("{?"));
        }
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestPath);
