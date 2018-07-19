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
 *	@file test/test_array.cpp
 *	@brief		Test suit for Solace::Array
 ******************************************************************************/
#include <solace/array.hpp>    // Class being tested.
#include <solace/string.hpp>   // Non POD subject.

#include <gtest/gtest.h>
#include "mockTypes.hpp"

using namespace Solace;

template <typename T, size_t N>
typename Array<T>::size_type nativeArrayLength(const T (& SOLACE_UNUSED(t))[N]) { return N; }

class TestArray : public ::testing::Test  {

protected:

	static const Array<int>::size_type ZERO;
	static const Array<int>::size_type TEST_SIZE_0;
	static const Array<int>::size_type TEST_SIZE_1;

	struct NonPodStruct {

        static int TotalCount;

		static const int IVALUE_DEFAULT;
		static const String STR_DEFAULT;

		int iValue;
		String str;

		NonPodStruct(int i, const String& inStr) :
			iValue(i), str(inStr)
		{
			++TotalCount;
		}

		NonPodStruct() : iValue(IVALUE_DEFAULT), str(STR_DEFAULT)
		{
			++TotalCount;
		}

        NonPodStruct(NonPodStruct&& other) : iValue(other.iValue), str(std::move(other.str))
        {
            ++TotalCount;
        }

        NonPodStruct(const NonPodStruct& other) : iValue(other.iValue), str(other.str)
        {
            ++TotalCount;
        }

        virtual ~NonPodStruct() {
			--TotalCount;
		}

        NonPodStruct& operator= (const NonPodStruct& rhs) {
            iValue = rhs.iValue;
            str = rhs.str;

            return (*this);
        }

        bool operator== (const NonPodStruct& other) const {
            return iValue == other.iValue && str == other.str;
        }
	};


	struct DerivedNonPodStruct  : public NonPodStruct {
		float fValue;

        virtual ~DerivedNonPodStruct() = default;

		DerivedNonPodStruct() :
            NonPodStruct(312, "Derived String"), fValue(3.1415f)
		{
		}

        DerivedNonPodStruct(int x, float f, const String& inStr) :
                NonPodStruct(x, inStr), fValue(f)
        {
        }

	};

public:

    void setUp() {
        // TODO(abbyssoul): Debug::BeginMemCheck();
        EXPECT_EQ(0, NonPodStruct::TotalCount);
        EXPECT_EQ(0, SimpleType::InstanceCount);
        EXPECT_EQ(0, SometimesConstructable::InstanceCount);
	}

    void tearDown() {
        // TODO(abbyssoul): Debug::EndMemCheck();
        EXPECT_EQ(0, NonPodStruct::TotalCount);
        EXPECT_EQ(0, SimpleType::InstanceCount);
        EXPECT_EQ(0, SometimesConstructable::InstanceCount);
    }


    struct Composite {
        Array<NonPodStruct> nonPods;
        int uselessPadding;
        Array<SimpleType> simpletons;

        Composite(int x, std::initializer_list<NonPodStruct> nons, std::initializer_list<SimpleType> simps) :
            nonPods(nons),
            uselessPadding(x),
            simpletons(simps)
        {}
    };
};

TEST_F(TestArray, testEmpty) {
        {
            const Array<int> empty_array(0);

            EXPECT_TRUE(empty_array.empty());
            EXPECT_EQ(ZERO, empty_array.size());
            EXPECT_TRUE(empty_array.begin() == empty_array.end());
        }

        {
            Array<NonPodStruct> empty_array;

            EXPECT_TRUE(empty_array.empty());
            EXPECT_EQ(ZERO, empty_array.size());
            EXPECT_TRUE(empty_array.begin() == empty_array.end());
        }

        {
            const Array<NonPodStruct> empty_array(0);

            EXPECT_TRUE(empty_array.empty());
            EXPECT_EQ(ZERO, empty_array.size());
            EXPECT_TRUE(empty_array.begin() == empty_array.end());
        }

        {
            const Array<DerivedNonPodStruct> empty_array(0);

            EXPECT_TRUE(empty_array.empty());
            EXPECT_EQ(ZERO, empty_array.size());
            EXPECT_TRUE(empty_array.begin() == empty_array.end());
        }
}

TEST_F(TestArray, testCopy) {
    {
        Array<int> a1(0);
        Array<int> a2(20);

        EXPECT_TRUE(a1.empty());
        EXPECT_TRUE(!a2.empty());

        for (Array<int>::size_type i = 0; i < a2.size(); ++i) {
            a2[i] = static_cast<int>(2*i) - 1;
        }

        a1 = a2;
        EXPECT_TRUE(!a1.empty());
        EXPECT_EQ(a1.size(), a2.size());
        for (Array<int>::size_type i = 0; i < a1.size(); ++i) {
            EXPECT_EQ(static_cast<int>(2*i) - 1, a1[i]);
        }
    }
}

TEST_F(TestArray, testBasics) {
    Array<uint> array(TEST_SIZE_0);

    EXPECT_TRUE(!array.empty());
    EXPECT_EQ(TEST_SIZE_0, array.size());

    for (auto i = ZERO, end = array.size(); i < end; ++i) {
        EXPECT_EQ(0U, array[i]);
    }

    uint count = 0;
    for (auto i = array.begin(), end = array.end(); i != end; ++i) {
        EXPECT_EQ(0U, *i);

        *i = count++;
    }
    EXPECT_EQ(static_cast<Array<uint>::size_type>(count), array.size());

    for (auto i = ZERO, end = array.size(); i < end; ++i) {
        EXPECT_EQ(static_cast<uint>(i), array[i]);
    }

    // TODO(abbyssoul): Test access pass end!!
}

TEST_F(TestArray, testString) {
    Array<String> array(TEST_SIZE_0);

    EXPECT_TRUE(!array.empty());
    EXPECT_EQ(TEST_SIZE_0, array.size());

    for (auto i = ZERO, end = array.size(); i < end; ++i) {
        EXPECT_EQ(String::Empty, array[i]);
    }

    auto count = ZERO;
    for (auto& i : array) {
        EXPECT_TRUE(i.empty());

        i = "Item " + std::to_string(count++);
    }

    EXPECT_EQ(count, array.size());

    for (auto i = ZERO, end = array.size(); i < end; ++i) {
        EXPECT_EQ(String("Item " + std::to_string(i)), array[i]);
    }
}

TEST_F(TestArray, testNonPods) {
    EXPECT_EQ(0, NonPodStruct::TotalCount);
    {
        Array<NonPodStruct> array(TEST_SIZE_1);

        EXPECT_EQ(TEST_SIZE_1, array.size());
        EXPECT_EQ(static_cast<Array<NonPodStruct>::size_type>(NonPodStruct::TotalCount), array.size());

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(NonPodStruct::IVALUE_DEFAULT, array[i].iValue);
            EXPECT_EQ(NonPodStruct::STR_DEFAULT, array[i].str);
        }

        decltype(NonPodStruct::iValue) count = ZERO;
        for (auto &i : array) {
            EXPECT_EQ(NonPodStruct::IVALUE_DEFAULT, i.iValue);
            EXPECT_EQ(NonPodStruct::STR_DEFAULT, i.str);

            i.iValue = count++;
            i.str = "Item " + std::to_string(i.iValue);
        }

        EXPECT_EQ(static_cast<Array<NonPodStruct>::size_type>(count), array.size());

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(static_cast<int>(i), array[i].iValue);
            EXPECT_EQ(String("Item " + std::to_string(i)), array[i].str);
        }
    }
    EXPECT_EQ(0, NonPodStruct::TotalCount);
}

TEST_F(TestArray, testInitializerList) {
    {
        const int native_array[] = {0, 1, 2, 3};
        const Array<int> array = {0, 1, 2, 3};

        EXPECT_EQ(nativeArrayLength(native_array), array.size());

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(native_array[i], array[i]);
        }
    }

    {
        const String native_array[] = {"Abc", "", "dfe", "_xyz3"};
        const Array<String> array = {"Abc", "", "dfe", "_xyz3"};

        EXPECT_EQ(nativeArrayLength(native_array), array.size());

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(native_array[i], array[i]);
        }
    }

    EXPECT_EQ(0, NonPodStruct::TotalCount);
    {
        const NonPodStruct native_array[] = {
                NonPodStruct(0, "yyyz"),
                NonPodStruct(),
                NonPodStruct(-321, "yyx"),
                NonPodStruct(990, "x^hhf")
        };
        EXPECT_EQ(nativeArrayLength(native_array),
                                static_cast<Array<NonPodStruct>::size_type>(NonPodStruct::TotalCount));

        const Array<NonPodStruct> array = {
                NonPodStruct(0, "yyyz"),
                NonPodStruct(),
                NonPodStruct(-321, "yyx"),
                NonPodStruct(990, "x^hhf")
        };

        EXPECT_EQ(nativeArrayLength(native_array), array.size());
        EXPECT_EQ(nativeArrayLength(native_array) + array.size(),
                                static_cast<Array<NonPodStruct>::size_type>(NonPodStruct::TotalCount));

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(native_array[i].iValue, array[i].iValue);
            EXPECT_EQ(native_array[i].str, array[i].str);
        }
    }
    EXPECT_EQ(0, NonPodStruct::TotalCount);

}

TEST_F(TestArray, testFromNativeConvertion) {

    {
        const int native_array[] = {0, 1, 2, 3};
        const Array<int> array(nativeArrayLength(native_array), native_array);

        EXPECT_EQ(nativeArrayLength(native_array), array.size());

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(native_array[i], array[i]);
        }
    }

    {
        const String native_array[] = {"Abc", "", "dfe", "_xyz3"};
        const Array<String> array(nativeArrayLength(native_array), native_array);

        EXPECT_EQ(nativeArrayLength(native_array), array.size());

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(native_array[i], array[i]);
        }
    }

    {
        const NonPodStruct native_array[] = {
                NonPodStruct(0, "yyyz"),
                NonPodStruct(),
                NonPodStruct(-321, "yyx"),
                NonPodStruct(990, "x^hhf")
        };
        const Array<NonPodStruct> array(nativeArrayLength(native_array), native_array);

        EXPECT_EQ(nativeArrayLength(native_array), array.size());

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(native_array[i].iValue, array[i].iValue);
            EXPECT_EQ(native_array[i].str, array[i].str);
        }
    }
}

template <typename T>
Array<T> moveArray(std::initializer_list<T> list) {
    return {list};
}

TEST_F(TestArray, testMoveAssignment) {
    {
        // Test on integral types
        Array<int> array(0);

        EXPECT_TRUE(array.empty());
        EXPECT_EQ(ZERO, array.size());

        array = moveArray<int>({1, 2, 3});
        EXPECT_TRUE(!array.empty());
        const int src1[] = {1, 2, 3};

        EXPECT_EQ(static_cast<Array<int>::size_type>(3), array.size());
        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(src1[i], array[i]);
        }
    }

    {   // Test on strings types
        Array<String> array(0);
        EXPECT_TRUE(array.empty());

        array = moveArray<String>({"tasrd", "", "hhha", "asd"});
        EXPECT_TRUE(!array.empty());

        const String src[] = {"tasrd", "", "hhha", "asd"};
        EXPECT_EQ(static_cast<Array<String>::size_type>(4), array.size());
        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(src[i], array[i]);
        }
    }

    {   // Test on non-pod types
        Array<NonPodStruct> array(0);
        const NonPodStruct src[] = {
                NonPodStruct(0, "yyyz"),
                NonPodStruct(),
                NonPodStruct(-321, "yyx"),
                NonPodStruct(990, "x^hhf")
        };
        EXPECT_TRUE(array.empty());

        array = moveArray<NonPodStruct>({
                                                NonPodStruct(0, "yyyz"),
                                                NonPodStruct(),
                                                NonPodStruct(-321, "yyx"),
                                                NonPodStruct(990, "x^hhf")
                                        });
        EXPECT_TRUE(!array.empty());
        EXPECT_EQ(static_cast<Array<NonPodStruct>::size_type>(4), array.size());

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(src[i].iValue, array[i].iValue);
            EXPECT_EQ(src[i].str, array[i].str);
        }
    }
}

TEST_F(TestArray, testEquals) {
    {
        const Array<int> array = {1, 2, 3};

        const int equal_native_array[] = {1, 2, 3};
        const auto equal_native_array_length = nativeArrayLength(equal_native_array);

        const int nequal_native_array_0[] = {0, 1, 2, 3};
        const auto nequal_native_array_0_length = nativeArrayLength(nequal_native_array_0);

        const int nequal_native_array_1[] = {3, 2, 1};
        const auto nequal_native_array_1_length = nativeArrayLength(nequal_native_array_1);

        const Array<int> array_eq(equal_native_array_length, equal_native_array);
        const Array<int> array_neq_0(nequal_native_array_0_length, nequal_native_array_0);
        const Array<int> array_neq_1(nequal_native_array_1_length, nequal_native_array_1);

        EXPECT_EQ(equal_native_array_length, array.size());
        EXPECT_TRUE(nequal_native_array_0_length != array.size());
        EXPECT_TRUE(nequal_native_array_1_length == array.size());

        EXPECT_TRUE(array.equals({1, 2, 3}));
        EXPECT_TRUE(!array.equals({3, 2, 3}));
        EXPECT_TRUE(!array.equals({1, 2, 3, 4}));

        EXPECT_TRUE(array.equals(array_eq));
        EXPECT_TRUE(!array.equals(array_neq_0));
        EXPECT_TRUE(!array.equals(array_neq_1));

        EXPECT_EQ(true, array == array_eq);
        EXPECT_EQ(false, array != array_eq);

        EXPECT_EQ(false, array == array_neq_0);
        EXPECT_EQ(true, array != array_neq_0);

        EXPECT_EQ(false, array == array_neq_1);
        EXPECT_EQ(true, array != array_neq_1);
    }

    {
        const Array<String> array = {"tasrd", "", "hhha", "asd"};

        const String equal_native_array[] = {"tasrd", "", "hhha", "asd"};
        const auto equal_native_array_length = nativeArrayLength(equal_native_array);

        const String nequal_native_array_0[] = {"tasrd", "", "hhha", "asd", "ugaga"};
        const auto nequal_native_array_0_length = nativeArrayLength(nequal_native_array_0);

        const String nequal_native_array_1[] = {"tasrd", "", "hhha", "basd"};
        const auto nequal_native_array_1_length = nativeArrayLength(nequal_native_array_1);

        const Array<String> array_eq(equal_native_array_length, equal_native_array);
        const Array<String> array_neq_0(nequal_native_array_0_length, nequal_native_array_0);
        const Array<String> array_neq_1(nequal_native_array_1_length, nequal_native_array_1);

        EXPECT_EQ(equal_native_array_length, array.size());
        EXPECT_TRUE(nequal_native_array_0_length != array.size());
        EXPECT_TRUE(nequal_native_array_1_length == array.size());

        EXPECT_TRUE(array.equals({"tasrd", "", "hhha", "asd"}));
        EXPECT_TRUE(!array.equals({"tasrd", "", "hhha", "basd"}));
        EXPECT_TRUE(!array.equals({"ugaga", "tasrd", "", "hhha", "asd"}));

        EXPECT_TRUE(array.equals(array_eq));
        EXPECT_TRUE(!array.equals(array_neq_0));
        EXPECT_TRUE(!array.equals(array_neq_1));

        EXPECT_EQ(true, array == array_eq);
        EXPECT_EQ(false, array != array_eq);

        EXPECT_EQ(false, array == array_neq_0);
        EXPECT_EQ(true, array != array_neq_0);

        EXPECT_EQ(false, array == array_neq_1);
        EXPECT_EQ(true, array != array_neq_1);
    }

    {
        const Array<NonPodStruct> array = {
                NonPodStruct(0, "yyyz"),
                NonPodStruct(),
                NonPodStruct(-321, "yyx"),
                NonPodStruct(990, "x^hhf")
        };

        const NonPodStruct equal_native_array[] = {
                NonPodStruct(0, "yyyz"),
                NonPodStruct(),
                NonPodStruct(-321, "yyx"),
                NonPodStruct(990, "x^hhf")
        };
        const auto equal_native_array_length = nativeArrayLength(equal_native_array);

        const NonPodStruct nequal_native_array_0[] = {
                NonPodStruct(-31, "kek-yyyz"),
                NonPodStruct(81, "ddds"),
                NonPodStruct(-321, "yyx"),
                NonPodStruct(21, "32"),
                NonPodStruct(990, "x^hhf")
        };
        const auto nequal_native_array_0_length = nativeArrayLength(nequal_native_array_0);

        const NonPodStruct nequal_native_array_1[] = {
                NonPodStruct(-31, "kek-yyyz"),
                NonPodStruct(-1, "ddds"),
                NonPodStruct(0, "dhf")
        };

        const auto nequal_native_array_1_length = nativeArrayLength(nequal_native_array_1);

        const Array<NonPodStruct> array_eq(equal_native_array_length, equal_native_array);
        const Array<NonPodStruct> array_neq_0(nequal_native_array_0_length, nequal_native_array_0);
        const Array<NonPodStruct> array_neq_1(nequal_native_array_1_length, nequal_native_array_1);

        EXPECT_EQ(equal_native_array_length, array.size());
        EXPECT_TRUE(nequal_native_array_0_length != array.size());
        EXPECT_TRUE(nequal_native_array_1_length != array.size());

        EXPECT_TRUE(array.equals({
                                            NonPodStruct(0, "yyyz"),
                                            NonPodStruct(),
                                            NonPodStruct(-321, "yyx"),
                                            NonPodStruct(990, "x^hhf")
                                    }));

        EXPECT_TRUE(array.equals(array_eq));
        EXPECT_TRUE(!array.equals(array_neq_0));
        EXPECT_TRUE(!array.equals(array_neq_1));

        EXPECT_EQ(true, array == array_eq);
        EXPECT_EQ(false, array != array_eq);

        EXPECT_EQ(false, array == array_neq_0);
        EXPECT_EQ(true, array != array_neq_0);

        EXPECT_EQ(false, array == array_neq_1);
        EXPECT_EQ(true, array != array_neq_1);
    }
}

TEST_F(TestArray, testIndexOf) {
    // TODO(abbyssoul): Implementation
}

TEST_F(TestArray, testContains) {
    // TODO(abbyssoul): Implementation
}

TEST_F(TestArray, testExtend) {
    // TODO(abbyssoul): Implementation
}


const Array<int>::size_type TestArray::ZERO = 0;
const Array<int>::size_type TestArray::TEST_SIZE_0 = 7;
const Array<int>::size_type TestArray::TEST_SIZE_1 = 35;

const int 		TestArray::NonPodStruct::IVALUE_DEFAULT = -123;
const String 	TestArray::NonPodStruct::STR_DEFAULT = "test_value";

int TestArray::NonPodStruct::TotalCount = 0;
