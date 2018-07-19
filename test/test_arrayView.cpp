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

int fillOdd(size_t i) {
    return static_cast<int>(2*i) - 1;
}

int fillEven(size_t i) {
    static constexpr int bias = 2928;

    return 2*static_cast<int>(i) + bias;
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
        CPPUNIT_TEST(testEquals_EmptyArray);
        CPPUNIT_TEST(testEquals_IntegralType);
        CPPUNIT_TEST(testEquals_NonPodType);


        CPPUNIT_TEST(testIndexOf);
        CPPUNIT_TEST(testContains);
        CPPUNIT_TEST(testFillWithConstValue);
        CPPUNIT_TEST(testFillWithGenerator);
        CPPUNIT_TEST(testFillWithConstExplosiveValue);
        CPPUNIT_TEST(testFillWithGeneratorOfExplosiveValue);


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
        std::string str;


        virtual ~NonPodStruct() {
            --TotalCount;
        }

        NonPodStruct(int i, const char* inStr) :
            iValue(i), str(inStr)
        {
            ++TotalCount;
        }

        NonPodStruct(int i, std::string&& inStr) :
            iValue(i), str(std::move(inStr))
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

        NonPodStruct& operator= (const NonPodStruct& rhs) {
            iValue = rhs.iValue;
            str = rhs.str;

            return (*this);
        }

        NonPodStruct& operator= (NonPodStruct&& rhs) {
            iValue = std::move(rhs.iValue);
            str = std::move(rhs.str);

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

        DerivedNonPodStruct(int x, float f, const char* inStr) :
                NonPodStruct(x, inStr), fValue(f)
        {
        }

    };

    static NonPodStruct fillOddNonPods(size_t i) {
        const auto index = static_cast<int>(i)*2 - 1;
        auto str = std::string("Some Odd string: ") + std::to_string(index);

        return NonPodStruct(index, std::move(str));
    }

    static NonPodStruct fillEvenNonPods(size_t i) {
        const auto index = static_cast<int>(i)*2 + 1;
        auto str = std::string("Some Event string: ") + std::to_string(index);

        return NonPodStruct(index, std::move(str));
    }


    template<typename T>
    class NonPodGuard {
    public:
        ~NonPodGuard() {
            for (size_t i = 0; i < _size; ++i) {
                reinterpret_cast<T*>(_mem + i * sizeof(T))->~T();
            }
        }

        NonPodGuard(byte* mem, size_t size) :
            _mem(mem),
            _size(size)
        {
            for (size_t i = 0; i < _size; ++i) {
                new ((_mem + i * sizeof(T))) T();
            }
        }

    private:
        byte*   _mem;
        size_t  _size;
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
            CPPUNIT_ASSERT(empty_array == nullptr);
        }

        {
            const ArrayView<NonPodStruct> empty_array(nullptr);

            CPPUNIT_ASSERT(empty_array.empty());
            CPPUNIT_ASSERT_EQUAL(ZERO, empty_array.size());
            CPPUNIT_ASSERT(empty_array.begin() == empty_array.end());
            CPPUNIT_ASSERT(empty_array == nullptr);
        }

        {
            const ArrayView<DerivedNonPodStruct> empty_array;

            CPPUNIT_ASSERT(empty_array.empty());
            CPPUNIT_ASSERT_EQUAL(ZERO, empty_array.size());
            CPPUNIT_ASSERT(empty_array.begin() == empty_array.end());
            CPPUNIT_ASSERT(empty_array == nullptr);
        }
    }


    void testCopyConstruction() {
        int src[16];
        const ArrayView<int>::size_type srcSize = sizeof(src) / sizeof(int);
        generateTestArray(src, fillOdd);


        ArrayView<int> a2(src);
        CPPUNIT_ASSERT(!a2.empty());
        CPPUNIT_ASSERT_EQUAL(srcSize, a2.size());


        // Create a copy:
        ArrayView<int> a1(a2);

        CPPUNIT_ASSERT(!a1.empty());
        CPPUNIT_ASSERT_EQUAL(a1.size(), a2.size());

        // Check that the data is the same:
        for (ArrayView<int>::size_type i = 0; i < a1.size(); ++i) {
            CPPUNIT_ASSERT_EQUAL(static_cast<int>(2*i) - 1, a1[i]);
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
            generateTestArray(src, fillOdd);

            ArrayView<int> a1;
            ArrayView<int> a2(src);

            CPPUNIT_ASSERT(a1.empty());
            CPPUNIT_ASSERT(!a2.empty());

            // Copy arrays
            a1 = a2;

            CPPUNIT_ASSERT(!a1.empty());
            CPPUNIT_ASSERT(!a2.empty());
            CPPUNIT_ASSERT_EQUAL(a1.size(), a2.size());

            for (ArrayView<int>::size_type i = 0; i < a1.size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(fillOdd(i), a1[i]);
            }

            // Make sure that if underlaying memory changed - this is reflected in arrayView:
            generateTestArray(src, fillEven);
            for (ArrayView<int>::size_type i = 0; i < a1.size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(fillEven(i), a1[i]);
                CPPUNIT_ASSERT_EQUAL(fillEven(i), a2[i]);
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
*/

    void testEquals_EmptyArray() {
        ArrayView<int> emptyArray;

        CPPUNIT_ASSERT(emptyArray == nullptr);
        CPPUNIT_ASSERT(!(emptyArray != nullptr));
        CPPUNIT_ASSERT(emptyArray.equals(emptyArray));

        {  // Check that array views of the same memory are equal:
            ArrayView<int> differenEmptyArray;

            CPPUNIT_ASSERT(emptyArray.equals(differenEmptyArray));
            CPPUNIT_ASSERT(differenEmptyArray.equals(emptyArray));
            CPPUNIT_ASSERT(emptyArray == differenEmptyArray);
            CPPUNIT_ASSERT(differenEmptyArray == emptyArray);
            CPPUNIT_ASSERT(!(emptyArray != differenEmptyArray));
            CPPUNIT_ASSERT(!(differenEmptyArray != emptyArray));
        }
    }

    void testEquals_IntegralType() {
        int src[32];
        generateTestArray(src, fillOdd);

        auto array = arrayView(src);

        // Make sure it is not equals empty array if it is not empty
        CPPUNIT_ASSERT(!array.equals(ArrayView<int>()));
        CPPUNIT_ASSERT(!(array == ArrayView<int>()));
        CPPUNIT_ASSERT(!(array == nullptr));
        CPPUNIT_ASSERT(array != ArrayView<int>());
        CPPUNIT_ASSERT(array != nullptr);

        // Self equality is important
        CPPUNIT_ASSERT(array.equals(array));

        {  // Check that array views of the same memory are equal:
            ArrayView<int> arraySharingMemory(src);

            CPPUNIT_ASSERT(array.equals(arraySharingMemory));
            CPPUNIT_ASSERT(arraySharingMemory.equals(array));
            CPPUNIT_ASSERT(array == arraySharingMemory);
            CPPUNIT_ASSERT(arraySharingMemory == array);
            CPPUNIT_ASSERT(!(array != arraySharingMemory));
            CPPUNIT_ASSERT(!(arraySharingMemory != array));
        }

        {  // Unrelated memory buffer array equals by value:
            byte byteSrc[32 * sizeof (int)];
            ArrayView<int> arrayBytes(wrapMemory(byteSrc));
            arrayBytes.fill(fillOdd);

            CPPUNIT_ASSERT(array.equals(arrayBytes));
            CPPUNIT_ASSERT(arrayBytes.equals(array));
            CPPUNIT_ASSERT(array == arrayBytes);
            CPPUNIT_ASSERT(arrayBytes == array);
            CPPUNIT_ASSERT(!(array != arrayBytes));
            CPPUNIT_ASSERT(!(arrayBytes != array));
        }

        {  // Unrelated smaller memory buffer array filled with the same values no equals:
            byte byteSrc[24 * sizeof (int)];
            ArrayView<int> arrayBytes(wrapMemory(byteSrc));
            arrayBytes.fill(fillOdd);

            CPPUNIT_ASSERT(!array.equals(arrayBytes));
            CPPUNIT_ASSERT(!arrayBytes.equals(array));
            CPPUNIT_ASSERT(array != arrayBytes);
            CPPUNIT_ASSERT(arrayBytes != array);
            CPPUNIT_ASSERT(!(array == arrayBytes));
            CPPUNIT_ASSERT(!(arrayBytes == array));
        }

        {  // Unrelated memory buffer array filled with different values not equal by value:
            byte byteSrc[32 * sizeof (int)];
            ArrayView<int> arrayBytes(wrapMemory(byteSrc));
            arrayBytes.fill(fillEven);

            CPPUNIT_ASSERT(!array.equals(arrayBytes));
            CPPUNIT_ASSERT(!arrayBytes.equals(array));
            CPPUNIT_ASSERT(array != arrayBytes);
            CPPUNIT_ASSERT(arrayBytes != array);
            CPPUNIT_ASSERT(!(array == arrayBytes));
            CPPUNIT_ASSERT(!(arrayBytes == array));
        }
    }

    void testEquals_NonPodType() {
        static constexpr size_t kNonPodStruct = 81;

        NonPodStruct src[kNonPodStruct];
        generateTestArray(src, fillOddNonPods);

        auto array = arrayView(src);

        // Make sure it is not equals empty array if it is not empty
        CPPUNIT_ASSERT(!array.equals(ArrayView<NonPodStruct>()));
        CPPUNIT_ASSERT(!(array == ArrayView<NonPodStruct>()));
        CPPUNIT_ASSERT(!(array == nullptr));
        CPPUNIT_ASSERT(array != ArrayView<NonPodStruct>());
        CPPUNIT_ASSERT(array != nullptr);

        // Self equality is important
        CPPUNIT_ASSERT(array.equals(array));

        {  // Check that array views of the same memory are equal:
            ArrayView<NonPodStruct> arraySharingMemory(src);

            CPPUNIT_ASSERT(array.equals(arraySharingMemory));
            CPPUNIT_ASSERT(arraySharingMemory.equals(array));
            CPPUNIT_ASSERT(array == arraySharingMemory);
            CPPUNIT_ASSERT(arraySharingMemory == array);
            CPPUNIT_ASSERT(!(array != arraySharingMemory));
            CPPUNIT_ASSERT(!(arraySharingMemory != array));
        }

        {  // Unrelated memory buffer array equals by value:
            byte byteSrc[kNonPodStruct * sizeof(NonPodStruct)];
            NonPodGuard<NonPodStruct> guard(byteSrc, kNonPodStruct);

            ArrayView<NonPodStruct> arrayBytes(wrapMemory(byteSrc));
            arrayBytes.fill(fillOddNonPods);

            CPPUNIT_ASSERT(array.equals(arrayBytes));
            CPPUNIT_ASSERT(arrayBytes.equals(array));
            CPPUNIT_ASSERT(array == arrayBytes);
            CPPUNIT_ASSERT(arrayBytes == array);
            CPPUNIT_ASSERT(!(array != arrayBytes));
            CPPUNIT_ASSERT(!(arrayBytes != array));
        }

        {  // Unrelated smaller memory buffer array filled with the same values no equals:
            static constexpr size_t kOtherNonPodStruct = 112;
            byte byteSrc[kOtherNonPodStruct * sizeof(NonPodStruct)];
            NonPodGuard<NonPodStruct> guard(byteSrc, kOtherNonPodStruct);

            ArrayView<NonPodStruct> arrayBytes(wrapMemory(byteSrc));
            arrayBytes.fill(fillOddNonPods);

            CPPUNIT_ASSERT(!array.equals(arrayBytes));
            CPPUNIT_ASSERT(!arrayBytes.equals(array));
            CPPUNIT_ASSERT(array != arrayBytes);
            CPPUNIT_ASSERT(arrayBytes != array);
            CPPUNIT_ASSERT(!(array == arrayBytes));
            CPPUNIT_ASSERT(!(arrayBytes == array));
        }

        {  // Unrelated memory buffer array filled with different values not equal by value:
            byte byteSrc[kNonPodStruct * sizeof(NonPodStruct)];
            NonPodGuard<NonPodStruct> guard(byteSrc, kNonPodStruct);

            ArrayView<NonPodStruct> arrayBytes(wrapMemory(byteSrc));
            arrayBytes.fill(fillEvenNonPods);

            CPPUNIT_ASSERT(!array.equals(arrayBytes));
            CPPUNIT_ASSERT(!arrayBytes.equals(array));
            CPPUNIT_ASSERT(array != arrayBytes);
            CPPUNIT_ASSERT(arrayBytes != array);
            CPPUNIT_ASSERT(!(array == arrayBytes));
            CPPUNIT_ASSERT(!(arrayBytes == array));
        }
    }

    void testIndexOf() {
        int src[16];
        generateTestArray(src, [](size_t i) { return (2*static_cast<int>(i) - 1); });

        auto array = arrayView(src);

        {  // Test for existing value:
            const auto maybeIndex = array.indexOf(2*4 - 1);
            CPPUNIT_ASSERT(maybeIndex.isSome());
            CPPUNIT_ASSERT_EQUAL(ArrayView<int>::size_type(4), maybeIndex.get());
        }

        {  // Can we find this sequance? Yes we can
            const auto view = ArrayView<SimpleType>(wrapMemory(src));
            CPPUNIT_ASSERT(view.indexOf(SimpleType(5, 7, 9)).isSome());
        }

        {  // Test for non-existing value:
            CPPUNIT_ASSERT(array.indexOf(3*4 + 128).isNone());
        }

        {  // Test empty array contains nothing
            CPPUNIT_ASSERT(ArrayView<int>().indexOf(2*3 - 1).isNone());
        }

        {  // Can we find this sequance? Nope
            const auto view = ArrayView<SimpleType>(wrapMemory(src));
            CPPUNIT_ASSERT(view.indexOf(SimpleType(3, 2, 1)).isNone());
        }
    }

    void testContains() {
        {  // Test empty array contains nothing
            CPPUNIT_ASSERT(!ArrayView<int>().contains(2*3 - 1));
        }

        int src[24];
        generateTestArray(src, [](size_t i) { return static_cast<int>(i)*2 + 3; });


        auto array = arrayView(src);

        {  // Test for an existing value:
            CPPUNIT_ASSERT(array.contains(2*9 + 3));
        }

        {  // Test for non-existing value:
            CPPUNIT_ASSERT(!array.contains(-41));
        }

        {  // Can we find this sequance? Yes we can
            const auto view = ArrayView<SimpleType>(wrapMemory(src));
            CPPUNIT_ASSERT(view.contains(SimpleType(15, 17, 19)));
        }
    }

    void testFillWithConstValue() {
        int src[24];
        auto array = arrayView(src);

        array.fill(42);

        for (const auto i : array) {
            CPPUNIT_ASSERT_EQUAL(42, i);
        }
    }

    void testFillWithConstExplosiveValue() {
        CPPUNIT_ASSERT_EQUAL(0, SometimesConstructable::InstanceCount);
        {
            static constexpr size_t kNonPodStruct = 24;
            byte src[kNonPodStruct * sizeof (SometimesConstructable)];

            SometimesConstructable::BlowUpEveryInstance = 0;
            NonPodGuard<SometimesConstructable> guard(src, kNonPodStruct);

            SometimesConstructable::BlowUpEveryInstance = 9;
            ArrayView<SometimesConstructable> array(wrapMemory(src));

            // This should not throw as we don't create any new instances apart from +1 used as a temp template
            array.fill(SometimesConstructable(99));
            CPPUNIT_ASSERT_EQUAL(static_cast<int>(kNonPodStruct), SometimesConstructable::InstanceCount);

            for (int i = 0; i < SometimesConstructable::InstanceCount; ++i) {
                CPPUNIT_ASSERT_EQUAL(99, array[i].someValue);
            }
        }
        CPPUNIT_ASSERT_EQUAL(0, SometimesConstructable::InstanceCount);
    }

    void testFillWithGenerator() {
        int src[24];
        auto array = arrayView(src);

        array.fill([](ArrayView<int>::size_type i) { return static_cast<int>(i)*2 - 187; });

        for (ArrayView<int>::size_type i = 0; i < array.size(); ++i) {
            CPPUNIT_ASSERT_EQUAL(static_cast<int>(i)*2 - 187, array[i]);
        }
    }

    void testFillWithGeneratorOfExplosiveValue() {
        CPPUNIT_ASSERT_EQUAL(0, SometimesConstructable::InstanceCount);
        {
            static constexpr size_t kNonPodStruct = 81;
            byte src[kNonPodStruct * sizeof(SometimesConstructable)];

            SometimesConstructable::BlowUpEveryInstance = 0;
            NonPodGuard<SometimesConstructable> guard(src, kNonPodStruct);

            SometimesConstructable::BlowUpEveryInstance = 13;
            ArrayView<SometimesConstructable> array(wrapMemory(src));

            // This should not throw as we don't create any new instances apart from +1 used as a temp template
            array.fill([](size_t i ) { return SometimesConstructable(fillOdd(i)); });
            CPPUNIT_ASSERT_EQUAL(static_cast<int>(kNonPodStruct), SometimesConstructable::InstanceCount);

            for (int i = 0; i < SometimesConstructable::InstanceCount; ++i) {
                CPPUNIT_ASSERT_EQUAL(fillOdd(i), array[i].someValue);
            }
        }
        // Make sure that after the array has been destroyed no instances of SometimesConstructable exist.
        CPPUNIT_ASSERT_EQUAL(0, SometimesConstructable::InstanceCount);
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

        array.forEach([&allEq](ArrayView<int>::size_type i, int x) {
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
const char*		TestArrayView::NonPodStruct::STR_DEFAULT = "Deafult TestArrayView::NonPodStruct::STR_DEFAULT";


ArrayView<int>::size_type TestArrayView::NonPodStruct::TotalCount = 0;


CPPUNIT_TEST_SUITE_REGISTRATION(TestArrayView);
