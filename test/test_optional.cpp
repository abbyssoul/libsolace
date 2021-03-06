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
 * @file: test/test_optional.cpp
*******************************************************************************/
#include <solace/optional.hpp>			// Class being tested
#include <solace/string.hpp>
#include <solace/exception.hpp>
#include <solace/output_utils.hpp>

#include <cmath>

#include <gtest/gtest.h>
#include "mockTypes.hpp"

using namespace Solace;

class TestOptional : public testing::Test  {

public:

    Optional<String> moveOptionalString(String&& value) {
		return Optional<String>(mv(value));
    }

	Optional<String> moveOptionalString(Result<String, Error>&& value) {

		return value
				? Optional<String>(value.moveResult())
				: none;
	}

    SimpleType moveSimpleType(int x, int y, int z) {
        return SimpleType{x, y, z};
    }

    MoveOnlyType moveMoveonlyType(int value) {
        return MoveOnlyType{value};
    }

public:

    void SetUp() override {
        EXPECT_EQ(0, PimitiveType::InstanceCount);
        EXPECT_EQ(0, SimpleType::InstanceCount);
        EXPECT_EQ(0, MoveOnlyType::InstanceCount);
    }

    void TearDown() override {
        EXPECT_EQ(0, PimitiveType::InstanceCount);
        EXPECT_EQ(0, SimpleType::InstanceCount);
        EXPECT_EQ(0, MoveOnlyType::InstanceCount);
    }
};

TEST_F(TestOptional, testConstructionIntegrals) {
    {
        {
            const Optional<int> v;
            EXPECT_TRUE(v.isNone());
        }

        {
            const int x = 32;
            auto v2 = Optional<int>(x);
            EXPECT_TRUE(v2.isSome());

            EXPECT_EQ(x, v2.get());
        }
    }

    {
        {
            const Optional<uint64> v;
            EXPECT_TRUE(v.isNone());
        }

        {
            uint64 x = 32;
            auto v2 = Optional<uint64>(x);
            EXPECT_TRUE(v2.isSome());

            EXPECT_EQ(x, v2.get());
        }
    }
}

TEST_F(TestOptional, testConstructionOf) {
    SimpleType t(1, -32, 3);
    EXPECT_TRUE(Optional<SimpleType>(t).isSome());
}

TEST_F(TestOptional, testMoveConstructionOf) {
    EXPECT_EQ(0, MoveOnlyType::InstanceCount);

    const int randomNumber = 3987;
    auto v2 = Optional<MoveOnlyType>(moveMoveonlyType(randomNumber));
    EXPECT_TRUE(v2.isSome());
    EXPECT_EQ(randomNumber, v2.get().x_);
    EXPECT_EQ(1, MoveOnlyType::InstanceCount);

	auto v3 = Optional<MoveOnlyType>(mv(v2));
    EXPECT_TRUE(v2.isSome());  // To be std::optional complient - moved-from optional is stil considered to hold a value
    EXPECT_TRUE(v3.isSome());
    EXPECT_EQ(randomNumber, (*v3).x_);
    EXPECT_EQ(2, MoveOnlyType::InstanceCount);
}


TEST_F(TestOptional, testConstruction) {
    {
        EXPECT_TRUE(Optional<int>().isNone());
        EXPECT_TRUE(Optional<SimpleType>().isNone());
        EXPECT_TRUE(Optional<int>{none}.isNone());
    }

    {
        EXPECT_EQ(0, MoveOnlyType::InstanceCount);
        const Optional<MoveOnlyType> v;
        EXPECT_TRUE(v.isNone());
        EXPECT_EQ(0, MoveOnlyType::InstanceCount);
    }
}

TEST_F(TestOptional, copyConstruct) {
	Optional<SimpleType> someValue{in_place, 3, 2, 1};
	EXPECT_EQ(1, SimpleType::InstanceCount);

	Optional<SimpleType> copyValue = someValue;
	EXPECT_TRUE(copyValue.isSome());
	EXPECT_EQ(2, SimpleType::InstanceCount);

	EXPECT_EQ(3, copyValue.get().x);
	EXPECT_EQ(2, copyValue.get().y);
	EXPECT_EQ(1, copyValue.get().z);
}


TEST_F(TestOptional, testConstructorTypeConvertion) {
    EXPECT_EQ(0, PimitiveType::InstanceCount);

    Optional<PimitiveType> ptype = Optional<int>(321);

    EXPECT_EQ(1, PimitiveType::InstanceCount);
	EXPECT_EQ(321, ptype.get().value);
}

/*
TEST_F(TestOptional, testAssignment) {
    {
        auto v1 = Optional<SimpleType>{};
        auto v2 = Optional<SimpleType>({3, 2, -123});

        EXPECT_TRUE(v1.isNone());
        EXPECT_EQ(1, SimpleType::InstanceCount);
        v1 = v2;
        EXPECT_EQ(2, SimpleType::InstanceCount);
        EXPECT_TRUE(v1.isSome());
        v2 = Optional<SimpleType>{};
        EXPECT_EQ(1, SimpleType::InstanceCount);
        v1 = v2;
        EXPECT_EQ(0, SimpleType::InstanceCount);
    }

    EXPECT_EQ(0, SimpleType::InstanceCount);
    {
        auto v2 = Optional<SimpleType>(moveSimpleType(3, 2718, 321));
        Optional<SimpleType> v1 = None();

        EXPECT_TRUE(v1.isNone());
        EXPECT_EQ(1, SimpleType::InstanceCount);
        v1 = v2;
        EXPECT_EQ(2, SimpleType::InstanceCount);
    }
}
*/

TEST_F(TestOptional, testSwap) {
    {
        auto v1 = Optional<int>{};
        auto v2 = Optional<int>{};

        EXPECT_TRUE(v1.isNone());
        swap(v1, v2);

        v2 = Optional<int>(3);
        EXPECT_TRUE(v1.isNone());
        EXPECT_TRUE(v2.isSome());
        EXPECT_EQ(3, v2.get());

        swap(v1, v2);
        EXPECT_TRUE(v1.isSome());
        EXPECT_TRUE(v2.isNone());

        EXPECT_EQ(3, v1.get());
        EXPECT_THROW(v2.get(), InvalidStateException);
    }
    {
        auto v1 = Optional<SimpleType>{};
        auto v2 = Optional<SimpleType>{};

        EXPECT_EQ(0, SimpleType::InstanceCount);
        EXPECT_TRUE(v1.isNone());
        swap(v1, v2);
        EXPECT_TRUE(v1.isNone());
        EXPECT_TRUE(v2.isNone());
        EXPECT_EQ(0, SimpleType::InstanceCount);

        v2 = Optional<SimpleType>(SimpleType{1, -3, 412});
        EXPECT_TRUE(v1.isNone());
        EXPECT_TRUE(v2.isSome());
        EXPECT_EQ(1, SimpleType::InstanceCount);
        EXPECT_EQ(-3, v2.get().y);

        swap(v1, v2);
        EXPECT_EQ(1, SimpleType::InstanceCount);
        EXPECT_TRUE(v1.isSome());
        EXPECT_TRUE(v2.isNone());

        EXPECT_EQ(412, v1.get().z);
        EXPECT_THROW(v2.get(), InvalidStateException);
    }

    {
        auto v1 = Optional<MoveOnlyType>{};
        auto v2 = Optional<MoveOnlyType>{};

        EXPECT_EQ(0, MoveOnlyType::InstanceCount);
        EXPECT_TRUE(v1.isNone());
        swap(v1, v2);
        EXPECT_TRUE(v1.isNone());
        EXPECT_TRUE(v2.isNone());
        EXPECT_EQ(0, SimpleType::InstanceCount);

        v2 = Optional<MoveOnlyType>(MoveOnlyType{998});
        EXPECT_TRUE(v1.isNone());
        EXPECT_TRUE(v2.isSome());
        EXPECT_EQ(1, MoveOnlyType::InstanceCount);
        EXPECT_EQ(998, v2.get().x_);

        swap(v1, v2);
        EXPECT_EQ(1, MoveOnlyType::InstanceCount);
        EXPECT_TRUE(v1.isSome());
        EXPECT_TRUE(v2.isNone());

        EXPECT_EQ(998, v1.get().x_);
        EXPECT_THROW(v2.get(), InvalidStateException);
    }
}

TEST_F(TestOptional, testMoveAssignemnt) {

    Optional<String> v1;
    EXPECT_TRUE(v1.isNone());

    v1 = moveOptionalString(makeString("hello"));
    EXPECT_TRUE(v1.isSome());
    EXPECT_EQ(StringLiteral("hello"), v1.get());


	auto maybeString = makeString("something different");
	v1 = maybeString ? maybeString.moveResult() : String{};
    EXPECT_TRUE(v1.isSome());
    EXPECT_EQ(StringLiteral("something different"), v1.get());
}

TEST_F(TestOptional, testEqualStrings) {
    const Optional<String> v1(moveOptionalString(makeString("hello")));
    const Optional<String> v3(moveOptionalString(makeString("hello")));

    EXPECT_EQ(v1, v3);
}


TEST_F(TestOptional, testEquals) {
	const Optional<SimpleType> o1{SimpleType{1, 2, 3}};
	const Optional<SimpleType> o2{SimpleType{3, 1, 1}};
	const Optional<SimpleType> o3{SimpleType{1, 2, 3}};

    EXPECT_TRUE(none == none);
    EXPECT_TRUE(Optional<SimpleType>{} == none);
    EXPECT_TRUE(none == Optional<SimpleType>{});

	// Value equal
    EXPECT_TRUE(o1 == o1);  // Self equality
    EXPECT_TRUE(o1 == o3);  // A == B
    EXPECT_TRUE(o3 == o1);  // B == A

	// False assertions:
	EXPECT_FALSE(o1 == o2);
	EXPECT_FALSE(o2 == o1);

	EXPECT_FALSE(o1 == none);
	EXPECT_FALSE(none == o1);
	EXPECT_FALSE(o1 == Optional<SimpleType>{});
	EXPECT_FALSE(Optional<SimpleType>{} == o1);

	// Not-equal
	EXPECT_TRUE(o1 != o2);  // A != B
	EXPECT_TRUE(o2 != o1);  // B != A

	EXPECT_TRUE(o1 != none);
	EXPECT_TRUE(none != o1);
    EXPECT_TRUE(o1 != Optional<SimpleType>{});
	EXPECT_TRUE(Optional<SimpleType>{} != o1);

	// False assertions:
	EXPECT_FALSE(o1 != o1);  // A == A
	EXPECT_FALSE(o1 != o3);  // A == B
	EXPECT_FALSE(o3 != o1);  // B == A
}


TEST_F(TestOptional, testEqualsValues) {
	EXPECT_EQ(Optional<int>(in_place, 4), 4);
	EXPECT_NE(Optional<int>(in_place, 3), 4);

	EXPECT_EQ(33, Optional<int>(in_place, 33));
	EXPECT_NE(18, Optional<int>(in_place, 27));

	EXPECT_TRUE(Optional<SimpleType>(in_place, 1, 2, 3) == SimpleType(1, 2, 3));
	EXPECT_TRUE(Optional<SimpleType>(in_place, 3, 1, 2) != SimpleType(1, 2, 3));

	EXPECT_TRUE(SimpleType(3, 1, 2) == Optional<SimpleType>(in_place, 3, 1, 2));
	EXPECT_TRUE(SimpleType(2, 3, 9) != Optional<SimpleType>(in_place, 2, 3, 3));
}


TEST_F(TestOptional, testEmpty) {
    auto v1 = Optional<int>{};

    EXPECT_TRUE(v1.isNone());
    EXPECT_TRUE(!v1.isSome());

    {
        auto const v = Optional<SimpleType>{};
        EXPECT_EQ(0, SimpleType::InstanceCount);

        EXPECT_TRUE(v.isNone());
        EXPECT_TRUE(!v.isSome());
        EXPECT_THROW(v.get(), InvalidStateException);
    }
    EXPECT_EQ(0, SimpleType::InstanceCount);

    {
        auto v = Optional<MoveOnlyType>{};
        EXPECT_EQ(0, MoveOnlyType::InstanceCount);

        EXPECT_TRUE(v.isNone());
        EXPECT_TRUE(!v.isSome());
        EXPECT_THROW(v.get(), InvalidStateException);
    }
    EXPECT_EQ(0, MoveOnlyType::InstanceCount);
}

TEST_F(TestOptional, testString) {
	auto maybeString = makeString("hello-xyz");
	Optional<String> v1{maybeString.moveResult()};

    EXPECT_TRUE(v1.isSome());
    EXPECT_TRUE(!v1.isNone());

    EXPECT_EQ(StringLiteral("hello-xyz"), v1.get());
}

TEST_F(TestOptional, testGetFromNoneRaises) {
    EXPECT_THROW(Optional<String>{}.get(), InvalidStateException);
}

TEST_F(TestOptional, testOrElse) {
    const SimpleType test(2, 0.0f, 2);
    const SimpleType testElse(321, -1, 5);

    auto v1 = Optional<SimpleType>(test);
    auto v2 = Optional<SimpleType>{};

    EXPECT_TRUE(v1.isSome());
    EXPECT_TRUE(v2.isNone());

    EXPECT_EQ(test, v1.orElse(testElse));
    EXPECT_EQ(testElse, v2.orElse(testElse));
}


TEST_F(TestOptional, testOrElseOperator) {
	auto noneValue = Optional<SimpleType>{};
	SimpleType value{7762, 2, -21};
	SimpleType testElse{321, -1, 5};

	{
		SimpleType otherV = noneValue || testElse;
		EXPECT_EQ(otherV, testElse);
	}

	EXPECT_EQ(Optional<SimpleType>{} || testElse, testElse);
	EXPECT_EQ(Optional<SimpleType>{value} || testElse, value);
}


static int my_atoi(const char* c) {
    return atoi(c);
}

TEST_F(TestOptional, testMap) {
    SimpleType test(32, 24, -3212);

    auto f = [&test](const int& content) -> SimpleType {
        return SimpleType(test.x * content, test.y, test.z);
    };

    // Test mapping using lambda
    auto const v1 = Optional<int>(18)
                                    .map(f);
    EXPECT_TRUE(v1.isSome());
    EXPECT_EQ(SimpleType(test.x * 18, test.y, test.z), v1.get());

    auto const v2 = Optional<SimpleType>{}
                                    .map([](SimpleType const& value) {
                                        return value.x;
                                    });

    EXPECT_TRUE(v2.isNone());
    EXPECT_EQ(48, v2.orElse(48));

    // Test mapping using regular C-function
    auto const meaningOfLife = Optional<const char*>("42")
                                    .map(my_atoi);
    EXPECT_TRUE(meaningOfLife.isSome());
    EXPECT_EQ(42, meaningOfLife.get());

    {
        auto v = Optional<MoveOnlyType>(MoveOnlyType(1024));
        auto const d = v.map([] (MoveOnlyType& q) {
            return q.x_ / 2;
        });

        EXPECT_TRUE(v.isSome());
        EXPECT_TRUE(d.isSome());
        EXPECT_EQ(1024, v.get().x_);
        EXPECT_EQ(512, d.get());
        EXPECT_EQ(1, MoveOnlyType::InstanceCount);
    }
}


PimitiveType mapSimpleToPrimitive(SimpleType t) noexcept {
	return {t.x*2};
}

TEST_F(TestOptional, mapWithFreeFunction) {
	auto o = Optional<SimpleType>{in_place, 128, 3, 2};

	auto mapped = o.map(mapSimpleToPrimitive);
	EXPECT_TRUE(mapped.isSome());
	EXPECT_EQ(256, (*mapped).value);
}

TEST_F(TestOptional, testFlatMap) {
    SimpleType test(32, 72, -312);

    auto f = [](const SimpleType& content) {
        return Optional<int>(content.x * 2);
    };

    auto const v1 = Optional<SimpleType>(test)
            .flatMap(f);

    EXPECT_TRUE(v1.isSome());
    EXPECT_EQ(test.x*2, v1.get());

    auto const v2 = Optional<SimpleType>{}
            .flatMap(f);

    EXPECT_TRUE(v2.isNone());
    EXPECT_EQ(42, v2.orElse(42));
}

TEST_F(TestOptional, testFilter) {
    EXPECT_TRUE(Optional<int>(4412)
                    .filter([](int t) { return t > 20;})
                    .isSome());

    EXPECT_TRUE(Optional<int>{}
                    .filter([](int t) { return t > 0;})
                    .isNone());

    EXPECT_TRUE(Optional<SimpleType>{}
					.filter([](SimpleType const& t) { return t.x != 0;})
                    .isNone());

	EXPECT_TRUE(Optional<SimpleType>(in_place, 32, 72, -858)
					.filter([](SimpleType const& t) { return t.x >= 0;})
                    .isSome());

    EXPECT_TRUE(Optional<MoveOnlyType>(in_place, 32)
					.filter([](MoveOnlyType const& t) { return t.x_ != 0;})
                    .isSome());
}

TEST_F(TestOptional, testMoveOnlyResult) {
    EXPECT_TRUE(Optional<MoveOnlyType>{}.isNone());

    EXPECT_EQ(0, MoveOnlyType::InstanceCount);
    auto mover = [] (int v) {
        return MoveOnlyType(v);
    };

    auto r = Optional<MoveOnlyType>(mover(321));

    EXPECT_TRUE(r.isSome());
    EXPECT_EQ(321, r.get().x_);
    EXPECT_EQ(1, MoveOnlyType::InstanceCount);
}

TEST_F(TestOptional, testMoveOnlyMapper) {
	auto r = Optional<MoveOnlyType>{in_place, 32};

	/* FIXME: This is broken as map() does not support functors that move value out. */
	auto op = r.map([](MoveOnlyType& m) {
			return m.x_ * 2;
	});

	EXPECT_TRUE(op.isSome());
	EXPECT_EQ(64, op.get());

	EXPECT_EQ(1, MoveOnlyType::InstanceCount);
}


TEST_F(TestOptional, testMoveOnlyMove) {
	auto r = Optional<MoveOnlyType>{in_place, 732};
	auto p = [](Optional<MoveOnlyType>& value) { return value.move(); } (r);

    EXPECT_EQ(2, MoveOnlyType::InstanceCount);
    EXPECT_EQ(732, p.x_);

	// R is moved-out of
	EXPECT_EQ(0, r.get().x_);
	EXPECT_EQ(0, r.orElse(MoveOnlyType(-9876)).x_);
}


TEST_F(TestOptional, optionalPointer) {
    struct Dummy {
        virtual ~Dummy() noexcept = default;

        Dummy() = default;
        Dummy(Dummy const&) = delete;
        Dummy(Dummy&&) = default;
        Dummy& operator= (Dummy const&) = delete;
        Dummy& operator= (Dummy&&) = default;

        virtual StringView getSome() const noexcept = 0;
    };

    auto getMaybe = [](uint i) noexcept -> Optional<Dummy*> {
        return (i < 12)
                ? Optional<Dummy*>{}
                : Optional<Dummy*>{nullptr};
    };


	auto const r = Optional<MoveOnlyType*>{nullptr};

    EXPECT_EQ(0, MoveOnlyType::InstanceCount);
    EXPECT_TRUE(r.isSome());

    auto const r2 = getMaybe(0);
    EXPECT_EQ(0, MoveOnlyType::InstanceCount);
    EXPECT_TRUE(r2.isNone());
}


TEST_F(TestOptional, refTypes) {
	auto const maybeNonRef = Optional<MoveOnlyType&>{};
	ASSERT_TRUE(maybeNonRef.isNone());
	EXPECT_EQ(0, MoveOnlyType::InstanceCount);

	MoveOnlyType value{3213};
	auto const maybeRef = Optional<MoveOnlyType&>{value};
	ASSERT_TRUE(maybeNonRef.isNone());

	MoveOnlyType const& valueRef = *maybeRef;
	EXPECT_EQ(3213, valueRef.x_);
	EXPECT_EQ(1, MoveOnlyType::InstanceCount);
}
