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
#include <string>   // Non POD subject.

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
            static const char STR_DEFAULT[];

            int iValue{};
            std::string str;


            virtual ~NonPodStruct() {
                --TotalCount;
            }

			// cppcheck-suppress passedByValue
			NonPodStruct(int i, std::string inStr) noexcept
				: iValue{i}
				, str{mv(inStr)}
			{
				++TotalCount;
            }

            NonPodStruct()
					: iValue{IVALUE_DEFAULT}
					, str{STR_DEFAULT}
			{
				++TotalCount;
            }

            NonPodStruct(NonPodStruct&& other)
				: iValue{other.iValue}
				, str{mv(other.str)}
            {
                ++TotalCount;
            }

            NonPodStruct(NonPodStruct const& other)
				: iValue{other.iValue}
				, str{other.str}
            {
                ++TotalCount;
            }


            NonPodStruct& operator= (const NonPodStruct& rhs) = default;

            bool operator== (const NonPodStruct& other) const {
                return iValue == other.iValue && str == other.str;
            }

        };


        struct DerivedNonPodStruct  : public NonPodStruct {

            float fValue{3.1415f};

            DerivedNonPodStruct()
				: NonPodStruct{312, "Derived String"}
            {
            }

			// cppcheck-suppress passedByValue
			DerivedNonPodStruct(int x, float f, std::string inStr)
				: NonPodStruct{x, mv(inStr)}
				, fValue{f}
            {
            }

	};

public:

    void SetUp() override {
        // TODO(abbyssoul): Debug::BeginMemCheck();
        EXPECT_EQ(0, NonPodStruct::TotalCount);
        EXPECT_EQ(0, SimpleType::InstanceCount);
        EXPECT_EQ(0, SometimesConstructable::InstanceCount);
    }

    void TearDown() override {
        // TODO(abbyssoul): Debug::EndMemCheck();
        EXPECT_EQ(0, NonPodStruct::TotalCount);
        EXPECT_EQ(0, SimpleType::InstanceCount);
        EXPECT_EQ(0, SometimesConstructable::InstanceCount);
    }

};


TEST_F(TestArray, testEmpty) {
        {
            const Array<int> empty_array{};

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
            const Array<NonPodStruct> empty_array;

            EXPECT_TRUE(empty_array.empty());
            EXPECT_EQ(ZERO, empty_array.size());
            EXPECT_TRUE(empty_array.begin() == empty_array.end());
        }

        {
            const Array<DerivedNonPodStruct> empty_array;

            EXPECT_TRUE(empty_array.empty());
            EXPECT_EQ(ZERO, empty_array.size());
            EXPECT_TRUE(empty_array.begin() == empty_array.end());
        }
}

TEST_F(TestArray, testMove) {
    Array<int> a1;
    EXPECT_TRUE(a1.empty());

	auto maybeArrya = makeArray<int>(20);
	ASSERT_TRUE(maybeArrya.isOk());

	auto& a2 = maybeArrya.unwrap();
	EXPECT_FALSE(a2.empty());
    for (Array<int>::size_type i = 0; i < a2.size(); ++i) {
        a2[i] = static_cast<int>(2*i) - 1;
    }

	a1 = maybeArrya.moveResult();
    EXPECT_TRUE(!a1.empty());
    EXPECT_TRUE(a2.empty());

    for (Array<int>::size_type i = 0; i < a1.size(); ++i) {
        EXPECT_EQ(static_cast<int>(2*i) - 1, a1[i]);
    }
}

TEST_F(TestArray, testBasics) {
    std::vector<uint> def(TEST_SIZE_1);
    for (auto const& i : def) {
        EXPECT_EQ(0U, i);
    }

	auto maybeArray = makeArray<uint>(TEST_SIZE_0);
	ASSERT_TRUE(maybeArray.isOk());

	auto& array = maybeArray.unwrap();
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
	auto maybeArray = makeArray<std::string>(TEST_SIZE_0);
	ASSERT_TRUE(maybeArray.isOk());

	auto& array = maybeArray.unwrap();
	EXPECT_TRUE(!array.empty());
    EXPECT_EQ(TEST_SIZE_0, array.size());

    std::string empty;
    for (auto i = ZERO, end = array.size(); i < end; ++i) {
        EXPECT_EQ(empty, array[i]);
    }

    auto count = ZERO;
    for (auto& i : array) {
        EXPECT_TRUE(i.empty());

        i = "Item " + std::to_string(count++);
    }

    EXPECT_EQ(count, array.size());

    for (auto i = ZERO, end = array.size(); i < end; ++i) {
        EXPECT_EQ(std::string("Item ") + std::to_string(i), array[i]);
    }
}

TEST_F(TestArray, testNonPods) {
    EXPECT_EQ(0, NonPodStruct::TotalCount);
    {
		auto maybeArray = makeArray<NonPodStruct>(TEST_SIZE_1);
		ASSERT_TRUE(maybeArray.isOk());

		auto& array = maybeArray.unwrap();

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
            EXPECT_EQ(std::string("Item ") + std::to_string(i), array[i].str);
        }
    }
    EXPECT_EQ(0, NonPodStruct::TotalCount);
}

TEST_F(TestArray, testInitializerList) {
    {
        const int native_array[] = {0, 1, 2, 3};
		auto const maybeArray = makeArrayOf<int>(0, 1, 2, 3);
		ASSERT_TRUE(maybeArray.isOk());

		auto& array = maybeArray.unwrap();
        EXPECT_EQ(nativeArrayLength(native_array), array.size());

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(native_array[i], array[i]);
        }
    }

    {
        const StringLiteral native_array[] = {"Abc", "", "dfe", "_xyz3"};
		auto const maybeArray = makeArrayOf<StringView>("Abc", "", "dfe", "_xyz3");
		ASSERT_TRUE(maybeArray.isOk());

		auto& array = maybeArray.unwrap();
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

		auto const maybeArray = makeArrayOf<NonPodStruct> (
                NonPodStruct(0, "yyyz"),
                NonPodStruct(),
                NonPodStruct(-321, "yyx"),
                NonPodStruct(990, "x^hhf"));
		ASSERT_TRUE(maybeArray.isOk());

		auto& array = maybeArray.unwrap();

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
		auto const maybeArray = makeArray<int>(nativeArrayLength(native_array), native_array);
		ASSERT_TRUE(maybeArray.isOk());

		auto& array = maybeArray.unwrap();
        EXPECT_EQ(nativeArrayLength(native_array), array.size());

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(native_array[i], array[i]);
        }
    }

    {
        const std::string native_array[] = {"Abc", "", "dfe", "_xyz3"};
		auto const maybeArray = makeArray<std::string>(nativeArrayLength(native_array), native_array);
		ASSERT_TRUE(maybeArray.isOk());

		auto& array = maybeArray.unwrap();
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
		auto const maybeArray = makeArray<NonPodStruct>(nativeArrayLength(native_array), native_array);
		ASSERT_TRUE(maybeArray.isOk());

		auto& array = maybeArray.unwrap();
        EXPECT_EQ(nativeArrayLength(native_array), array.size());

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(native_array[i].iValue, array[i].iValue);
            EXPECT_EQ(native_array[i].str, array[i].str);
        }
    }
}


TEST_F(TestArray, testMoveAssignment) {
    {
        // Test on integral types
		Array<int> array{};

        EXPECT_TRUE(array.empty());
        EXPECT_EQ(ZERO, array.size());

		auto maybeArray = makeArrayOf<int>(1, 2, 3);
		ASSERT_TRUE(maybeArray.isOk());

		array = maybeArray.moveResult();
        EXPECT_TRUE(!array.empty());
        const int src1[] = {1, 2, 3};

		EXPECT_EQ(3U, array.size());
        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(src1[i], array[i]);
        }
    }

    {   // Test on strings types
		Array<std::string> array{};
        EXPECT_TRUE(array.empty());

		auto maybeArray = makeArrayOf<std::string>("tasrd", "", "hhha", "asd");
		ASSERT_TRUE(maybeArray.isOk());

		array = maybeArray.moveResult();
        EXPECT_TRUE(!array.empty());

        std::string const src[] = {"tasrd", "", "hhha", "asd"};
		EXPECT_EQ(4U, array.size());
        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(src[i], array[i]);
        }
    }

    {   // Test on non-pod types
		Array<NonPodStruct> array{};
        const NonPodStruct src[] = {
                NonPodStruct(0, "yyyz"),
                NonPodStruct(),
                NonPodStruct(-321, "yyx"),
                NonPodStruct(990, "x^hhf")
        };
        EXPECT_TRUE(array.empty());

		auto maybeArray = makeArrayOf<NonPodStruct>(
					NonPodStruct(0, "yyyz"),
					NonPodStruct(),
					NonPodStruct(-321, "yyx"),
					NonPodStruct(990, "x^hhf"));
		ASSERT_TRUE(maybeArray.isOk());

		array = maybeArray.moveResult();
        EXPECT_TRUE(!array.empty());
		EXPECT_EQ(4U, array.size());

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(src[i].iValue, array[i].iValue);
            EXPECT_EQ(src[i].str, array[i].str);
        }
    }
}

TEST_F(TestArray, testEquals) {
    {
		auto const maybeArray = makeArrayOf<int>(1, 2, 3);
		ASSERT_TRUE(maybeArray.isOk());

		auto& array = maybeArray.unwrap();

        const int equal_native_array[] = {1, 2, 3};
        auto const equal_native_array_length = nativeArrayLength(equal_native_array);

        const int nequal_native_array_0[] = {0, 1, 2, 3};
        auto const nequal_native_array_0_length = nativeArrayLength(nequal_native_array_0);

        const int nequal_native_array_1[] = {3, 2, 1};
        auto const nequal_native_array_1_length = nativeArrayLength(nequal_native_array_1);

        auto const array_eq = makeArray<int>(equal_native_array_length, equal_native_array);
        auto const array_neq_0 = makeArray<int>(nequal_native_array_0_length, nequal_native_array_0);
        auto const array_neq_1 = makeArray<int>(nequal_native_array_1_length, nequal_native_array_1);

		ASSERT_TRUE(array_eq.isOk());
		ASSERT_TRUE(array_neq_0.isOk());
		ASSERT_TRUE(array_neq_1.isOk());


        EXPECT_EQ(equal_native_array_length, array.size());
        EXPECT_TRUE(nequal_native_array_0_length != array.size());
        EXPECT_TRUE(nequal_native_array_1_length == array.size());

        EXPECT_TRUE(array.equals({1, 2, 3}));
        EXPECT_TRUE(!array.equals({3, 2, 3}));
        EXPECT_TRUE(!array.equals({1, 2, 3, 4}));

		EXPECT_TRUE(array.equals(*array_eq));
		EXPECT_TRUE(!array.equals(*array_neq_0));
		EXPECT_TRUE(!array.equals(*array_neq_1));

		EXPECT_EQ(true, array == *array_eq);
		EXPECT_EQ(false, array != *array_eq);

		EXPECT_EQ(false, array == *array_neq_0);
		EXPECT_EQ(true, array != *array_neq_0);

		EXPECT_EQ(false, array == *array_neq_1);
		EXPECT_EQ(true, array != *array_neq_1);
    }

    {
		auto const maybeArray = makeArrayOf<std::string>("tasrd", "", "hhha", "asd");
		ASSERT_TRUE(maybeArray.isOk());

		auto& array = maybeArray.unwrap();

        const std::string equal_native_array[] = {"tasrd", "", "hhha", "asd"};
        auto const equal_native_array_length = nativeArrayLength(equal_native_array);

        const std::string nequal_native_array_0[] = {"tasrd", "", "hhha", "asd", "ugaga"};
        auto const nequal_native_array_0_length = nativeArrayLength(nequal_native_array_0);

        const std::string nequal_native_array_1[] = {"tasrd", "", "hhha", "basd"};
        auto const nequal_native_array_1_length = nativeArrayLength(nequal_native_array_1);

		auto const marray_eq = makeArray<std::string>(equal_native_array_length, equal_native_array);
		auto const marray_neq_0 = makeArray<std::string>(nequal_native_array_0_length, nequal_native_array_0);
		auto const marray_neq_1 = makeArray<std::string>(nequal_native_array_1_length, nequal_native_array_1);
		ASSERT_TRUE(marray_eq.isOk());
		ASSERT_TRUE(marray_neq_0.isOk());
		ASSERT_TRUE(marray_neq_1.isOk());

		auto& array_eq = marray_eq.unwrap();
		auto& array_neq_0 = marray_neq_0.unwrap();
		auto& array_neq_1 = marray_neq_0.unwrap();

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
		auto const maybeArray = makeArrayOf<NonPodStruct>(
                NonPodStruct(0, "yyyz"),
                NonPodStruct(),
                NonPodStruct(-321, "yyx"),
                NonPodStruct(990, "x^hhf"));
		ASSERT_TRUE(maybeArray.isOk());

        const NonPodStruct equal_native_array[] = {
                NonPodStruct(0, "yyyz"),
                NonPodStruct(),
                NonPodStruct(-321, "yyx"),
                NonPodStruct(990, "x^hhf")
        };
        auto const equal_native_array_length = nativeArrayLength(equal_native_array);

        const NonPodStruct nequal_native_array_0[] = {
                NonPodStruct(-31, "kek-yyyz"),
                NonPodStruct(81, "ddds"),
                NonPodStruct(-321, "yyx"),
                NonPodStruct(21, "32"),
                NonPodStruct(990, "x^hhf")
        };
        auto const nequal_native_array_0_length = nativeArrayLength(nequal_native_array_0);

        const NonPodStruct nequal_native_array_1[] = {
                NonPodStruct(-31, "kek-yyyz"),
                NonPodStruct(-1, "ddds"),
                NonPodStruct(0, "dhf")
        };

        auto const nequal_native_array_1_length = nativeArrayLength(nequal_native_array_1);

		auto const marray_eq = makeArray<NonPodStruct>(equal_native_array_length, equal_native_array);
		auto const marray_neq_0 = makeArray<NonPodStruct>(nequal_native_array_0_length, nequal_native_array_0);
		auto const marray_neq_1 = makeArray<NonPodStruct>(nequal_native_array_1_length, nequal_native_array_1);
		ASSERT_TRUE(marray_eq.isOk());
		ASSERT_TRUE(marray_neq_0.isOk());
		ASSERT_TRUE(marray_neq_1.isOk());
		auto& array_eq = marray_eq.unwrap();
		auto& array_neq_0 = marray_neq_0.unwrap();
		auto& array_neq_1 = marray_neq_0.unwrap();

		auto& array = maybeArray.unwrap();
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
	auto const maybeArray = makeArrayOf<int>(1, 2, 3);
	ASSERT_TRUE(maybeArray.isOk());

	auto& array = maybeArray.unwrap();

	auto maybeIndex1 = array.indexOf(2);
	EXPECT_TRUE(maybeIndex1.isSome());
	EXPECT_EQ(1U, maybeIndex1.get());

	EXPECT_TRUE(array.indexOf(42).isNone());
}

TEST_F(TestArray, testContains) {
	auto const maybeArray = makeArrayOf<int>(1, 2, 3);
	ASSERT_TRUE(maybeArray.isOk());

	auto& array = maybeArray.unwrap();
	EXPECT_TRUE(array.contains(1));
	EXPECT_TRUE(array.contains(2));
	EXPECT_TRUE(array.contains(3));

	EXPECT_FALSE(array.contains(42));
	EXPECT_FALSE(array.contains(4));
}



/*
TEST_F(TestArray, testFill) {
    {
        Array<int> array(5);
        const int equal_array[] = {1, 2, 3};
        const size_t equal_length = nativeArrayLength(equal_array);
        const int nequal_array[] = {0, 1, 2, 3};
        const size_t nequal_length = nativeArrayLength(nequal_array);
        array.fill(equal_array, equal_length, 1);
        EXPECT_EQ(true, array.equals({1, 1, 2, 3, array[4]}));
        array.fill(nequal_array, nequal_length, 1);
        EXPECT_EQ(true, array.equals({1, 0, 1, 2, 3}));
        EXPECT_THROW(array.fill(nequal_array, nequal_length, 1),
                std::overflow_error);
        EXPECT_THROW(array.fill(nequal_array, 1, array.size() + 1),
                std::range_error);
        EXPECT_THROW(array.fill({5, 4, 3, 2, 1, 0}),
                std::overflow_error);
        array.fill({4, 3, 2, 1, 0});
        EXPECT_EQ(true, array.equals({4, 3, 2, 1, 0}));
    }
}
*/

TEST_F(TestArray, testForEach_byValue) {
	auto const maybeArray = makeArrayOf<int>(1, 2, 3, 4, 5, 6);
	ASSERT_TRUE(maybeArray.isOk());

	auto& array = maybeArray.unwrap();

    int acc = 0;
    array.forEach([&acc](int x) {
        acc += x;
    });


    EXPECT_EQ(21, acc);
}


TEST_F(TestArray, testForEach_byMutRef) {
	auto maybeArray = makeArrayOf<std::string>("Hello", " ", "world", "!");
	ASSERT_TRUE(maybeArray.isOk());
	auto& array = maybeArray.unwrap();

	std::string acc;
	array.forEach([&acc](std::string& x) {
		acc += x;
	});

	EXPECT_EQ(std::string("Hello world!"), acc);
}


TEST_F(TestArray, testForEach_byConstRef) {
	auto const maybeArray = makeArrayOf<std::string>("Hello", " ", "world", "!");
	ASSERT_TRUE(maybeArray.isOk());

	auto& array = maybeArray.unwrap();

    std::string acc;
    array.forEach([&acc](const std::string& x) {
        acc += x;
    });

    EXPECT_EQ(std::string("Hello world!"), acc);
}

TEST_F(TestArray, testForEach_byValueConversion) {
	auto const maybeArray = makeArrayOf<int>(1, 2, 3, 4, 5, 6);
	ASSERT_TRUE(maybeArray.isOk());

	auto& array = maybeArray.unwrap();

    double acc = 0;
    array.forEach([&acc](double x) {
        acc += x;
    });

    EXPECT_FLOAT_EQ(21.0, acc);
}

TEST_F(TestArray, testForEachIndexed) {
	auto const maybeArray = makeArrayOf<int>(1, 2, 3, 4, 5, 6);
	ASSERT_TRUE(maybeArray.isOk());

	auto& array = maybeArray.unwrap();
	bool allEq = true;

    array.forEach([&allEq](Array<int>::size_type i, Array<int>::size_type x) {
        allEq &= (i + 1 == x);
    });

    EXPECT_EQ(true, allEq);
}

/*
TEST_F(TestArray, testMap) {
    auto const array = allocArray<DerivedNonPodStruct>({
            DerivedNonPodStruct(32, 2.4, "hello"),
            DerivedNonPodStruct(-24, 2.4, " "),
            DerivedNonPodStruct(10, 2.4, "world"),
    });

    {
        auto r = array.map([](const DerivedNonPodStruct& content) {
            return content.iValue;
        });

        EXPECT_EQ(array.size(), r.size());
        for (Array<int>::size_type i = 0; i < array.size(); ++i) {
            EXPECT_EQ(r[i], array[i].iValue);
        }

    }
    {
        auto r = array.map([](const DerivedNonPodStruct& content) {
            return content.str;
        });

        EXPECT_EQ(array.size(), r.size());
        for (Array<int>::size_type i = 0; i < array.size(); ++i) {
            EXPECT_EQ(r[i], array[i].str);
        }

    }
}
*/

TEST_F(TestArray, testDeallocationWhenElementConstructorThrows) {
    SometimesConstructable::BlowUpEveryInstance = 9;

    EXPECT_ANY_THROW(auto unsuedArray = makeArray<SometimesConstructable>(10));

    EXPECT_EQ(0, SometimesConstructable::InstanceCount);
}


TEST_F(TestArray, testSet) {
	auto maybeArray = makeArrayOf<int>(1, 2, 3, 4, 5, 6);
	ASSERT_TRUE(maybeArray.isOk());

	auto& array = maybeArray.unwrap();

    int acc = 6;

    auto f = [&acc]() { return acc--; };
    array.set(0, f)
            .set(4, f)
            .set(5, f);

    EXPECT_EQ(3, acc);
    EXPECT_EQ(6, array[0]);
    EXPECT_EQ(5, array[4]);
    EXPECT_EQ(4, array[5]);
}

TEST_F(TestArray, settingOutOfBoundsThrows) {
	auto maybeArray = makeArray<int>(5);
	ASSERT_TRUE(maybeArray.isOk());

	EXPECT_ANY_THROW(maybeArray.unwrap().set(16, []() { return 321; }));
}

const Array<int>::size_type TestArray::ZERO = 0;
const Array<int>::size_type TestArray::TEST_SIZE_0 = 7;
const Array<int>::size_type TestArray::TEST_SIZE_1 = 35;

const int   TestArray::NonPodStruct::IVALUE_DEFAULT = -123;
const char  TestArray::NonPodStruct::STR_DEFAULT[] = "test_value";

int TestArray::NonPodStruct::TotalCount = 0;
