// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/thread_util.h"

#include <doctest/doctest.h>
#include <thread>

DOCTEST_TEST_SUITE("[potato][runtime] thread_util") {
    using namespace up;

    DOCTEST_TEST_CASE("setCurrentThreadName") {
        // basically just making sure we're not crashing
        auto thread = std::thread([] { setCurrentThreadName("test thread"); });

        thread.join();
    }

    DOCTEST_TEST_CASE("currentThreadSmallId") {
        DOCTEST_CHECK_EQ(currentSmallThreadId(), 0);

        // Yes, this could be a lambda, but I'm seeing a bad miscompilation bug in MSVC 16.6 Preview 5
        // when using lambdas here. These threads end up invoking the lambdas in test_concurrent_queue,
        // and vice versa that file invokes these lambdas. Naturally, the captured state and environment
        // are totally wrong and at best we get assert failures and at worst we get crashes.
        //
        struct Task {
            SmallThreadId& id;
            Task(SmallThreadId& i) : id(i) {}
            void operator()() const { DOCTEST_CHECK_NE(id = currentSmallThreadId(), 0); }
        };

        // ensure it doesn't change
        DOCTEST_CHECK_EQ(currentSmallThreadId(), 0);

        SmallThreadId id1;
        Task t1(id1);
        auto thread = std::thread(t1);
        thread.join();

        SmallThreadId id2;
        Task t2(id2);
        thread = std::thread(t2);
        thread.join();

        DOCTEST_CHECK_NE(id1, id2);

        // other threads shouldn't have changed
        DOCTEST_CHECK_EQ(currentSmallThreadId(), 0);
    }
}
