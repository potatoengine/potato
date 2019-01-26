#include "grimm/concurrency/thread_util.h"
#include "doctest.h"
#include <thread>

DOCTEST_TEST_SUITE("[grimm][concurrency] thread_util") {
    using namespace gm;

    DOCTEST_TEST_CASE("setCurrentThreadName") {
        // basically just making sure we're not crashing
        auto thread = std::thread([] {
            setCurrentThreadName("test thread");
        });

        thread.join();
    }

    DOCTEST_TEST_CASE("currentThreadSmallId") {
        DOCTEST_CHECK_EQ(currentSmallThreadId(), 0);

        // ensure it doesn't change
        DOCTEST_CHECK_EQ(currentSmallThreadId(), 0);

        SmallThreadId id1;
        auto thread = std::thread([&id1] {
            DOCTEST_CHECK_NE(id1 = currentSmallThreadId(), 0);
        });
        thread.join();

        SmallThreadId id2;
        thread = std::thread([&id2] {
            DOCTEST_CHECK_NE(id2 = currentSmallThreadId(), 0);
        });
        thread.join();

        DOCTEST_CHECK_NE(id1, id2);

        // other threads shouldn't have changed
        DOCTEST_CHECK_EQ(currentSmallThreadId(), 0);
    }
}
