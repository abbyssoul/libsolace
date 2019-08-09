/*
*  Copyright 2018 Ivan Ryabov
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
 * @file: test/test_memoryResource.cpp
*******************************************************************************/
#include <solace/memoryResource.hpp>  // Class being tested

#include <gtest/gtest.h>

using namespace Solace;

class TestMemoryResource : public ::testing::Test  {

protected:

    class MockDisposer : public MemoryResource::Disposer {
    public:
        MockDisposer(uint count) :
            _count(count)
        {}


        void dispose(MemoryView* SOLACE_UNUSED(view)) const override {
            // NO-OP
            --_count;
        }

        uint count() const noexcept { return _count; }
    private:
        mutable uint _count{};
    };
};

TEST_F(TestMemoryResource, moveAssignment) {
    byte fakes[32];

    auto disposer = MockDisposer(1);
    auto buff = MemoryResource(wrapMemory(fakes), &disposer);
    EXPECT_EQ(1u, disposer.count());

    {
        MemoryResource otherBuff = std::move(buff);
        EXPECT_EQ(1u, disposer.count());
    }


    EXPECT_EQ(0u, disposer.count());
    EXPECT_TRUE(buff.empty());
}
