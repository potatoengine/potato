#include "grimm/concurrency/semaphore.h"
#include "doctest.h"
#include <thread>

DOCTEST_TEST_SUITE("[grimm][concurrency] Semaphore") {
    using namespace gm;

    DOCTEST_TEST_CASE("default") {
        Semaphore sem(0);

        sem.signal();
        sem.wait();
    }

    DOCTEST_TEST_CASE("thread") {
        Semaphore in(0);
        Semaphore out(0);

        auto thread = std::thread([&] {
            in.wait();
            out.signal();
        });

        in.signal();
        out.wait();

        thread.join();
    }
}
