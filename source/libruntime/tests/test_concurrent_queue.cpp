#include "potato/runtime/concurrent_queue.h"

#include <doctest/doctest.h>

#include <thread>

DOCTEST_TEST_SUITE("[potato][runtime] ConcurrentQueue") {
    using namespace up;

    DOCTEST_TEST_CASE("default") { ConcurrentQueue<int> queue; }

    DOCTEST_TEST_CASE("thread") {
        ConcurrentQueue<int> queue;

        int last = 0;
        auto consumer = std::thread([&] {
            int value = 0;
            while (queue.dequeWait(value)) {
                last = value;
            }
        });

        auto producer = std::thread([&] {
            for (int i = 0; i != 1024; ++i) {
                queue.enqueWait(i);
            }
            queue.close();
        });
        producer.detach();

        consumer.join();

        DOCTEST_REQUIRE(queue.isClosed());

        DOCTEST_CHECK_EQ(last, 1023);
    }
}
