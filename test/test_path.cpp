#include <solace/path.hpp>			// Class being tested
#include <solace/exception.hpp>     // Checked expcetions

#include "mockTypes.hpp"
#include <gtest/gtest.h>
#include <cstring>

using namespace Solace;

class TestPath : public testing::Test  {

public:

    Path moveMe() {
        return Path("abc");
    }

    void setUp() {
	}

    void tearDown() {
	}

};

TEST_F(TestPath, testRootIsSingleComponent) {
    EXPECT_EQ(static_cast<Path::size_type>(1), Path::Root.getComponentsCount());
}

TEST_F(TestPath, testRootAbsolute) {
    EXPECT_TRUE(Path::Root.isAbsolute());
}

TEST_F(TestPath, testEmpty) {
    Path emptyPath;
    EXPECT_TRUE(emptyPath.empty());

    Path notEmptyPath("file");
    EXPECT_TRUE(!notEmptyPath.empty());

    Path notEmptyPath2({"some", "path", "to", "file"});
    EXPECT_TRUE(!notEmptyPath.empty());
}

TEST_F(TestPath, testLength) {
    EXPECT_EQ(static_cast<String::size_type>(0), Path().length());

    EXPECT_EQ(static_cast<String::size_type>(4), Path("file").length());

    // Special case of a root path
    EXPECT_EQ(static_cast<String::size_type>(1), Path({""}).length());
    EXPECT_EQ(static_cast<String::size_type>(5), Path({""}).length("[]lll"));

    EXPECT_EQ(static_cast<String::size_type>(std::strlen("some/path/to/file")),
                            Path({"some", "path", "to", "file"}).length());

    EXPECT_EQ(static_cast<String::size_type>(std::strlen("/file")),
                            Path({"", "file"}).length());

    EXPECT_EQ(static_cast<String::size_type>(std::strlen("/some/path/to/file")),
                            Path({"", "some", "path", "to", "file"}).length());

    EXPECT_EQ(static_cast<String::size_type>(std::strlen("[-:]some[-:]path[-:]to[-:]file")),
                            Path({"", "some", "path", "to", "file"}).length("[-:]"));
}

/**
    * Test implementation and contract of IComparable
    */
TEST_F(TestPath, testComparable) {
    const Path p1({"1", "2", "3", "4", "file"});
    const Path p2({"1", "2", "3", "4", "file"});
    const Path p_different({"something", "2", "3", "file"});

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
    EXPECT_TRUE(Path("aa").compareTo(Path("ab")) < 0);
    EXPECT_TRUE(Path({"a", "a"}).compareTo({"a", "b"}) < 0);
    EXPECT_TRUE(Path({"a", "a"}).compareTo({"a", "c"}) < 0);
    EXPECT_TRUE(Path({"a", "b", "c"}).compareTo({"a", "b", "c", "d"}) < 0);
    EXPECT_TRUE(Path({"a", "b", "c"}).compareTo({"a", "b", "c", "d", "e"}) < 0);
    EXPECT_TRUE(Path({"a", "a", "c"}).compareTo({"a", "c", "c", "d", "e"}) < 0);

    EXPECT_TRUE(Path("ab").compareTo(Path("aa")) > 0);
    EXPECT_TRUE(Path({"a", "b"}).compareTo({"a", "a"}) > 0);
    EXPECT_TRUE(Path({"a", "c"}).compareTo({"a", "a"}) > 0);
    EXPECT_TRUE(Path({"a", "b", "c", "d"}).compareTo({"a", "b", "c"}) > 0);
    EXPECT_TRUE(Path({"a", "b", "c", "d", "e"}).compareTo({"a", "b", "c"}) > 0);
    EXPECT_TRUE(Path({"a", "c", "c", "d", "e"}).compareTo({"a", "a", "c"}) > 0);
}

TEST_F(TestPath, testStartsWith) {
    {
        const Path p({"some", "path", "to", "a", "file"});
        EXPECT_TRUE(p.startsWith(p));
        EXPECT_TRUE(p.startsWith("som"));
        EXPECT_TRUE(p.startsWith("some"));

        EXPECT_EQ(true, p.startsWith({"some", "pa"}));
        EXPECT_EQ(true, p.startsWith({"some", "path"}));
        EXPECT_EQ(true, p.startsWith({"some", "path", "t"}));

        EXPECT_EQ(false, p.startsWith({"so", "pa"}));
        EXPECT_EQ(false, p.startsWith({"some", "pa", "to"}));
    }

    {
        const Path p1({"1", "2", "3", "4", "file"});
        const Path p2({"1", "2", "3"});
        const Path p3({"2", "3", "4"});

        EXPECT_TRUE(p1.startsWith(p1));
        EXPECT_TRUE(p1.startsWith(Path("1")));
        EXPECT_TRUE(p1.startsWith(p2));

        EXPECT_EQ(false, p1.startsWith(p3));
        EXPECT_EQ(false, p2.startsWith(p3));
        EXPECT_EQ(false, p3.startsWith(p1));
        EXPECT_EQ(false, p3.startsWith(Path({"2", "3", "4", ""})));
    }
}

TEST_F(TestPath, testEndsWith) {
    {
        const Path p({"some", "path", "to", "awesome", "file.awe"});
        EXPECT_TRUE(p.endsWith(p));
        EXPECT_TRUE(p.endsWith("awe"));
        EXPECT_TRUE(p.endsWith("file.awe"));

        EXPECT_EQ(true, p.endsWith({"some", "file.awe"}));
        EXPECT_EQ(true, p.endsWith({"awesome", "file.awe"}));
        EXPECT_EQ(true, p.endsWith({"to", "awesome", "file.awe"}));

        EXPECT_EQ(false, p.endsWith({"to", "awe", "file.awe"}));
        EXPECT_EQ(false, p.endsWith({"to", "some", "file.awe"}));
    }

    {
        const Path p1({"1", "2", "3", "4", "file"});
        const Path p2({"3", "4", "file"});
        const Path p3({"2", "3", "4"});

        EXPECT_TRUE(p1.endsWith(p1));
        EXPECT_TRUE(p1.endsWith(Path("file")));
        EXPECT_TRUE(p1.endsWith(p2));

        EXPECT_EQ(false, p1.endsWith(p3));
        EXPECT_EQ(false, p2.endsWith(p3));
        EXPECT_EQ(false, p3.endsWith(p1));
        EXPECT_EQ(false, p3.endsWith(Path{"", "1", "2", "3", "4"}));
    }
}

TEST_F(TestPath, testContains) {
    const Path p1({"1", "2", "3", "4", "file"});
    const Path p2({"2", "3", "4"});
    const Path p3({"4", "3", "file"});

    EXPECT_TRUE(p1.contains(p1));
    EXPECT_TRUE(p1.contains("file"));
    EXPECT_TRUE(p1.contains("2"));
    EXPECT_TRUE(p1.contains(p2));

    EXPECT_EQ(false, p1.contains(p3));
    EXPECT_EQ(false, p2.contains(p3));

    // Shorter path can not contain a longer one!
    EXPECT_EQ(false, p2.contains(p1));

    EXPECT_EQ(false, p1.contains(Path({"1", "2", "5"})));
}

TEST_F(TestPath, testGetParent) {
    {
        const Path root("");
        EXPECT_EQ(root, root.getParent());

        const Path p3("file");

        EXPECT_EQ(Path({"1", "2", "3", "4"}), Path({"1", "2", "3", "4", "file"}).getParent());
        EXPECT_EQ(p3, p3.getParent());
        EXPECT_EQ(root, Path({"", "file"}).getParent());
    }
    {
        Path root;
        EXPECT_TRUE(root.empty());

        Path p = root.getParent();
        EXPECT_TRUE(p.empty());

        root = moveMe();
        EXPECT_TRUE(!root.empty());
        EXPECT_TRUE(p.empty());
    }
}

TEST_F(TestPath, testBasename) {
    EXPECT_EQ(StringView(), Path().getBasename());
    EXPECT_EQ(Path::Delimiter, Path({""}).getBasename());
    EXPECT_EQ(StringView("file"), Path("file").getBasename());
    EXPECT_EQ(StringView("file"), Path({"file"}).getBasename());
    EXPECT_EQ(StringView("file"), Path({"", "file"}).getBasename());

    EXPECT_EQ(StringView("."), Path({"."}).getBasename());
    EXPECT_EQ(StringView(".."), Path({".."}).getBasename());
    EXPECT_EQ(StringView("."), Path({"", "."}).getBasename());
    EXPECT_EQ(StringView(".."), Path({"", ".."}).getBasename());

    EXPECT_EQ(StringView("etc"), Path({"", "etc"}).getBasename());
    EXPECT_EQ(StringView(""), Path({"", "etc", ""}).getBasename());
    EXPECT_EQ(StringView("file"), Path({"", "etc", "file"}).getBasename());
    EXPECT_EQ(StringView(".."), Path({"", "etc", ".."}).getBasename());
    EXPECT_EQ(StringView("."), Path({"etc", "..", "."}).getBasename());

    EXPECT_EQ(StringView("file"), Path({"1", "2", "3", "4", "file"}).getBasename());
}


TEST_F(TestPath, testUnixBasename) {
    EXPECT_EQ(StringView("lib"),
                            Path::parse("/usr/lib").unwrap().getBasename());

    // FIXME(abbyssoul): This is directly from basename spec which we don't comply with atm :'(
    EXPECT_EQ(StringView("usr"),
                            Path::parse("/usr/").unwrap().getBasename());

    EXPECT_EQ(StringView("/"),
                            Path::parse("/").unwrap().getBasename());
}


TEST_F(TestPath, testComponents) {
    const String components[] = {"1", "2", "3", "4", "file"};
    const Path p({components[0], components[1], components[2], components[3], components[4]});

    EXPECT_EQ(static_cast<Path::size_type>(5), p.getComponentsCount());
    for (Path::size_type i = 0; i < p.getComponentsCount(); ++i) {
        EXPECT_EQ(components[i], p.getComponent(i));
    }
}

TEST_F(TestPath, testFirst) {
    EXPECT_EQ(String::Empty, Path().first());
    EXPECT_EQ(String::Empty, Path({""}).first());

    EXPECT_EQ(String("file"), Path({"file"}).first());
    EXPECT_EQ(String("etc"), Path({"etc", "file"}).first());
    EXPECT_EQ(String(""), Path({"", "etc", "file"}).first());
}

TEST_F(TestPath, testLast) {
    EXPECT_EQ(String::Empty, Path().last());
    EXPECT_EQ(String::Empty, Path({""}).last());

    EXPECT_EQ(String("file"), Path({"file"}).last());
    EXPECT_EQ(String("file"), Path({"etc", "file"}).last());
    EXPECT_EQ(String("file"), Path({"", "etc", "file"}).last());
}

TEST_F(TestPath, testSubpath) {
    EXPECT_EQ(Path({"1", "2", "3"}), Path({"1", "2", "3", "4", "file"}).subpath(0, 3));
    EXPECT_EQ(Path({"3", "4", "file"}), Path({"1", "2", "3", "4", "file"}).subpath(2, 5));
    EXPECT_EQ(Path({"2", "3"}), Path({"1", "2", "3", "4", "file"}).subpath(1, 3));

    // Error modes:

    // End index outside of path lenght
    EXPECT_THROW(Path({"1", "2", "3", "4", "file"}).subpath(1, 23), IndexOutOfRangeException);
    // Start index oustide of path lenght
    EXPECT_THROW(Path({"1", "2", "3", "4", "file"}).subpath(17, 18), IndexOutOfRangeException);
    // Start greater then end index
    EXPECT_THROW(Path({"1", "2", "3", "4", "file"}).subpath(3, 1), IndexOutOfRangeException);
}


TEST_F(TestPath, testJoin) {
    EXPECT_EQ(Path({"etc", "file"}), Path("etc").join(Path("file")));
    EXPECT_EQ(Path({"etc", "file"}), Path("etc").join("file"));

    EXPECT_EQ(Path({"etc", "file"}), Path("etc") / Path("file"));
    EXPECT_EQ(Path({"etc", "file"}), Path("etc") / "file");

    EXPECT_EQ(Path({"etc", "file"}), Path::join(Path("etc"), Path("file")));
    EXPECT_EQ(Path({"etc", "file"}), Path::join(Path("etc"), "file"));

    EXPECT_EQ(Path({"etc", "some", "long", "path"}),
                            Path({"etc", "some", "long", "path"}));
    EXPECT_EQ(Path({"etc", "some", "long", "path"}),
                            Path({"etc", "some"}) / Path("long") / Path("path"));
}

TEST_F(TestPath, testIterable) {
    const Path p({"e", "so", "lon", "path", "foilx"});

    String::size_type i = 0;
    for (auto& v : p) {
        ++i;
        EXPECT_EQ(i, v.length());
    }
}

TEST_F(TestPath, testForEach) {
    int index = 0;
    Array<int> counts(6);

    Path({"e", "so", "long", "pat", "fx", "x"}).forEach([&index, &counts] (const String& component){
        counts[index++] = component.length();
    });

    EXPECT_EQ(Array<int>({1, 2, 4, 3, 2, 1}), (counts));
}


TEST_F(TestPath, testIsAbsolute) {
    EXPECT_TRUE(!Path("etc").isAbsolute());
    EXPECT_TRUE(!Path({"etc", "2", "file"}).isAbsolute());
    EXPECT_TRUE(Path({"", "etc", "dir", "file"}).isAbsolute());
    EXPECT_TRUE(Path({"", "2", "f", ""}).isAbsolute());
}


TEST_F(TestPath, testIsRelative) {
    EXPECT_TRUE(Path("etc").isRelative());
    EXPECT_TRUE(Path({"1", "2", "f"}).isRelative());
    EXPECT_TRUE(!Path({"", "1", "2", "f"}).isRelative());
    EXPECT_TRUE(Path({"1", "2", "f", ""}).isRelative());
}


TEST_F(TestPath, testNormalize) {

    EXPECT_EQ(Path({"1", "2", "f"}),
                            Path({"1", ".", "2", "f"}).normalize());

    EXPECT_EQ(Path({"1", "f"}),
                            Path({"1", "2", "..", "f"}).normalize());

    EXPECT_EQ(Path({"1", "3"}),
                            Path({".", "1", "2", "..", "3", ".", "f", ".."}).normalize());
}

/**
    * Test implementation and contract of IFormattable
    */
TEST_F(TestPath, testToString) {
    EXPECT_EQ(String("/"), Path("").toString());
    EXPECT_EQ(String("[:]"), Path("").toString("[:]"));
    EXPECT_EQ(String("filename"), Path("filename").toString());
    EXPECT_EQ(String("filename"), Path("filename").toString("[:]"));

    {
        const Path p({"3", "2", "1"});
        EXPECT_EQ(String("3/2/1"), p.toString());

        EXPECT_EQ(String("3|:2|:1"), p.toString("|:"));
    }

    {
        const Path p({"", "etc", "something", "1"});
        EXPECT_EQ(String("/etc/something/1"), p.toString());

        EXPECT_EQ(String("|:etc|:something|:1"), p.toString("|:"));
    }
}


/**
    * Test implementation and contract of parsable
    */
TEST_F(TestPath, testParsing) {
    {
        EXPECT_EQ(Path("some-long_path"),
                                Path::parse("some-long_path").unwrap());
    }
    {
        EXPECT_EQ(Path({"", "etc"}),
                                Path::parse("/etc").unwrap());
    }
    {
        EXPECT_EQ(Path::Root, Path::parse("").unwrap());
        EXPECT_EQ(Path::Root, Path::parse("/").unwrap());
        EXPECT_EQ(Path::Root, Path::parse(Path::Delimiter).unwrap());
    }
    {
        EXPECT_EQ(Path({"some", "file", "path.321"}),
                                Path::parse("some/file/path.321").unwrap());
    }
    {
        EXPECT_EQ(Path({"some", "file", "path.321"}),
                                Path::parse("some/file/path.321/").unwrap());
    }
    {
        EXPECT_EQ(Path({"some", "file", "", "path.321"}),
                                Path::parse("some/file//path.321/").unwrap());
    }
    {
        EXPECT_EQ(Path({"", "!)", "$@#&@#", "some", "file", "path"}),
                                Path::parse("/!)/$@#&@#/some/file/path").unwrap());
    }
    {
        EXPECT_EQ(Path({"some", "file", "path"}),
                                Path::parse("some.file.path", ".").unwrap());
    }
    {
        EXPECT_EQ(Path({"some", "file", "", "path"}),
                                Path::parse("some.file..path", ".").unwrap());
    }
    {
        EXPECT_EQ(Path({"", "some", "file", "path"}),
                                Path::parse("{?some{?file{?path{?", "{?").unwrap());
    }
    {
        EXPECT_EQ(Path({"", "some", "", "file", "path"}),
                                Path::parse("{?some{?{?file{?path{?", "{?").unwrap());
    }
}

/**
    * Test consistency of parsing and toString implementation
    */
TEST_F(TestPath, testParsing_and_ToString_are_consistent) {
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