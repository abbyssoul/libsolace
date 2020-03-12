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
#pragma once
#ifndef SOLACE_MOCKTYPES_HPP
#define SOLACE_MOCKTYPES_HPP

#include <solace/array.hpp>
#include <solace/exception.hpp>

#include <iostream>



struct PimitiveType {

    static int InstanceCount;

	int value{};

    ~PimitiveType() noexcept {
        --InstanceCount;
    }

    constexpr PimitiveType() noexcept = default;

    PimitiveType(int x_) noexcept
		: value{x_}
    {
        ++InstanceCount;
    }

    PimitiveType(PimitiveType const& rhs) noexcept
		: value{rhs.value}
    {
        ++InstanceCount;
    }

    PimitiveType(PimitiveType&& rhs) noexcept
		: value{rhs.value}
    {
        ++InstanceCount;

        swap(rhs);
    }

    PimitiveType& operator= (PimitiveType const & rhs) noexcept = default;
    PimitiveType& operator= (PimitiveType&& rhs) noexcept = default;

    PimitiveType& swap(PimitiveType& rhs) noexcept {
        using std::swap;

		swap(value, rhs.value);

        return *this;
    }

    friend bool operator == (PimitiveType const& a, PimitiveType const& b) noexcept {
		return (a.value == b.value);
    }

};




struct SimpleType {

    static int InstanceCount;

    int x{}, y{}, z{};

	~SimpleType() noexcept {
        --InstanceCount;
    }

    SimpleType() noexcept {
        ++InstanceCount;
    }

	SimpleType(int x_, int y_, int z_) noexcept
		: x{x_}
		, y{y_}
		, z{z_}
    {
        ++InstanceCount;
    }

	SimpleType(SimpleType const& rhs) noexcept
		: x{rhs.x}
		, y{rhs.y}
		, z{rhs.z}
    {
        ++InstanceCount;
    }

    SimpleType(SimpleType&& rhs) noexcept {
        ++InstanceCount;

        swap(rhs);
    }

	SimpleType& operator= (SimpleType const& rhs) noexcept {
		SimpleType copy{rhs};
		return swap(copy);
    }

	SimpleType& operator= (SimpleType&& rhs) noexcept {
        return swap(rhs);
    }

    SimpleType& swap(SimpleType& rhs) noexcept {
        using std::swap;

        swap(x, rhs.x);
        swap(y, rhs.y);
        swap(z, rhs.z);

        return *this;
    }

	friend bool operator== (SimpleType const& a, SimpleType const& b) noexcept {
        return ((a.x == b.x) && (a.y == b.y) && (a.z == b.z));
    }

	friend bool operator!= (SimpleType const& a, SimpleType const& b) noexcept {
        return !(a == b);
    }
};



struct MoveOnlyType {
    static int InstanceCount;

	~MoveOnlyType() noexcept {
        --InstanceCount;
    }

	MoveOnlyType(int x) noexcept
		: x_{x}
	{
        ++InstanceCount;
    }

    MoveOnlyType(const MoveOnlyType& rhs) = delete;
    MoveOnlyType& operator= (const MoveOnlyType& rhs) = delete;

	MoveOnlyType(MoveOnlyType&& rhs)
		: x_{Solace::exchange(rhs.x_, 0)}
	{
        ++InstanceCount;
    }

    MoveOnlyType& operator= (MoveOnlyType&& rhs) {
        x_ = rhs.x_;
        rhs.x_ = -1;

        return (*this);
    }

    int x_;
};


struct SometimesConstructable {
    static int InstanceCount;
    static int BlowUpEveryInstance;

    int someValue;

    ~SometimesConstructable() {
        --InstanceCount;
    }

    static void maybeBlowup() {
        if (BlowUpEveryInstance > 0 && ((InstanceCount + 1) % BlowUpEveryInstance) == 0) {
            throw Solace::Exception("Blowing up on purpose");
        }
    }

    SometimesConstructable() : SometimesConstructable(3)
    {}

    SometimesConstructable(int value) : someValue(value) {
        maybeBlowup();

        ++InstanceCount;
    }

    SometimesConstructable(SometimesConstructable const& rhs): someValue(rhs.someValue) {
        maybeBlowup();
        ++InstanceCount;
    }

    SometimesConstructable(SometimesConstructable&& rhs) : someValue(rhs.someValue) {
        maybeBlowup();
        ++InstanceCount;
    }

    SometimesConstructable& operator= (SometimesConstructable const& rhs) {
        someValue = rhs.someValue;

        return *this;
    }

    SometimesConstructable& operator= (SometimesConstructable&& rhs) {
        someValue = rhs.someValue;

        return *this;
    }

    bool operator == (SometimesConstructable const& rhs) const {
        return (someValue == rhs.someValue);
    }
};

std::ostream& operator<< (std::ostream& ostr, const SimpleType& t);


template <typename T>
std::ostream& operator<< (std::ostream& ostr, const Solace::Array<T>& a) {
    ostr << '[';

    for (typename Solace::Array<T>::size_type end = a.size(), i = 0; i < end; ++i) {
        ostr << a[i];
        if (i < end - 1) {
            ostr << ',' << ' ';
        }
    }

    ostr << ']';

    return ostr;
}

#endif  // SOLACE_MOCKTYPES_HPP
