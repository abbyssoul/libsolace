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
 *	@file test/test_arrayView.cpp
 *	@brief		Test set of Solace::ArrayView
 ******************************************************************************/
#include <solace/arrayView.hpp>    // Class being tested.

#include <cppunit/extensions/HelperMacros.h>
#include "mockTypes.hpp"


using namespace Solace;


template <typename T, size_t N>
typename ArrayView<T>::size_type nativeArrayLength(const T (& SOLACE_UNUSED(t))[N]) { return N; }


template <typename T, size_t N, typename F>
T* generateTestArray(T (&carray)[N], F generator) {
    for (size_t i = 0; i < N; ++i) {
        carray[i] = generator(i);
    }

    return carray;
}


template <typename T>
std::ostream& operator<< (std::ostream& ostr, const Solace::ArrayView<T>& a) {
    ostr << '[';

    for (typename Solace::ArrayView<T>::size_type end = a.size(), i = 0; i < end; ++i) {
        ostr << a[i];
        if (i < end - 1) {
            ostr << ',' << ' ';
        }
    }

    ostr << ']';

    return ostr;
}



class TestArrayView :
        public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestArrayView);
        CPPUNIT_TEST(testEmpty);
        CPPUNIT_TEST(testCopyConstruction);
        CPPUNIT_TEST(testCopy);
/*
        CPPUNIT_TEST(testBasics);
        CPPUNIT_TEST(testString);
        CPPUNIT_TEST(testNonPods);
        CPPUNIT_TEST(testInitializerList);
        CPPUNIT_TEST(testFromNativeConvertion);
        CPPUNIT_TEST(testMoveAssignment);
*/
//        CPPUNIT_TEST(testEquals);
        CPPUNIT_TEST(testIndexOf);
        CPPUNIT_TEST(testContains);
        CPPUNIT_TEST(testFill);

        // ForEach methods
        /*
        CPPUNIT_TEST(testForEach_byValue);
        CPPUNIT_TEST(testForEach_byValueConversion);
        CPPUNIT_TEST(testForEach_byConstRef);
        CPPUNIT_TEST(testForEachIndexed);

        CPPUNIT_TEST(testMap);
        */

    CPPUNIT_TEST_SUITE_END();

protected:

    static const ArrayView<int>::size_type ZERO;
    static const ArrayView<int>::size_type TEST_SIZE_0;
    static const ArrayView<int>::size_type TEST_SIZE_1;

    struct NonPodStruct {

        static ArrayView<int>::size_type TotalCount;

        static const int IVALUE_DEFAULT;
        static const char* STR_DEFAULT;

        int iValue;
        const char* str;

        NonPodStruct(int i, const char* inStr) :
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

        DerivedNonPodStruct() :
            NonPodStruct(312, "Derived String"), fValue(3.1415f)
        {
        }

        DerivedNonPodStruct(int x, float f, const char* inStr) :
                NonPodStruct(x, inStr), fValue(f)
        {
        }

        virtual ~DerivedNonPodStruct() = default;

    };

public:

    void setUp() override {
        // TODO(abbyssoul): Debug::BeginMemCheck();
        CPPUNIT_ASSERT_EQUAL(ZERO, NonPodStruct::TotalCount);
        CPPUNIT_ASSERT_EQUAL(0, SometimesConstructable::InstanceCount);
    }

    void tearDown() override {
        // TODO(abbyssoul): Debug::EndMemCheck();
        CPPUNIT_ASSERT_EQUAL(ZERO, NonPodStruct::TotalCount);
        CPPUNIT_ASSERT_EQUAL(0, SometimesConstructable::InstanceCount);
    }

    void testEmpty() {
        {
            const ArrayView<int> empty_array;

            CPPUNIT_ASSERT(empty_array.empty());
            CPPUNIT_ASSERT_EQUAL(ZERO, empty_array.size());
            CPPUNIT_ASSERT(empty_array.begin() == empty_array.end());
        }

        {
            const ArrayView<NonPodStruct> empty_array(nullptr);

            CPPUNIT_ASSERT(empty_array.empty());
            CPPUNIT_ASSERT_EQUAL(ZERO, empty_array.size());
            CPPUNIT_ASSERT(empty_array.begin() == empty_array.end());
        }

        {
            const ArrayView<DerivedNonPodStruct> empty_array;

            CPPUNIT_ASSERT(empty_array.empty());
            CPPUNIT_ASSERT_EQUAL(ZERO, empty_array.size());
            CPPUNIT_ASSERT(empty_array.begin() == empty_array.end());
        }
    }

    void testCopyConstruction() {
        int src[16];
        const ArrayView<int>::size_type srcSize = sizeof(src) / sizeof(int);
        generateTestArray(src, [](size_t i) { return (2*i - 1); });


        ArrayView<int> a2(src);
        CPPUNIT_ASSERT(!a2.empty());
        CPPUNIT_ASSERT_EQUAL(srcSize, a2.size());


        // Create a copy:
        ArrayView<int> a1(a2);

        CPPUNIT_ASSERT(!a1.empty());
        CPPUNIT_ASSERT_EQUAL(a1.size(), a2.size());

        // Check that the data is the same:
        for (ArrayView<int>::size_type i = 0; i < a1.size(); ++i) {
            CPPUNIT_ASSERT_EQUAL(static_cast<int>(2*i - 1), a1[i]);
        }

        // Check that changing values in the original C-array changes ArrayView values:
        auto newGen = [](size_t i) { return static_cast<int>(2*i + 3); };
        generateTestArray(src, newGen);

        CPPUNIT_ASSERT_EQUAL(a1.size(), a2.size());
        // Check that the data is the same:
        for (ArrayView<int>::size_type i = 0; i < a1.size(); ++i) {
            CPPUNIT_ASSERT_EQUAL(newGen(i), a1[i]);
            CPPUNIT_ASSERT_EQUAL(newGen(i), a2[i]);
        }
    }

    void testCopy() {
        {
            int src[16];

            ArrayView<int> a1;
            ArrayView<int> a2(src);

            CPPUNIT_ASSERT(a1.empty());
            CPPUNIT_ASSERT(!a2.empty());

            for (ArrayView<int>::size_type i = 0; i < a2.size(); ++i) {
                a2[i] = static_cast<int>(2*i - 1);
            }

            // Copy arrays
            a1 = a2;

            CPPUNIT_ASSERT(!a1.empty());
            CPPUNIT_ASSERT_EQUAL(a1.size(), a2.size());
            for (ArrayView<int>::size_type i = 0; i < a1.size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(static_cast<int>(2*i - 1), a1[i]);
            }
        }
    }
/*
    void testBasics() {
        ArrayView<uint> array(TEST_SIZE_0);

        CPPUNIT_ASSERT(!array.empty());
        CPPUNIT_ASSERT_EQUAL(TEST_SIZE_0, array.size());

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            CPPUNIT_ASSERT_EQUAL(0U, array[i]);
        }

        uint count = 0;
        for (auto i = array.begin(), end = array.end(); i != end; ++i) {
            CPPUNIT_ASSERT_EQUAL(0U, *i);

            *i = count++;
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<ArrayView<uint>::size_type>(count), array.size());

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            CPPUNIT_ASSERT_EQUAL(static_cast<uint>(i), array[i]);
        }

        // TODO(abbyssoul): Test access pass end!!
    }

    void testString() {
        ArrayView<String> array(TEST_SIZE_0);

        CPPUNIT_ASSERT(!array.empty());
        CPPUNIT_ASSERT_EQUAL(TEST_SIZE_0, array.size());

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            CPPUNIT_ASSERT_EQUAL(String::Empty, array[i]);
        }

        auto count = ZERO;
        for (auto& i : array) {
            CPPUNIT_ASSERT(i.empty());

            i = "Item " + std::to_string(count++);
        }

        CPPUNIT_ASSERT_EQUAL(count, array.size());

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            CPPUNIT_ASSERT_EQUAL(String("Item " + std::to_string(i)), array[i]);
        }
    }

    void testNonPods() {
        CPPUNIT_ASSERT_EQUAL(ZERO, NonPodStruct::TotalCount);
        {
            ArrayView<NonPodStruct> array(TEST_SIZE_1);

            CPPUNIT_ASSERT_EQUAL(TEST_SIZE_1, array.size());
            CPPUNIT_ASSERT_EQUAL(NonPodStruct::TotalCount, array.size());

            for (auto i = ZERO, end = array.size(); i < end; ++i) {
                CPPUNIT_ASSERT_EQUAL(NonPodStruct::IVALUE_DEFAULT, array[i].iValue);
                CPPUNIT_ASSERT_EQUAL(NonPodStruct::STR_DEFAULT, array[i].str);
            }

            decltype(NonPodStruct::iValue) count = ZERO;
            for (auto &i : array) {
                CPPUNIT_ASSERT_EQUAL(NonPodStruct::IVALUE_DEFAULT, i.iValue);
                CPPUNIT_ASSERT_EQUAL(NonPodStruct::STR_DEFAULT, i.str);

                i.iValue = count++;
                i.str = "Item " + std::to_string(i.iValue);
            }

            CPPUNIT_ASSERT_EQUAL(static_cast<ArrayView<NonPodStruct>::size_type>(count), array.size());

            for (auto i = ZERO, end = array.size(); i < end; ++i) {
                CPPUNIT_ASSERT_EQUAL(static_cast<int>(i), array[i].iValue);
                CPPUNIT_ASSERT_EQUAL(String("Item " + std::to_string(i)), array[i].str);
            }
        }
        CPPUNIT_ASSERT_EQUAL(ZERO, NonPodStruct::TotalCount);
    }

    void testInitializerList() {
        {
            const int native_array[] = {0, 1, 2, 3};
            const ArrayView<int> array = {0, 1, 2, 3};

            CPPUNIT_ASSERT_EQUAL(nativeArrayLength(native_array), array.size());

            for (auto i = ZERO, end = array.size(); i < end; ++i) {
                CPPUNIT_ASSERT_EQUAL(native_array[i], array[i]);
            }
        }

        {
            const String native_array[] = {"Abc", "", "dfe", "_xyz3"};
            const ArrayView<String> array = {"Abc", "", "dfe", "_xyz3"};

            CPPUNIT_ASSERT_EQUAL(nativeArrayLength(native_array), array.size());

            for (auto i = ZERO, end = array.size(); i < end; ++i) {
                CPPUNIT_ASSERT_EQUAL(native_array[i], array[i]);
            }
        }

        CPPUNIT_ASSERT_EQUAL(ZERO, NonPodStruct::TotalCount);
        {
            const NonPodStruct native_array[] = {
                    NonPodStruct(0, "yyyz"),
                    NonPodStruct(),
                    NonPodStruct(-321, "yyx"),
                    NonPodStruct(990, "x^hhf")
            };
            CPPUNIT_ASSERT_EQUAL(nativeArrayLength(native_array), NonPodStruct::TotalCount);

            const ArrayView<NonPodStruct> array = {
                    NonPodStruct(0, "yyyz"),
                    NonPodStruct(),
                    NonPodStruct(-321, "yyx"),
                    NonPodStruct(990, "x^hhf")
            };

            CPPUNIT_ASSERT_EQUAL(nativeArrayLength(native_array), array.size());
            CPPUNIT_ASSERT_EQUAL(nativeArrayLength(native_array) + array.size(), NonPodStruct::TotalCount);

            for (auto i = ZERO, end = array.size(); i < end; ++i) {
                CPPUNIT_ASSERT_EQUAL(native_array[i].iValue, array[i].iValue);
                CPPUNIT_ASSERT_EQUAL(native_array[i].str, array[i].str);
            }
        }
        CPPUNIT_ASSERT_EQUAL(ZERO, NonPodStruct::TotalCount);

    }

    void testFromNativeConvertion() {

        {
            const int native_array[] = {0, 1, 2, 3};
            const ArrayView<int> array(nativeArrayLength(native_array), native_array);

            CPPUNIT_ASSERT_EQUAL(nativeArrayLength(native_array), array.size());

            for (auto i = ZERO, end = array.size(); i < end; ++i) {
                CPPUNIT_ASSERT_EQUAL(native_array[i], array[i]);
            }
        }

        {
            const String native_array[] = {"Abc", "", "dfe", "_xyz3"};
            const ArrayView<String> array(nativeArrayLength(native_array), native_array);

            CPPUNIT_ASSERT_EQUAL(nativeArrayLength(native_array), array.size());

            for (auto i = ZERO, end = array.size(); i < end; ++i) {
                CPPUNIT_ASSERT_EQUAL(native_array[i], array[i]);
            }
        }

        {
            const NonPodStruct native_array[] = {
                    NonPodStruct(0, "yyyz"),
                    NonPodStruct(),
                    NonPodStruct(-321, "yyx"),
                    NonPodStruct(990, "x^hhf")
            };
            const ArrayView<NonPodStruct> array(nativeArrayLength(native_array), native_array);

            CPPUNIT_ASSERT_EQUAL(nativeArrayLength(native_array), array.size());

            for (auto i = ZERO, end = array.size(); i < end; ++i) {
                CPPUNIT_ASSERT_EQUAL(native_array[i].iValue, array[i].iValue);
                CPPUNIT_ASSERT_EQUAL(native_array[i].str, array[i].str);
            }
        }
    }

    template <typename T>
    ArrayView<T> moveArray(std::initializer_list<T> list) {
        return {list};
    }

    void testMoveAssignment() {
        {// Test on integral types
            ArrayView<int> array(0);

            CPPUNIT_ASSERT(array.empty());
            CPPUNIT_ASSERT_EQUAL(ZERO, array.size());

            array = moveArray<int>({1, 2, 3});
            CPPUNIT_ASSERT(!array.empty());
            const int src1[] = {1, 2, 3};

            CPPUNIT_ASSERT_EQUAL(static_cast<ArrayView<int>::size_type>(3), array.size());
            for (auto i = ZERO, end = array.size(); i < end; ++i) {
                CPPUNIT_ASSERT_EQUAL(src1[i], array[i]);
            }
        }

        {   // Test on strings types
            ArrayView<String> array(0);
            CPPUNIT_ASSERT(array.empty());

            array = moveArray<String>({"tasrd", "", "hhha", "asd"});
            CPPUNIT_ASSERT(!array.empty());

            const String src[] = {"tasrd", "", "hhha", "asd"};
            CPPUNIT_ASSERT_EQUAL(static_cast<ArrayView<String>::size_type>(4), array.size());
            for (auto i = ZERO, end = array.size(); i < end; ++i) {
                CPPUNIT_ASSERT_EQUAL(src[i], array[i]);
            }
        }

        {   // Test on non-pod types
            ArrayView<NonPodStruct> array(0);
            const NonPodStruct src[] = {
                    NonPodStruct(0, "yyyz"),
                    NonPodStruct(),
                    NonPodStruct(-321, "yyx"),
                    NonPodStruct(990, "x^hhf")
            };
            CPPUNIT_ASSERT(array.empty());

            array = moveArray<NonPodStruct>({
                                                    NonPodStruct(0, "yyyz"),
                                                    NonPodStruct(),
                                                    NonPodStruct(-321, "yyx"),
                                                    NonPodStruct(990, "x^hhf")
                                            });
            CPPUNIT_ASSERT(!array.empty());
            CPPUNIT_ASSERT_EQUAL(static_cast<ArrayView<NonPodStruct>::size_type>(4), array.size());

            for (auto i = ZERO, end = array.size(); i < end; ++i) {
                CPPUNIT_ASSERT_EQUAL(src[i].iValue, array[i].iValue);
                CPPUNIT_ASSERT_EQUAL(src[i].str, array[i].str);
            }
        }
    }

    void testEquals() {
        {
            const ArrayView<int> array = {1, 2, 3};

            const int equal_native_array[] = {1, 2, 3};
            const auto equal_native_array_length = nativeArrayLength(equal_native_array);

            const int nequal_native_array_0[] = {0, 1, 2, 3};
            const auto nequal_native_array_0_length = nativeArrayLength(nequal_native_array_0);

            const int nequal_native_array_1[] = {3, 2, 1};
            const auto nequal_native_array_1_length = nativeArrayLength(nequal_native_array_1);

            const ArrayView<int> array_eq(equal_native_array_length, equal_native_array);
            const ArrayView<int> array_neq_0(nequal_native_array_0_length, nequal_native_array_0);
            const ArrayView<int> array_neq_1(nequal_native_array_1_length, nequal_native_array_1);

            CPPUNIT_ASSERT_EQUAL(equal_native_array_length, array.size());
            CPPUNIT_ASSERT(nequal_native_array_0_length != array.size());
            CPPUNIT_ASSERT(nequal_native_array_1_length == array.size());

            CPPUNIT_ASSERT(array.equals({1, 2, 3}));
            CPPUNIT_ASSERT(!array.equals({3, 2, 3}));
            CPPUNIT_ASSERT(!array.equals({1, 2, 3, 4}));

            CPPUNIT_ASSERT(array.equals(array_eq));
            CPPUNIT_ASSERT(!array.equals(array_neq_0));
            CPPUNIT_ASSERT(!array.equals(array_neq_1));

            CPPUNIT_ASSERT_EQUAL(true, array == array_eq);
            CPPUNIT_ASSERT_EQUAL(false, array != array_eq);

            CPPUNIT_ASSERT_EQUAL(false, array == array_neq_0);
            CPPUNIT_ASSERT_EQUAL(true, array != array_neq_0);

            CPPUNIT_ASSERT_EQUAL(false, array == array_neq_1);
            CPPUNIT_ASSERT_EQUAL(true, array != array_neq_1);
        }

        {
            const ArrayView<String> array = {"tasrd", "", "hhha", "asd"};

            const String equal_native_array[] = {"tasrd", "", "hhha", "asd"};
            const auto equal_native_array_length = nativeArrayLength(equal_native_array);

            const String nequal_native_array_0[] = {"tasrd", "", "hhha", "asd", "ugaga"};
            const auto nequal_native_array_0_length = nativeArrayLength(nequal_native_array_0);

            const String nequal_native_array_1[] = {"tasrd", "", "hhha", "basd"};
            const auto nequal_native_array_1_length = nativeArrayLength(nequal_native_array_1);

            const ArrayView<String> array_eq(equal_native_array_length, equal_native_array);
            const ArrayView<String> array_neq_0(nequal_native_array_0_length, nequal_native_array_0);
            const ArrayView<String> array_neq_1(nequal_native_array_1_length, nequal_native_array_1);

            CPPUNIT_ASSERT_EQUAL(equal_native_array_length, array.size());
            CPPUNIT_ASSERT(nequal_native_array_0_length != array.size());
            CPPUNIT_ASSERT(nequal_native_array_1_length == array.size());

            CPPUNIT_ASSERT(array.equals({"tasrd", "", "hhha", "asd"}));
            CPPUNIT_ASSERT(!array.equals({"tasrd", "", "hhha", "basd"}));
            CPPUNIT_ASSERT(!array.equals({"ugaga", "tasrd", "", "hhha", "asd"}));

            CPPUNIT_ASSERT(array.equals(array_eq));
            CPPUNIT_ASSERT(!array.equals(array_neq_0));
            CPPUNIT_ASSERT(!array.equals(array_neq_1));

            CPPUNIT_ASSERT_EQUAL(true, array == array_eq);
            CPPUNIT_ASSERT_EQUAL(false, array != array_eq);

            CPPUNIT_ASSERT_EQUAL(false, array == array_neq_0);
            CPPUNIT_ASSERT_EQUAL(true, array != array_neq_0);

            CPPUNIT_ASSERT_EQUAL(false, array == array_neq_1);
            CPPUNIT_ASSERT_EQUAL(true, array != array_neq_1);
        }

        {
            const ArrayView<NonPodStruct> array = {
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

            const ArrayView<NonPodStruct> array_eq(equal_native_array_length, equal_native_array);
            const ArrayView<NonPodStruct> array_neq_0(nequal_native_array_0_length, nequal_native_array_0);
            const ArrayView<NonPodStruct> array_neq_1(nequal_native_array_1_length, nequal_native_array_1);

            CPPUNIT_ASSERT_EQUAL(equal_native_array_length, array.size());
            CPPUNIT_ASSERT(nequal_native_array_0_length != array.size());
            CPPUNIT_ASSERT(nequal_native_array_1_length != array.size());

            CPPUNIT_ASSERT(array.equals({
                                                NonPodStruct(0, "yyyz"),
                                                NonPodStruct(),
                                                NonPodStruct(-321, "yyx"),
                                                NonPodStruct(990, "x^hhf")
                                        }));

            CPPUNIT_ASSERT(array.equals(array_eq));
            CPPUNIT_ASSERT(!array.equals(array_neq_0));
            CPPUNIT_ASSERT(!array.equals(array_neq_1));

            CPPUNIT_ASSERT_EQUAL(true, array == array_eq);
            CPPUNIT_ASSERT_EQUAL(false, array != array_eq);

            CPPUNIT_ASSERT_EQUAL(false, array == array_neq_0);
            CPPUNIT_ASSERT_EQUAL(true, array != array_neq_0);

            CPPUNIT_ASSERT_EQUAL(false, array == array_neq_1);
            CPPUNIT_ASSERT_EQUAL(true, array != array_neq_1);
        }
    }

    */

    void testIndexOf() {
        int src[16];
        generateTestArray(src, [](size_t i) { return (2*i - 1); });

        auto array = arrayView(src);

        {  // Test for existing value:
            const auto maybeIndex = array.indexOf(2*4 - 1);
            CPPUNIT_ASSERT(maybeIndex.isSome());
            CPPUNIT_ASSERT_EQUAL(ArrayView<int>::size_type(4), maybeIndex.get());
        }


        {  // Test for non-existing value:
            CPPUNIT_ASSERT(array.indexOf(3*4 + 128).isNone());
        }

        {  // Test empty array contains nothing
            CPPUNIT_ASSERT(ArrayView<int>().indexOf(2*3 - 1).isNone());
        }

        {  // Floats are so imprecise
            CPPUNIT_ASSERT(ArrayView<float32>().indexOf(3.1415f).isNone());
        }
    }

    void testContains() {
        int src[24];
        generateTestArray(src, [](size_t i) { return (2*i + 3); });

        {  // Test empty array contains nothing
            CPPUNIT_ASSERT(!ArrayView<int>().contains(2*3 - 1));
        }

        auto array = arrayView(src);

        {  // Test for existing value:
            CPPUNIT_ASSERT(array.contains(2*9 + 3));
        }

        {  // Test for non-existing value:
            CPPUNIT_ASSERT(!array.contains(-41));
        }
    }

    void testFill() {
        {  // Const-value filling:
            int src[24];
            auto array = arrayView(src);

            array.fill(42);

            for (const auto i : array) {
                CPPUNIT_ASSERT_EQUAL(42, i);
            }
        }

        {  // Fill using generator:
            int src[24];
            auto array = arrayView(src);

            array.fill([](size_t i) { return static_cast<int>(i*2 + 1); });

            for (ArrayView<int>::size_type i = 0; i < array.size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(static_cast<int>(i*2 + 1), array[i]);
            }
        }
    }

/*
    void testForEach_byValue() {
        const ArrayView<int> array = {1, 2, 3, 4, 5, 6};

        int acc = 0;
        array.forEach([&acc](int x) {
            acc += x;
        });

        CPPUNIT_ASSERT_EQUAL(21, acc);
    }

    void testForEach_byConstRef() {
        const ArrayView<String> array = {"Hello", " ", "world", "!"};

        String acc;
        array.forEach([&acc](const String& x) {
            acc = acc.concat(x);
        });

        CPPUNIT_ASSERT_EQUAL(String("Hello world!"), acc);
    }

    void testForEach_byValueConversion() {
        const ArrayView<int> array = {1, 2, 3, 4, 5, 6};

        double acc = 0;
        array.forEach([&acc](double x) {
            acc += x;
        });

        CPPUNIT_ASSERT_DOUBLES_EQUAL(21.0, acc, 0.001);
    }

    void testForEachIndexed() {
        const ArrayView<int> array = {1, 2, 3, 4, 5, 6};
        bool allEq = true;

        array.forEachIndexed([&allEq](ArrayView<int>::size_type i, ArrayView<int>::size_type x) {
            allEq &= (i + 1 == x);
        });

        CPPUNIT_ASSERT_EQUAL(true, allEq);
    }


    void testMap() {
        const ArrayView<DerivedNonPodStruct> array = {
                DerivedNonPodStruct(32, 2.4, "hello"),
                DerivedNonPodStruct(-24, 2.4, " "),
                DerivedNonPodStruct(10, 2.4, "world"),
        };

        {
            auto r = array.map([](const DerivedNonPodStruct& content) {
                return content.iValue;
            });

            CPPUNIT_ASSERT_EQUAL(array.size(), r.size());
            for (ArrayView<int>::size_type i = 0; i < array.size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(r[i], array[i].iValue);
            }

        }
        {
            auto r = array.map([](const DerivedNonPodStruct& content) {
                return content.str;
            });

            CPPUNIT_ASSERT_EQUAL(array.size(), r.size());
            for (ArrayView<int>::size_type i = 0; i < array.size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(r[i], array[i].str);
            }

        }
    }

    void testDeallocationWhenElementConstructorThrows() {

        SometimesConstructable::BlowUpEveryInstance = 9;

        CPPUNIT_ASSERT_THROW(const ArrayView<SometimesConstructable> sholdFail(10), Exception);
        CPPUNIT_ASSERT_EQUAL(0, SometimesConstructable::InstanceCount);
    }
    */

};

const ArrayView<int>::size_type TestArrayView::ZERO = 0;
const ArrayView<int>::size_type TestArrayView::TEST_SIZE_0 = 7;
const ArrayView<int>::size_type TestArrayView::TEST_SIZE_1 = 35;

const int 		TestArrayView::NonPodStruct::IVALUE_DEFAULT = -123;

ArrayView<int>::size_type TestArrayView::NonPodStruct::TotalCount = 0;


CPPUNIT_TEST_SUITE_REGISTRATION(TestArrayView);
