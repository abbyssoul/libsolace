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

#include <cppunit/extensions/HelperMacros.h>
#include <cstring>

using namespace Solace;


class TestPath : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestPath);
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
        CPPUNIT_ASSERT_EQUAL(-1, Path("aa").compareTo(Path("ab")));
        CPPUNIT_ASSERT_EQUAL(-1, Path({"a", "a"}).compareTo({"a", "b"}));
        CPPUNIT_ASSERT_EQUAL(-2, Path({"a", "a"}).compareTo({"a", "c"}));
        CPPUNIT_ASSERT_EQUAL(-1, Path({"a", "b", "c"}).compareTo({"a", "b", "c", "d"}));
        CPPUNIT_ASSERT_EQUAL(-2, Path({"a", "b", "c"}).compareTo({"a", "b", "c", "d", "e"}));
        CPPUNIT_ASSERT_EQUAL(-4, Path({"a", "a", "c"}).compareTo({"a", "c", "c", "d", "e"}));

        CPPUNIT_ASSERT_EQUAL(+1, Path("ab").compareTo(Path("aa")));
        CPPUNIT_ASSERT_EQUAL(+1, Path({"a", "b"}).compareTo({"a", "a"}));
        CPPUNIT_ASSERT_EQUAL(+2, Path({"a", "c"}).compareTo({"a", "a"}));
        CPPUNIT_ASSERT_EQUAL(+1, Path({"a", "b", "c", "d"}).compareTo({"a", "b", "c"}));
        CPPUNIT_ASSERT_EQUAL(+2, Path({"a", "b", "c", "d", "e"}).compareTo({"a", "b", "c"}));
        CPPUNIT_ASSERT_EQUAL(+4, Path({"a", "c", "c", "d", "e"}).compareTo({"a", "a", "c"}));
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
    }

    void testGetParent() {
        {
            const Path root("");
            CPPUNIT_ASSERT_EQUAL(root, root.getParent());

            const Path p1({"1", "2", "3", "4", "file"});
            const Path p2({"1", "2", "3", "4"});
            const Path p3("file");
            const Path p4({"", "file"});

            CPPUNIT_ASSERT_EQUAL(p2, p1.getParent());
            CPPUNIT_ASSERT_EQUAL(p3, p3.getParent());
            CPPUNIT_ASSERT_EQUAL(root, p4.getParent());
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
        CPPUNIT_ASSERT_EQUAL(String(""), Path({""}).getBasename());
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
            i++;
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
            const auto v = Path::parse("some-long_path");
            CPPUNIT_ASSERT_EQUAL(Path("some-long_path"), v);
        }
        {
            const auto v = Path::parse("/etc");
            CPPUNIT_ASSERT_EQUAL(Path({"", "etc"}), v);
        }
        {
            const auto v = Path::parse("some/file/path.321");
            CPPUNIT_ASSERT_EQUAL(Path({"some", "file", "path.321"}), v);
        }
        {
            const auto v = Path::parse("some/file/path.321/");
            CPPUNIT_ASSERT_EQUAL(Path({"some", "file", "path.321", ""}), v);
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
            const auto v = Path::parse("{?some{?file{?path{?", "\\{\\?");
            CPPUNIT_ASSERT_EQUAL(Path({"", "some", "file", "path", ""}), v);
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
            const String src("some/file/path.321/");
            const auto v = Path::parse(src);
            CPPUNIT_ASSERT_EQUAL(src, v.toString());
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
            const String src("{?some{?file{?path{?");
            const auto v = Path::parse(src, "\\{\\?");
            CPPUNIT_ASSERT_EQUAL(src, v.toString("{?"));
        }
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestPath);
