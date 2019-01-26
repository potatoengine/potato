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

        auto thread = std::thread([] {
            DOCTEST_CHECK_EQ(currentSmallThreadId(), 1);
        });
        thread.join();

        thread = std::thread([] {
            DOCTEST_CHECK_EQ(currentSmallThreadId(), 2);
        });
        thread.join();

        // other threads shouldn't have changed
        DOCTEST_CHECK_EQ(currentSmallThreadId(), 0);
    }
}
