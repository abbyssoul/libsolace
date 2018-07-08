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
 * @file: test/test_memoryBuffer.cpp
 * @author: soultaker
*******************************************************************************/
#include <solace/memoryBuffer.hpp>  // Class being tested

#include <cppunit/extensions/HelperMacros.h>


using namespace Solace;


class TestMemoryBuffer :
        public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestMemoryBuffer);
        CPPUNIT_TEST(moveAssignment);
    CPPUNIT_TEST_SUITE_END();

protected:

    class MockDisposer : public MemoryViewDisposer {
    public:
        MockDisposer(uint count) :
            _count(count)
        {}


        void dispose(ImmutableMemoryView* SOLACE_UNUSED(view)) const override {
            // NO-OP
            --_count;
        }

        uint count() const noexcept { return _count; }
    private:
        mutable uint _count{};
    };

public:

    void moveAssignment() {
        byte fakes[32];

        auto disposer = MockDisposer(1);
        auto buff = MemoryBuffer(wrapMemory(fakes), &disposer);
        CPPUNIT_ASSERT_EQUAL(1u, disposer.count());

        {
            MemoryBuffer otherBuff = std::move(buff);
            CPPUNIT_ASSERT_EQUAL(1u, disposer.count());
        }


        CPPUNIT_ASSERT_EQUAL(0u, disposer.count());
        CPPUNIT_ASSERT(buff.empty());
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMemoryBuffer);
