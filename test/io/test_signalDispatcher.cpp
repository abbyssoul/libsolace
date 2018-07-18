#include <solace/io/signalDispatcher.hpp>  // Class being tested

#include <gtest/gtest.h>

#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

using namespace Solace;
using namespace Solace::IO;

class TestIOSignalDispatcher : public ::testing::Test {

public:

    void setUp() {
	}

    void tearDown() {
	}
};

TEST_F(TestIOSignalDispatcher, testSubscription) {
    bool signaled = false;

    SignalDispatcher::getInstance().attachHandler(SIGALRM, [&signaled](int signalId) {
        signaled = (signalId == SIGALRM);
    });

    // Generate ALARM signal, in 1 sec
    itimerval timeToSleep;
    timeToSleep.it_interval.tv_sec = 0;
    timeToSleep.it_interval.tv_usec = 0;
    timeToSleep.it_value.tv_sec = 0;
    timeToSleep.it_value.tv_usec = 250 * 1000;
    EXPECT_EQ(0, setitimer(ITIMER_REAL, &timeToSleep, nullptr));

    usleep(400 * 1000);
    EXPECT_TRUE(signaled);

    // Reset
    signaled = false;
    int count = 0;

    SignalDispatcher::getInstance().attachHandler(SIGALRM, [&count](int signalId) {
        count += (signalId == SIGALRM) ? 1 : 0;
    });
    SignalDispatcher::getInstance().attachHandler(SIGALRM, [&count](int signalId) {
        count += (signalId == SIGALRM) ? 1 : 0;
    });
    SignalDispatcher::getInstance().attachHandler(SIGALRM, [&count](int signalId) {
        count += (signalId == SIGALRM) ? 1 : 0;
    });

    timeToSleep.it_value.tv_usec = 300 * 1000;
    EXPECT_EQ(0, setitimer(ITIMER_REAL, &timeToSleep, nullptr));

    usleep(600 * 1000);

    EXPECT_TRUE(signaled);
    EXPECT_EQ(3, count);
}