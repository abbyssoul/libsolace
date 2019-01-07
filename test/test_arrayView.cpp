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

#include <gtest/gtest.h>
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



class TestArrayView : public ::testing::Test {

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

        ~DerivedNonPodStruct() override = default;

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
        auto const index = static_cast<int>(i)*2 - 1;
        auto str = std::string("Some Odd string: ") + std::to_string(index);

        return NonPodStruct(index, std::move(str));
    }

    static NonPodStruct fillEvenNonPods(size_t i) {
        auto const index = static_cast<int>(i)*2 + 1;
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

    void SetUp() override {
        // TODO(abbyssoul): Debug::BeginMemCheck();
        EXPECT_EQ(ZERO, NonPodStruct::TotalCount);
        EXPECT_EQ(0, SometimesConstructable::InstanceCount);
    }

    void TearDown() override {
        // TODO(abbyssoul): Debug::EndMemCheck();
        EXPECT_EQ(ZERO, NonPodStruct::TotalCount);
        EXPECT_EQ(0, SometimesConstructable::InstanceCount);
    }
};

TEST_F(TestArrayView, testEmpty) {
    {
        const ArrayView<int> empty_array;

        EXPECT_TRUE(empty_array.empty());
        EXPECT_EQ(ZERO, empty_array.size());
        EXPECT_TRUE(empty_array.begin() == empty_array.end());
        EXPECT_TRUE(empty_array == nullptr);
    }

    {
        const ArrayView<NonPodStruct> empty_array(nullptr);

        EXPECT_TRUE(empty_array.empty());
        EXPECT_EQ(ZERO, empty_array.size());
        EXPECT_TRUE(empty_array.begin() == empty_array.end());
        EXPECT_TRUE(empty_array == nullptr);
    }

    {
        const ArrayView<DerivedNonPodStruct> empty_array;

        EXPECT_TRUE(empty_array.empty());
        EXPECT_EQ(ZERO, empty_array.size());
        EXPECT_TRUE(empty_array.begin() == empty_array.end());
        EXPECT_TRUE(empty_array == nullptr);
    }
}


TEST_F(TestArrayView, testConstructionFromMemoryResource) {
    byte buffer[32];  // 32 bytes is 8 u32.

    EXPECT_EQ(32, arrayView(buffer).size());
    EXPECT_EQ(8, arrayView(reinterpret_cast<uint32*>(buffer), sizeof (buffer) / sizeof (uint32)).size());
}


TEST_F(TestArrayView, testCopyConstruction) {
    int src[16];
    const ArrayView<int>::size_type srcSize = sizeof(src) / sizeof(int);
    generateTestArray(src, fillOdd);


    ArrayView<int> a2(src);
    EXPECT_TRUE(!a2.empty());
    EXPECT_EQ(srcSize, a2.size());


    // Create a copy:
    ArrayView<int> a1(a2);

    EXPECT_TRUE(!a1.empty());
    EXPECT_EQ(a1.size(), a2.size());

    // Check that the data is the same:
    for (ArrayView<int>::size_type i = 0; i < a1.size(); ++i) {
        EXPECT_EQ(static_cast<int>(2*i) - 1, a1[i]);
    }

    // Check that changing values in the original C-array changes ArrayView values:
    auto newGen = [](size_t i) { return static_cast<int>(2*i + 3); };
    generateTestArray(src, newGen);

    EXPECT_EQ(a1.size(), a2.size());
    // Check that the data is the same:
    for (ArrayView<int>::size_type i = 0; i < a1.size(); ++i) {
        EXPECT_EQ(newGen(i), a1[i]);
        EXPECT_EQ(newGen(i), a2[i]);
    }
}

TEST_F(TestArrayView, testCopy) {
    {
        int src[16];
        generateTestArray(src, fillOdd);

        ArrayView<int> a1;
        ArrayView<int> a2(src);

        EXPECT_TRUE(a1.empty());
        EXPECT_TRUE(!a2.empty());

        // Copy arrays
        a1 = a2;

        EXPECT_TRUE(!a1.empty());
        EXPECT_TRUE(!a2.empty());
        EXPECT_EQ(a1.size(), a2.size());

        for (ArrayView<int>::size_type i = 0; i < a1.size(); ++i) {
            EXPECT_EQ(fillOdd(i), a1[i]);
        }

        // Make sure that if underlaying memory changed - this is reflected in arrayView:
        generateTestArray(src, fillEven);
        for (ArrayView<int>::size_type i = 0; i < a1.size(); ++i) {
            EXPECT_EQ(fillEven(i), a1[i]);
            EXPECT_EQ(fillEven(i), a2[i]);
        }
    }
}
/*
TEST_F(TestArrayView, testBasics) {
    ArrayView<uint> array(TEST_SIZE_0);

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
    EXPECT_EQ(static_cast<ArrayView<uint>::size_type>(count), array.size());

    for (auto i = ZERO, end = array.size(); i < end; ++i) {
        EXPECT_EQ(static_cast<uint>(i), array[i]);
    }

    // TODO(abbyssoul): Test access pass end!!
}

TEST_F(TestArrayView, testString) {
    ArrayView<String> array(TEST_SIZE_0);

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

TEST_F(TestArrayView, testNonPods) {
    EXPECT_EQ(ZERO, NonPodStruct::TotalCount);
    {
        ArrayView<NonPodStruct> array(TEST_SIZE_1);

        EXPECT_EQ(TEST_SIZE_1, array.size());
        EXPECT_EQ(NonPodStruct::TotalCount, array.size());

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

        EXPECT_EQ(static_cast<ArrayView<NonPodStruct>::size_type>(count), array.size());

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(static_cast<int>(i), array[i].iValue);
            EXPECT_EQ(String("Item " + std::to_string(i)), array[i].str);
        }
    }
    EXPECT_EQ(ZERO, NonPodStruct::TotalCount);
}

TEST_F(TestArrayView, testInitializerList) {
    {
        const int native_array[] = {0, 1, 2, 3};
        const ArrayView<int> array = {0, 1, 2, 3};

        EXPECT_EQ(nativeArrayLength(native_array), array.size());

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(native_array[i], array[i]);
        }
    }

    {
        const String native_array[] = {"Abc", "", "dfe", "_xyz3"};
        const ArrayView<String> array = {"Abc", "", "dfe", "_xyz3"};

        EXPECT_EQ(nativeArrayLength(native_array), array.size());

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(native_array[i], array[i]);
        }
    }

    EXPECT_EQ(ZERO, NonPodStruct::TotalCount);
    {
        const NonPodStruct native_array[] = {
                NonPodStruct(0, "yyyz"),
                NonPodStruct(),
                NonPodStruct(-321, "yyx"),
                NonPodStruct(990, "x^hhf")
        };
        EXPECT_EQ(nativeArrayLength(native_array), NonPodStruct::TotalCount);

        const ArrayView<NonPodStruct> array = {
                NonPodStruct(0, "yyyz"),
                NonPodStruct(),
                NonPodStruct(-321, "yyx"),
                NonPodStruct(990, "x^hhf")
        };

        EXPECT_EQ(nativeArrayLength(native_array), array.size());
        EXPECT_EQ(nativeArrayLength(native_array) + array.size(), NonPodStruct::TotalCount);

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(native_array[i].iValue, array[i].iValue);
            EXPECT_EQ(native_array[i].str, array[i].str);
        }
    }
    EXPECT_EQ(ZERO, NonPodStruct::TotalCount);

}

TEST_F(TestArrayView, testFromNativeConvertion) {

    {
        const int native_array[] = {0, 1, 2, 3};
        const ArrayView<int> array(nativeArrayLength(native_array), native_array);

        EXPECT_EQ(nativeArrayLength(native_array), array.size());

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(native_array[i], array[i]);
        }
    }

    {
        const String native_array[] = {"Abc", "", "dfe", "_xyz3"};
        const ArrayView<String> array(nativeArrayLength(native_array), native_array);

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
        const ArrayView<NonPodStruct> array(nativeArrayLength(native_array), native_array);

        EXPECT_EQ(nativeArrayLength(native_array), array.size());

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(native_array[i].iValue, array[i].iValue);
            EXPECT_EQ(native_array[i].str, array[i].str);
        }
    }
}

template <typename T>
ArrayView<T> moveArray(std::initializer_list<T> list) {
    return {list};
}

TEST_F(TestArrayView, testMoveAssignment) {
    {// Test on integral types
        ArrayView<int> array(0);

        EXPECT_TRUE(array.empty());
        EXPECT_EQ(ZERO, array.size());

        array = moveArray<int>({1, 2, 3});
        EXPECT_TRUE(!array.empty());
        const int src1[] = {1, 2, 3};

        EXPECT_EQ(3, array.size());
        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(src1[i], array[i]);
        }
    }

    {   // Test on strings types
        ArrayView<String> array(0);
        EXPECT_TRUE(array.empty());

        array = moveArray<String>({"tasrd", "", "hhha", "asd"});
        EXPECT_TRUE(!array.empty());

        const String src[] = {"tasrd", "", "hhha", "asd"};
        EXPECT_EQ(4, array.size());
        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(src[i], array[i]);
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
        EXPECT_TRUE(array.empty());

        array = moveArray<NonPodStruct>({
                                                NonPodStruct(0, "yyyz"),
                                                NonPodStruct(),
                                                NonPodStruct(-321, "yyx"),
                                                NonPodStruct(990, "x^hhf")
                                        });
        EXPECT_TRUE(!array.empty());
        EXPECT_EQ(4, array.size());

        for (auto i = ZERO, end = array.size(); i < end; ++i) {
            EXPECT_EQ(src[i].iValue, array[i].iValue);
            EXPECT_EQ(src[i].str, array[i].str);
        }
    }
}
*/

TEST_F(TestArrayView, testEquals_EmptyArray) {
    ArrayView<int> emptyArray;

    EXPECT_TRUE(emptyArray == nullptr);
    EXPECT_TRUE(!(emptyArray != nullptr));
    EXPECT_TRUE(emptyArray.equals(emptyArray));

    {  // Check that array views of the same memory are equal:
        ArrayView<int> differenEmptyArray;

        EXPECT_TRUE(emptyArray.equals(differenEmptyArray));
        EXPECT_TRUE(differenEmptyArray.equals(emptyArray));
        EXPECT_TRUE(emptyArray == differenEmptyArray);
        EXPECT_TRUE(differenEmptyArray == emptyArray);
        EXPECT_TRUE(!(emptyArray != differenEmptyArray));
        EXPECT_TRUE(!(differenEmptyArray != emptyArray));
    }
}

TEST_F(TestArrayView, testEquals_IntegralType) {
    int src[32];
    generateTestArray(src, fillOdd);

    auto array = arrayView(src);

    // Make sure it is not equals empty array if it is not empty
    EXPECT_TRUE(!array.equals(ArrayView<int>()));
    EXPECT_TRUE(!(array == ArrayView<int>()));
    EXPECT_TRUE(!(array == nullptr));
    EXPECT_TRUE(array != ArrayView<int>());
    EXPECT_TRUE(array != nullptr);

    // Self equality is important
    EXPECT_TRUE(array.equals(array));

    {  // Check that array views of the same memory are equal:
        ArrayView<int> arraySharingMemory(src);

        EXPECT_TRUE(array.equals(arraySharingMemory));
        EXPECT_TRUE(arraySharingMemory.equals(array));
        EXPECT_TRUE(array == arraySharingMemory);
        EXPECT_TRUE(arraySharingMemory == array);
        EXPECT_TRUE(!(array != arraySharingMemory));
        EXPECT_TRUE(!(arraySharingMemory != array));
    }

    {  // Unrelated memory buffer array equals by value:
        byte byteSrc[32 * sizeof (int)];
        ArrayView<int> arrayBytes(wrapMemory(byteSrc));
        arrayBytes.fill(fillOdd);

        EXPECT_TRUE(array.equals(arrayBytes));
        EXPECT_TRUE(arrayBytes.equals(array));
        EXPECT_TRUE(array == arrayBytes);
        EXPECT_TRUE(arrayBytes == array);
        EXPECT_TRUE(!(array != arrayBytes));
        EXPECT_TRUE(!(arrayBytes != array));
    }

    {  // Unrelated smaller memory buffer array filled with the same values no equals:
        byte byteSrc[24 * sizeof (int)];
        ArrayView<int> arrayBytes(wrapMemory(byteSrc));
        arrayBytes.fill(fillOdd);

        EXPECT_TRUE(!array.equals(arrayBytes));
        EXPECT_TRUE(!arrayBytes.equals(array));
        EXPECT_TRUE(array != arrayBytes);
        EXPECT_TRUE(arrayBytes != array);
        EXPECT_TRUE(!(array == arrayBytes));
        EXPECT_TRUE(!(arrayBytes == array));
    }

    {  // Unrelated memory buffer array filled with different values not equal by value:
        byte byteSrc[32 * sizeof (int)];
        ArrayView<int> arrayBytes(wrapMemory(byteSrc));
        arrayBytes.fill(fillEven);

        EXPECT_TRUE(!array.equals(arrayBytes));
        EXPECT_TRUE(!arrayBytes.equals(array));
        EXPECT_TRUE(array != arrayBytes);
        EXPECT_TRUE(arrayBytes != array);
        EXPECT_TRUE(!(array == arrayBytes));
        EXPECT_TRUE(!(arrayBytes == array));
    }
}

TEST_F(TestArrayView, testEquals_NonPodType) {
    static constexpr size_t kNonPodStruct = 81;

    NonPodStruct src[kNonPodStruct];
    generateTestArray(src, fillOddNonPods);

    auto array = arrayView(src);

    // Make sure it is not equals empty array if it is not empty
    EXPECT_TRUE(!array.equals(ArrayView<NonPodStruct>()));
    EXPECT_TRUE(!(array == ArrayView<NonPodStruct>()));
    EXPECT_TRUE(!(array == nullptr));
    EXPECT_TRUE(array != ArrayView<NonPodStruct>());
    EXPECT_TRUE(array != nullptr);

    // Self equality is important
    EXPECT_TRUE(array.equals(array));

    {  // Check that array views of the same memory are equal:
        ArrayView<NonPodStruct> arraySharingMemory(src);

        EXPECT_TRUE(array.equals(arraySharingMemory));
        EXPECT_TRUE(arraySharingMemory.equals(array));
        EXPECT_TRUE(array == arraySharingMemory);
        EXPECT_TRUE(arraySharingMemory == array);
        EXPECT_TRUE(!(array != arraySharingMemory));
        EXPECT_TRUE(!(arraySharingMemory != array));
    }

    {  // Unrelated memory buffer array equals by value:
        byte byteSrc[kNonPodStruct * sizeof(NonPodStruct)];
        NonPodGuard<NonPodStruct> guard(byteSrc, kNonPodStruct);

        ArrayView<NonPodStruct> arrayBytes(wrapMemory(byteSrc));
        arrayBytes.fill(fillOddNonPods);

        EXPECT_TRUE(array.equals(arrayBytes));
        EXPECT_TRUE(arrayBytes.equals(array));
        EXPECT_TRUE(array == arrayBytes);
        EXPECT_TRUE(arrayBytes == array);
        EXPECT_TRUE(!(array != arrayBytes));
        EXPECT_TRUE(!(arrayBytes != array));
    }

    {  // Unrelated smaller memory buffer array filled with the same values no equals:
        static constexpr size_t kOtherNonPodStruct = 112;
        byte byteSrc[kOtherNonPodStruct * sizeof(NonPodStruct)];
        NonPodGuard<NonPodStruct> guard(byteSrc, kOtherNonPodStruct);

        ArrayView<NonPodStruct> arrayBytes(wrapMemory(byteSrc));
        arrayBytes.fill(fillOddNonPods);

        EXPECT_TRUE(!array.equals(arrayBytes));
        EXPECT_TRUE(!arrayBytes.equals(array));
        EXPECT_TRUE(array != arrayBytes);
        EXPECT_TRUE(arrayBytes != array);
        EXPECT_TRUE(!(array == arrayBytes));
        EXPECT_TRUE(!(arrayBytes == array));
    }

    {  // Unrelated memory buffer array filled with different values not equal by value:
        byte byteSrc[kNonPodStruct * sizeof(NonPodStruct)];
        NonPodGuard<NonPodStruct> guard(byteSrc, kNonPodStruct);

        ArrayView<NonPodStruct> arrayBytes(wrapMemory(byteSrc));
        arrayBytes.fill(fillEvenNonPods);

        EXPECT_TRUE(!array.equals(arrayBytes));
        EXPECT_TRUE(!arrayBytes.equals(array));
        EXPECT_TRUE(array != arrayBytes);
        EXPECT_TRUE(arrayBytes != array);
        EXPECT_TRUE(!(array == arrayBytes));
        EXPECT_TRUE(!(arrayBytes == array));
    }
}

TEST_F(TestArrayView, testIndexOf) {
    int src[16];
    generateTestArray(src, [](size_t i) { return (2*static_cast<int>(i) - 1); });

    auto array = arrayView(src);

    {  // Test for existing value:
        auto const maybeIndex = array.indexOf(2*4 - 1);
        EXPECT_TRUE(maybeIndex.isSome());
        EXPECT_EQ(ArrayView<int>::size_type(4), maybeIndex.get());
    }

    {  // Can we find this sequance? Yes we can
        auto const view = ArrayView<SimpleType>(wrapMemory(src));
        EXPECT_TRUE(view.indexOf(SimpleType(5, 7, 9)).isSome());
    }

    {  // Test for non-existing value:
        EXPECT_TRUE(array.indexOf(3*4 + 128).isNone());
    }

    {  // Test empty array contains nothing
        EXPECT_TRUE(ArrayView<int>().indexOf(2*3 - 1).isNone());
    }

    {  // Can we find this sequance? Nope
        auto const view = ArrayView<SimpleType>(wrapMemory(src));
        EXPECT_TRUE(view.indexOf(SimpleType(3, 2, 1)).isNone());
    }
}

TEST_F(TestArrayView, testContains) {
    {  // Test empty array contains nothing
        EXPECT_TRUE(!ArrayView<int>().contains(2*3 - 1));
    }

    int src[24];
    generateTestArray(src, [](size_t i) { return static_cast<int>(i)*2 + 3; });


    auto array = arrayView(src);

    {  // Test for an existing value:
        EXPECT_TRUE(array.contains(2*9 + 3));
    }

    {  // Test for non-existing value:
        EXPECT_TRUE(!array.contains(-41));
    }

    {  // Can we find this sequance? Yes we can
        auto const view = ArrayView<SimpleType>(wrapMemory(src));
        EXPECT_TRUE(view.contains(SimpleType(15, 17, 19)));
    }
}

TEST_F(TestArrayView, testFillWithConstValue) {
    int src[24];
    auto array = arrayView(src);

    array.fill(42);

    for (auto const i : array) {
        EXPECT_EQ(42, i);
    }
}

TEST_F(TestArrayView, testFillWithConstExplosiveValue) {
    EXPECT_EQ(0, SometimesConstructable::InstanceCount);
    {
        static constexpr size_t kNonPodStruct = 24;
        byte src[kNonPodStruct * sizeof (SometimesConstructable)];

        SometimesConstructable::BlowUpEveryInstance = 0;
        NonPodGuard<SometimesConstructable> guard(src, kNonPodStruct);

        SometimesConstructable::BlowUpEveryInstance = 9;
        ArrayView<SometimesConstructable> array(wrapMemory(src));

        // This should not throw as we don't create any new instances apart from +1 used as a temp template
        array.fill(SometimesConstructable(99));
        EXPECT_EQ(static_cast<int>(kNonPodStruct), SometimesConstructable::InstanceCount);

        for (int i = 0; i < SometimesConstructable::InstanceCount; ++i) {
            EXPECT_EQ(99, array[i].someValue);
        }
    }
    EXPECT_EQ(0, SometimesConstructable::InstanceCount);
}

TEST_F(TestArrayView, testFillWithGenerator) {
    int src[24];
    auto array = arrayView(src);

    array.fill([](ArrayView<int>::size_type i) { return static_cast<int>(i)*2 - 187; });

    for (ArrayView<int>::size_type i = 0; i < array.size(); ++i) {
        EXPECT_EQ(static_cast<int>(i)*2 - 187, array[i]);
    }
}


TEST_F(TestArrayView, slice) {
    int src[24];
    auto array = arrayView(src);

    array.fill([](ArrayView<int>::size_type i) { return static_cast<int>(i); });

    EXPECT_EQ(array.size(), array.slice(0, array.size()).size());
    EXPECT_EQ(array, array.slice(0, array.size()));

    auto halfView = array.slice(12, 22);
    EXPECT_EQ(10, halfView.size());
    for (ArrayView<int>::size_type i = 0; i < halfView.size(); ++i) {
        EXPECT_EQ(12 + i, halfView[i]);
    }

    EXPECT_TRUE(array.slice(12, 12).empty());
    EXPECT_TRUE(array.slice(128, 300).empty());
    EXPECT_EQ(14, array.slice(10, 300).size());
    EXPECT_TRUE(array.slice(128, 21).empty());
    EXPECT_TRUE(array.slice(21, 7).empty());
}


TEST_F(TestArrayView, testFillWithGeneratorOfExplosiveValue) {
    EXPECT_EQ(0, SometimesConstructable::InstanceCount);
    {
        static constexpr size_t kNonPodStruct = 81;
        byte src[kNonPodStruct * sizeof(SometimesConstructable)];

        SometimesConstructable::BlowUpEveryInstance = 0;
        NonPodGuard<SometimesConstructable> guard(src, kNonPodStruct);

        SometimesConstructable::BlowUpEveryInstance = 13;
        ArrayView<SometimesConstructable> array(wrapMemory(src));

        // This should not throw as we don't create any new instances apart from +1 used as a temp template
        array.fill([](size_t i ) { return SometimesConstructable(fillOdd(i)); });
        EXPECT_EQ(static_cast<int>(kNonPodStruct), SometimesConstructable::InstanceCount);

        for (int i = 0; i < SometimesConstructable::InstanceCount; ++i) {
            EXPECT_EQ(fillOdd(i), array[i].someValue);
        }
    }
    // Make sure that after the array has been destroyed no instances of SometimesConstructable exist.
    EXPECT_EQ(0, SometimesConstructable::InstanceCount);
}


TEST_F(TestArrayView, testForEach_byValue) {
    int baseArray[] = {1, 2, 3, 4, 5, 6};
    const ArrayView<int> array{baseArray};

    int acc = 0;
    array.forEach([&acc](int x) {
        acc += x;
    });

    EXPECT_EQ(21, acc);
}

TEST_F(TestArrayView, testForEach_byConstRef) {
    EXPECT_EQ(0, SimpleType::InstanceCount);

    {
        SimpleType baseArray[] = {{3, 2, 1}, {2, 1, 3}, {0, -1, 2}, {-1, 0, -4}};
        const ArrayView<SimpleType> array{baseArray};

        SimpleType acc;
        array.forEach([&acc](const SimpleType& x) {
            acc.x += x.x;
            acc.y += x.y;
            acc.z += x.z;
        });

        EXPECT_EQ(SimpleType(4, 2, 2), acc);
    }
    EXPECT_EQ(0, SimpleType::InstanceCount);
}

TEST_F(TestArrayView, testForEach_byValueConversion) {
    int baseArray[] = {1, 2, 3, 4, 5, 6};
    const ArrayView<int> array{baseArray};

    double acc = 0;
    array.forEach([&acc](double x) {
        acc += x;
    });

    EXPECT_FLOAT_EQ(21.0, acc);
}


TEST_F(TestArrayView, testForEachIndexed) {
    int baseArray[] = {1, 2, 3, 4, 5, 6};
    const ArrayView<int> array{baseArray};
    bool allEq = true;

    array.forEach([&allEq](ArrayView<int>::size_type i, uint x) {
        allEq &= (i + 1 == x);
    });

    EXPECT_EQ(true, allEq);
}

/*
TEST_F(TestArrayView, testMap) {
    DerivedNonPodStruct baseArray[] = {
        DerivedNonPodStruct(32, 2.4, "hello"),
        DerivedNonPodStruct(-24, 2.4, " "),
        DerivedNonPodStruct(10, 2.4, "world")
    };
    const ArrayView<DerivedNonPodStruct> array{baseArray};

    {
        auto r = array.map([](const DerivedNonPodStruct& content) {
            return content.iValue;
        });

        EXPECT_EQ(array.size(), r.size());
        for (ArrayView<int>::size_type i = 0; i < array.size(); ++i) {
            EXPECT_EQ(r[i], array[i].iValue);
        }

    }
    {
        auto r = array.map([](const DerivedNonPodStruct& content) {
            return content.str;
        });

        EXPECT_EQ(array.size(), r.size());
        for (ArrayView<int>::size_type i = 0; i < array.size(); ++i) {
            EXPECT_EQ(r[i], array[i].str);
        }
    }
}
*/


const ArrayView<int>::size_type TestArrayView::ZERO = 0;
const ArrayView<int>::size_type TestArrayView::TEST_SIZE_0 = 7;
const ArrayView<int>::size_type TestArrayView::TEST_SIZE_1 = 35;

const int 		TestArrayView::NonPodStruct::IVALUE_DEFAULT = -123;
const char*		TestArrayView::NonPodStruct::STR_DEFAULT = "Deafult TestArrayView::NonPodStruct::STR_DEFAULT";


ArrayView<int>::size_type TestArrayView::NonPodStruct::TotalCount = 0;
