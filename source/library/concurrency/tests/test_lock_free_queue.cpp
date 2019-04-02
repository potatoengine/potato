#include "grimm/concurrency/lock_free_queue.h"
#include "doctest.h"
#include <thread>

DOCTEST_TEST_SUITE("[grimm][concurrency] LockFreeQueue") {
    using namespace gm;
    using namespace gm::concurrency;

    DOCTEST_TEST_CASE("default") {
        LockFreeQueue<int> queue;
    }

    DOCTEST_TEST_CASE("thread") {
        constexpr int size = 1024;
        LockFreeQueue<int, size> queue;

        for (int i = 0; i < size; ++i) {
            DOCTEST_CHECK(queue.tryEnque(i));
        }

        int total1 = 0;
        auto thread1 = std::thread([&] {
            int count;
            while (queue.tryDeque(count)) {
                total1 += count;
            }
        });

        int total2 = 0;
        auto thread2 = std::thread([&] {
            int count;
            while (queue.tryDeque(count)) {
                total2 += count;
            }
        });

        thread1.join();
        thread2.join();

        // sum of [0, size] is sum(0...size-1)
        //  identities:
        //   sum(1...N) = N(N+1)/2
        //   0+N=N
        //  expand identities:
        //   sum(0...N-1) = sum(1...N-1)
        //   sum(1...N-1) = (N-1)(N-1+1)/2
        //  simplify:
        //   (N-1)(N-1+1)/2 = N(N-1)/2
        DOCTEST_CHECK_EQ(total1 + total2, (size * (size - 1)) / 2);
    }
}
