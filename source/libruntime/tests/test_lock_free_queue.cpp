// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/lock_free_queue.h"

#include <doctest/doctest.h>
#include <thread>

DOCTEST_TEST_SUITE("[potato][runtime] LockFreeQueue") {
    using namespace up;

    DOCTEST_TEST_CASE("default") { LockFreeQueue<int> queue; }

    DOCTEST_TEST_CASE("fill") {
        LockFreeQueue<std::size_t> queue;

        for (std::size_t i = 0; i < queue.capacity(); ++i) {
            DOCTEST_CHECK(queue.tryEnque(i));
        }

        DOCTEST_CHECK(!queue.tryEnque(0));
    }

    DOCTEST_TEST_CASE("sequential") {
        LockFreeQueue<std::size_t> queue;

        for (std::size_t i = 0; i < queue.capacity(); ++i) {
            DOCTEST_CHECK(queue.tryEnque(i));
        }

        DOCTEST_CHECK(!queue.tryEnque(0));

        for (std::size_t i = 0; i < queue.capacity(); ++i) {
            std::size_t result = 0;
            DOCTEST_CHECK(queue.tryDeque(result));
            DOCTEST_CHECK_EQ(i, result);
        }

        std::size_t empty;
        DOCTEST_CHECK(!queue.tryDeque(empty));
    }

    DOCTEST_TEST_CASE("thread") {
        LockFreeQueue<std::size_t> queue;

        std::size_t total1 = 0;
        auto thread1 = std::thread([&] {
            for (;;) {
                std::size_t count;
                if (queue.tryDeque(count)) {
                    if (count == 0) {
                        break;
                    }
                    total1 += count;
                }
            }
        });

        std::size_t total2 = 0;
        auto thread2 = std::thread([&] {
            for (;;) {
                std::size_t count;
                if (queue.tryDeque(count)) {
                    if (count == 0) {
                        break;
                    }
                    total2 += count;
                }
            }
        });

        std::size_t expected = 0;
        for (std::size_t i = 2; i < queue.capacity(); ++i) {
            DOCTEST_CHECK(queue.tryEnque(i));
            expected += i;
        }

        // signals end to threads
        DOCTEST_CHECK(queue.tryEnque(0));
        DOCTEST_CHECK(queue.tryEnque(0));

        thread1.join();
        thread2.join();

        DOCTEST_CHECK_EQ(total1 + total2, expected);
    }
}
