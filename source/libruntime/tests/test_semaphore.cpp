// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/semaphore.h"

#include <catch2/catch.hpp>
#include <thread>

TEST_CASE("Semaphore", "[potato][runtime]") {
    using namespace up;

    SECTION("default") {
        Semaphore sem(0);

        sem.signal();
        sem.wait();
    }

    SECTION("initial") {
        Semaphore sem(2);

        CHECK(sem.tryWait());
        CHECK(sem.tryWait());
    }

    SECTION("thread") {
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
        CHECK_FALSE(out.tryWait());

        in.signal();
        out.wait();

        thread.join();
    }
}
