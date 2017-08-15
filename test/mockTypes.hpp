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

#include <utility>
#include <iostream>

struct PimitiveType {

    static int InstanceCount;

    int x;

    ~PimitiveType() {
        --InstanceCount;
    }

    PimitiveType() noexcept : x() {}

    PimitiveType(int x_) noexcept :
        x(x_)
    {
        ++InstanceCount;
    }

    PimitiveType(const PimitiveType& rhs) noexcept :
        x(rhs.x)
    {
        ++InstanceCount;
    }

    PimitiveType(PimitiveType&& rhs): PimitiveType() {
        ++InstanceCount;

        swap(rhs);
    }

    PimitiveType& operator= (const PimitiveType& rhs) {
        x = rhs.x;

        return (*this);
    }

    PimitiveType& operator= (PimitiveType&& rhs) {
        return swap(rhs);
    }

    PimitiveType& swap(PimitiveType& rhs) noexcept {
        using std::swap;

        swap(x, rhs.x);

        return *this;
    }

    friend bool operator == (const PimitiveType& a, const PimitiveType& b) {
        return (a.x == b.x);
    }

};


struct SimpleType {

    static int InstanceCount;

    int x, y, z;

    ~SimpleType() {
        --InstanceCount;
    }

    SimpleType() noexcept :
        x(),
        y(),
        z()
    {}

    SimpleType(int x_, int y_, int z_) noexcept :
        x(x_),
        y(y_),
        z(z_)
    {
        ++InstanceCount;
    }

    SimpleType(const SimpleType& rhs) noexcept :
        x(rhs.x),
        y(rhs.y),
        z(rhs.z)
    {
        ++InstanceCount;
    }

    SimpleType(SimpleType&& rhs): SimpleType() {
        ++InstanceCount;

        swap(rhs);
    }

    SimpleType& operator= (const SimpleType& rhs) {
        x = rhs.x;
        y = rhs.y;
        z = rhs.z;

        return (*this);
    }

    SimpleType& operator= (SimpleType&& rhs) {
        return swap(rhs);
    }

    SimpleType& swap(SimpleType& rhs) noexcept {
        using std::swap;

        swap(x, rhs.x);
        swap(y, rhs.y);
        swap(z, rhs.z);

        return *this;
    }

    friend bool operator == (const SimpleType& a, const SimpleType& b) {
        return ((a.x == b.x) && (a.y == b.y) && (a.z == b.z));
    }

};


struct MoveOnlyType {
    static int InstanceCount;

    ~MoveOnlyType() {
        --InstanceCount;
    }

    MoveOnlyType(int x): x_(x) {
        ++InstanceCount;
    }

    MoveOnlyType(const MoveOnlyType& rhs) = delete;
    MoveOnlyType& operator= (const MoveOnlyType& rhs) = delete;

    MoveOnlyType(MoveOnlyType&& rhs): x_(rhs.x_) {
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

    SometimesConstructable(): someValue(3) {
        if ((InstanceCount + 1) % BlowUpEveryInstance) {
            throw Solace::Exception("Blowing up on purpose");
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
