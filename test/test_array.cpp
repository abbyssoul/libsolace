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
 *	@author		soultaker
 *	@date		$LastChangedDate$
 *	@brief		Test set of April::Array
 *	ID:			$Id$
 ******************************************************************************/
#include <solace/array.hpp>    // Class being tested.
#include <solace/string.hpp>   // Non POD subject.
#include <solace/exception.hpp>


#include <cppunit/extensions/HelperMacros.h>


using namespace Solace;


template <typename T, size_t N>
uint32 nativeArrayLength(const T (& SOLACE_UNUSED(t))[N]) { return N; }


class TestArray : public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestArray);
        CPPUNIT_TEST(testEmpty);
        CPPUNIT_TEST(testCopy);

        CPPUNIT_TEST(testBasics);
        CPPUNIT_TEST(testString);
        CPPUNIT_TEST(testNonPods);
        CPPUNIT_TEST(testInitializerList);
        CPPUNIT_TEST(testFromNativeConvertion);
        CPPUNIT_TEST(testMoveAssignment);

        CPPUNIT_TEST(testEquals);
        CPPUNIT_TEST(testIndexOf);
        CPPUNIT_TEST(testContains);
        CPPUNIT_TEST(testExtend);

        CPPUNIT_TEST(testForEach);
        CPPUNIT_TEST(testMap);

        CPPUNIT_TEST(testDeallocationWhenElementConstructorThrows);
    CPPUNIT_TEST_SUITE_END();

protected:

	static const Array<int>::size_type ZERO;
	static const Array<int>::size_type TEST_SIZE_0;
	static const Array<int>::size_type TEST_SIZE_1;

	struct NonPodStruct {

		static Array<int>::size_type TotalCount;

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

		DerivedNonPodStruct() :
            NonPodStruct(312, "Derived String"), fValue(3.1415f)
		{
		}

        DerivedNonPodStruct(int x, float f, const String& inStr) :
                NonPodStruct(x, inStr), fValue(f)
        {
        }

        virtual ~DerivedNonPodStruct() = default;

	};

    struct SometimesConstructable {
        static int InstanceCount;
        static int BlowUpEveryInstance;

        int someValue;

        SometimesConstructable(): someValue(3) {
            if ((InstanceCount + 1) % BlowUpEveryInstance) {
                throw Exception("Blowing up");
            }

            ++InstanceCount;
        }

        ~SometimesConstructable() {
            --InstanceCount;
        }

        SometimesConstructable(const SometimesConstructable& rhs): someValue(rhs.someValue)
        {
            ++InstanceCount;
        }

        SometimesConstructable(SometimesConstructable&& rhs): someValue(rhs.someValue)
        {
            ++InstanceCount;
        }

        SometimesConstructable& operator= (const SometimesConstructable& rhs) {
            someValue = rhs.someValue;

            return *this;
        }

        SometimesConstructable& operator= (SometimesConstructable&& rhs) {
            someValue = rhs.someValue;
            return *this;
        }

        bool operator == (const SometimesConstructable& rhs) const {
            return someValue == rhs.someValue;
        }
    };


public:

    // cppcheck-suppress unusedFunction
    void setUp() override {
        // TODO(abbyssoul): Debug::BeginMemCheck();
        CPPUNIT_ASSERT_EQUAL(ZERO, NonPodStruct::TotalCount);
        CPPUNIT_ASSERT_EQUAL(0, SometimesConstructable::InstanceCount);
	}

    // cppcheck-suppress unusedFunction
    void tearDown() override {
        // TODO(abbyssoul): Debug::EndMemCheck();
        CPPUNIT_ASSERT_EQUAL(ZERO, NonPodStruct::TotalCount);
        CPPUNIT_ASSERT_EQUAL(0, SometimesConstructable::InstanceCount);
    }

	void testEmpty() {
        {
            const Array<int> empty_array(0);

            CPPUNIT_ASSERT(empty_array.empty());
            CPPUNIT_ASSERT_EQUAL(ZERO, empty_array.size());
        }

        {
            const Array<NonPodStruct> empty_array(0);

            CPPUNIT_ASSERT(empty_array.empty());
            CPPUNIT_ASSERT_EQUAL(ZERO, empty_array.size());
        }

        {
            const Array<DerivedNonPodStruct> empty_array(0);

            CPPUNIT_ASSERT(empty_array.empty());
            CPPUNIT_ASSERT_EQUAL(ZERO, empty_array.size());
        }
	}

	void testCopy() {
        {
            Array<int> a1(0);
            Array<int> a2(20);

            CPPUNIT_ASSERT(a1.empty());
            CPPUNIT_ASSERT(!a2.empty());

            for (Array<int>::size_type i = 0; i < a2.size(); ++i) {
                a2[i] = static_cast<int>(2*i - 1);
            }

            a1 = a2;
            CPPUNIT_ASSERT(!a1.empty());
            CPPUNIT_ASSERT_EQUAL(a1.size(), a2.size());
            for (Array<int>::size_type i = 0; i < a1.size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(static_cast<int>(2*i - 1), a1[i]);
            }
        }
	}

	void testBasics() {
		Array<uint> array(TEST_SIZE_0);

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
		CPPUNIT_ASSERT_EQUAL(static_cast<Array<uint>::size_type>(count), array.size());

		for (auto i = ZERO, end = array.size(); i < end; ++i) {
			CPPUNIT_ASSERT_EQUAL(static_cast<uint>(i), array[i]);
		}

        // TODO(abbyssoul): Test access pass end!!
	}

	void testString() {
		Array<String> array(TEST_SIZE_0);

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
            Array<NonPodStruct> array(TEST_SIZE_1);

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

            CPPUNIT_ASSERT_EQUAL(static_cast<Array<NonPodStruct>::size_type>(count), array.size());

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
			const Array<int> array = {0, 1, 2, 3};

            CPPUNIT_ASSERT_EQUAL(nativeArrayLength(native_array), array.size());

			for (auto i = ZERO, end = array.size(); i < end; ++i) {
				CPPUNIT_ASSERT_EQUAL(native_array[i], array[i]);
			}
		}

		{
			const String native_array[] = {"Abc", "", "dfe", "_xyz3"};
			const Array<String> array = {"Abc", "", "dfe", "_xyz3"};

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

			const Array<NonPodStruct> array = {
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
            const Array<int> array(nativeArrayLength(native_array), native_array);

            CPPUNIT_ASSERT_EQUAL(nativeArrayLength(native_array), array.size());

			for (auto i = ZERO, end = array.size(); i < end; ++i) {
				CPPUNIT_ASSERT_EQUAL(native_array[i], array[i]);
			}
		}

		{
			const String native_array[] = {"Abc", "", "dfe", "_xyz3"};
            const Array<String> array(nativeArrayLength(native_array), native_array);

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
            const Array<NonPodStruct> array(nativeArrayLength(native_array), native_array);

            CPPUNIT_ASSERT_EQUAL(nativeArrayLength(native_array), array.size());

			for (auto i = ZERO, end = array.size(); i < end; ++i) {
				CPPUNIT_ASSERT_EQUAL(native_array[i].iValue, array[i].iValue);
				CPPUNIT_ASSERT_EQUAL(native_array[i].str, array[i].str);
			}
		}
	}

    template <typename T>
    Array<T> moveArray(std::initializer_list<T> list) {
        return {list};
    }

	void testMoveAssignment() {
		{// Test on integral types
			Array<int> array(0);

            CPPUNIT_ASSERT(array.empty());
			CPPUNIT_ASSERT_EQUAL(ZERO, array.size());

            array = moveArray<int>({1, 2, 3});
            CPPUNIT_ASSERT(!array.empty());
            const int src1[] = {1, 2, 3};

            CPPUNIT_ASSERT_EQUAL(static_cast<Array<int>::size_type>(3), array.size());
			for (auto i = ZERO, end = array.size(); i < end; ++i) {
				CPPUNIT_ASSERT_EQUAL(src1[i], array[i]);
			}
		}

        {   // Test on strings types
			Array<String> array(0);
            CPPUNIT_ASSERT(array.empty());

            array = moveArray<String>({"tasrd", "", "hhha", "asd"});
            CPPUNIT_ASSERT(!array.empty());

            const String src[] = {"tasrd", "", "hhha", "asd"};
            CPPUNIT_ASSERT_EQUAL(static_cast<Array<String>::size_type>(4), array.size());
			for (auto i = ZERO, end = array.size(); i < end; ++i) {
				CPPUNIT_ASSERT_EQUAL(src[i], array[i]);
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
            CPPUNIT_ASSERT(array.empty());

            array = moveArray<NonPodStruct>({
                                                    NonPodStruct(0, "yyyz"),
                                                    NonPodStruct(),
                                                    NonPodStruct(-321, "yyx"),
                                                    NonPodStruct(990, "x^hhf")
                                            });
            CPPUNIT_ASSERT(!array.empty());
            CPPUNIT_ASSERT_EQUAL(static_cast<Array<NonPodStruct>::size_type>(4), array.size());

			for (auto i = ZERO, end = array.size(); i < end; ++i) {
				CPPUNIT_ASSERT_EQUAL(src[i].iValue, array[i].iValue);
				CPPUNIT_ASSERT_EQUAL(src[i].str, array[i].str);
			}
		}
	}

	void testEquals() {
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

	void testIndexOf() {
        // TODO(abbyssoul): Implementation
	}

    void testContains() {
        // TODO(abbyssoul): Implementation
    }

    void testExtend() {
        // TODO(abbyssoul): Implementation
    }

/*
    void testFill() {
        {
            Array<int> array(5);

            const int equal_array[] = {1, 2, 3};
            const size_t equal_length = nativeArrayLength(equal_array);

            const int nequal_array[] = {0, 1, 2, 3};
            const size_t nequal_length = nativeArrayLength(nequal_array);

            array.fill(equal_array, equal_length, 1);
            CPPUNIT_ASSERT_EQUAL(true, array.equals({1, 1, 2, 3, array[4]}));

            array.fill(nequal_array, nequal_length, 1);
            CPPUNIT_ASSERT_EQUAL(true, array.equals({1, 0, 1, 2, 3}));

            CPPUNIT_ASSERT_THROW(array.fill(nequal_array, nequal_length, 1),
                    std::overflow_error);

            CPPUNIT_ASSERT_THROW(array.fill(nequal_array, 1, array.size() + 1),
                    std::range_error);

            CPPUNIT_ASSERT_THROW(array.fill({5, 4, 3, 2, 1, 0}),
                    std::overflow_error);

            array.fill({4, 3, 2, 1, 0});
            CPPUNIT_ASSERT_EQUAL(true, array.equals({4, 3, 2, 1, 0}));
        }
    }
*/

    void testForEach() {
        {
            const Array<int> array = {1, 2, 3, 4, 5, 6};

            int acc = 0;
            array.forEach([&acc](int x) {
                acc += x;
            });

            CPPUNIT_ASSERT_EQUAL(21, acc);
        }
        {
            const Array<String> array = {"Hello", " ", "world", "!"};

            String acc;
            array.forEach([&acc](const String& x) {
                acc = acc.concat(x);
            });

            CPPUNIT_ASSERT_EQUAL(String("Hello world!"), acc);
        }
        {
            const Array<int> array = {1, 2, 3, 4, 5, 6};
            bool allEq = true;
            array.forEach([&allEq](Array<int>::size_type i, Array<int>::size_type x) {
                allEq &= (i + 1 == x);
            });

            CPPUNIT_ASSERT_EQUAL(true, allEq);
        }
    }

    void testMap() {
        const Array<DerivedNonPodStruct> array = {
                DerivedNonPodStruct(32, 2.4, "hello"),
                DerivedNonPodStruct(-24, 2.4, " "),
                DerivedNonPodStruct(10, 2.4, "world"),
        };

        {
            auto r = array.map<int>([](const DerivedNonPodStruct& content) {
                return content.iValue;
            });

            CPPUNIT_ASSERT_EQUAL(array.size(), r.size());
            for (Array<int>::size_type i = 0; i < array.size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(r[i], array[i].iValue);
            }

        }
        {
            auto r = array.map<String>([](const DerivedNonPodStruct& content) {
                return content.str;
            });

            CPPUNIT_ASSERT_EQUAL(array.size(), r.size());
            for (Array<int>::size_type i = 0; i < array.size(); ++i) {
                CPPUNIT_ASSERT_EQUAL(r[i], array[i].str);
            }

        }
    }

    void testDeallocationWhenElementConstructorThrows() {

        SometimesConstructable::BlowUpEveryInstance = 9;

        CPPUNIT_ASSERT_THROW(const Array<SometimesConstructable> sholdFail(10), Exception);
        CPPUNIT_ASSERT_EQUAL(0, SometimesConstructable::InstanceCount);
    }

};

const Array<int>::size_type TestArray::ZERO = 0;
const Array<int>::size_type TestArray::TEST_SIZE_0 = 7;
const Array<int>::size_type TestArray::TEST_SIZE_1 = 35;

const int 		TestArray::NonPodStruct::IVALUE_DEFAULT = -123;
const String 	TestArray::NonPodStruct::STR_DEFAULT = "test_value";

Array<int>::size_type TestArray::NonPodStruct::TotalCount = 0;

int TestArray::SometimesConstructable::InstanceCount = 0;
int TestArray::SometimesConstructable::BlowUpEveryInstance = 4;

CPPUNIT_TEST_SUITE_REGISTRATION(TestArray);
