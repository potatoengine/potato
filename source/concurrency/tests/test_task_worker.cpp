#include "grimm/concurrency/task_worker.h"
#include "doctest.h"
#include <thread>

DOCTEST_TEST_SUITE("[grimm][concurrency] TaskWorker") {
    using namespace gm;
    using namespace gm::concurrency;

    DOCTEST_TEST_CASE("single") {
        TaskQueue queue;
        TaskWorker worker1(queue, "Test Worker 1");
        TaskWorker worker2(queue, "Test Worker 2");

        int values[1024] = {};
        for (int i = 0; i != 1024; ++i) {
            queue.enqueWait([&values, i] { values[i] = i; });
        }

        queue.close();
        worker1.join();
        worker2.join();

        for (int i = 0; i != 1024; ++i) {
            DOCTEST_CHECK_EQ(values[i], i);
        }
    }
}
