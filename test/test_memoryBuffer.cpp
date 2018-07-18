#include <solace/memoryBuffer.hpp>  // Class being tested

#include <gtest/gtest.h>

using namespace Solace;

class TestMemoryBuffer : public ::testing::Test  {

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

    void setUp() {
	}

    void tearDown() {
	}
};

TEST_F(TestMemoryBuffer, moveAssignment) {
    byte fakes[32];

    auto disposer = MockDisposer(1);
    auto buff = MemoryBuffer(wrapMemory(fakes), &disposer);
    EXPECT_EQ(1u, disposer.count());

    {
        MemoryBuffer otherBuff = std::move(buff);
        EXPECT_EQ(1u, disposer.count());
    }


    EXPECT_EQ(0u, disposer.count());
    EXPECT_TRUE(buff.empty());
}