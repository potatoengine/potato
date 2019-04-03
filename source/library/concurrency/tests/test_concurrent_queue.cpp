#include "potato/concurrency/concurrent_queue.h"
#include <doctest/doctest.h>
#include <thread>

DOCTEST_TEST_SUITE("[potato][concurrency] ConcurrentQueue") {
    using namespace up;
    using namespace up::concurrency;

    DOCTEST_TEST_CASE("default") {
        ConcurrentQueue<int> queue;
    }

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

        DOCTEST_CHECK_EQ(last, 1023);
    }
}
