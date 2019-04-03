#include "potato/concurrency/semaphore.h"
#include <doctest/doctest.h>
#include <thread>

DOCTEST_TEST_SUITE("[potato][concurrency] Semaphore") {
    using namespace up;
    using namespace up::concurrency;

    DOCTEST_TEST_CASE("default") {
        Semaphore sem(0);

        sem.signal();
        sem.wait();
    }

    DOCTEST_TEST_CASE("initial") {
        Semaphore sem(2);

        DOCTEST_CHECK(sem.tryWait());
        DOCTEST_CHECK(sem.tryWait());
    }

    DOCTEST_TEST_CASE("thread") {
        Semaphore in(0);
        Semaphore out(0);

        auto thread = std::thread([&] {
            in.wait();
            out.signal();
        });

        // try to catch the thread skipping its first wait;
        // this might give a false-negative, but I'm okay
        // with that since it'll catch eventually (probably)
        std::this_thread::yield();
        DOCTEST_CHECK(!out.tryWait());

        in.signal();
        out.wait();

        thread.join();
    }
}
