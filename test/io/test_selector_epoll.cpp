#include <solace/io/selector.hpp>  // Class being tested

#include <solace/io/pipe.hpp>
#include <solace/exception.hpp>

#include <gtest/gtest.h>

using namespace Solace;
using namespace Solace::IO;


#ifdef SOLACE_PLATFORM_LINUX

class TestEPollSelector : public ::testing::Test {

public:

    void setUp() {
	}

    void tearDown() {
	}
};

TEST_F(TestEPollSelector, testSubscription) {
    Pipe p;

    auto s = Selector::createEPoll(2);
    s.add(&p.getReadEnd(), Selector::Read);
    s.add(&p.getWriteEnd(), Selector::Write);

    auto i = s.poll(1);
    EXPECT_TRUE(i != i.end());
    EXPECT_EQ(static_cast<uint>(1), i.size());

    auto ev = *i;
//        EXPECT_EQ(static_cast<void*>(&p.getWriteEnd()), ev.data);
    EXPECT_EQ(p.getWriteEnd().getSelectId(), ev.fd);
}


TEST_F(TestEPollSelector, testReadPolling) {
    Pipe p;

    auto s = Selector::createEPoll(1);
    s.add(&p.getReadEnd(), Selector::Read);

    auto i = s.poll(1);

    // Test that poll times out correctly as nothing has been written so far.
    EXPECT_TRUE(i == i.end());

    char msg[] = "message";
    const auto written = p.write(wrapMemory(msg));
    EXPECT_TRUE(written.isOk());

    i = s.poll(1);
    EXPECT_TRUE(i != i.end());

    auto ev = *i;
//        EXPECT_EQ(static_cast<void*>(&p.getReadEnd()), ev.data);
    EXPECT_EQ(p.getReadEnd().getSelectId(), ev.fd);

    char buff[100];
    auto m = wrapMemory(buff);
    auto dest = m.slice(0, written.unwrap());
    const auto bytesRead = p.read(dest);
    EXPECT_TRUE(bytesRead.isOk());
    EXPECT_EQ(written.unwrap(), bytesRead.unwrap());

    // There is no more data in the pipe so the next poll must time-out
    i = s.poll(1);

    // Test that poll times out correctly as nothing has been written so far.
    EXPECT_TRUE(i == i.end());
}


TEST_F(TestEPollSelector, testEmptyPolling) {
    Selector s = Selector::createEPoll(3);

    auto i = s.poll(1);
    EXPECT_TRUE(!(i != i.end()));
    EXPECT_EQ(static_cast<uint>(0), i.size());
    EXPECT_THROW(++i, IndexOutOfRangeException);
}

TEST_F(TestEPollSelector, testRemoval) {
    Pipe p;

    Selector s = Selector::createEPoll(5);
    s.add(&p.getReadEnd(), Selector::Read);
    s.add(&p.getWriteEnd(), Selector::Write);

    {
        auto i = s.poll(1);
        EXPECT_TRUE(i != i.end());
        EXPECT_EQ(static_cast<uint>(1), i.size());

        auto ev = *i;
        EXPECT_EQ(p.getWriteEnd().getSelectId(), ev.fd);
    }

    {
        s.remove(&p.getWriteEnd());
        auto i = s.poll(1);
        EXPECT_TRUE(i == i.end());
        EXPECT_EQ(static_cast<uint>(0), i.size());
    }
}

TEST_F(TestEPollSelector, testRemovalOfNotAddedItem) {
    Pipe p;

    auto s = Selector::createEPoll(5);
    EXPECT_NO_THROW(s.remove(&p.getReadEnd()));
    EXPECT_NO_THROW(s.remove(&p.getWriteEnd()));
}

#endif  // SOLACE_PLATFORM_LINUX
