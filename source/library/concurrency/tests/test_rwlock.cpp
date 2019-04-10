#include "potato/concurrency/rwlock.h"
#include <doctest/doctest.h>
#include <thread>

DOCTEST_TEST_SUITE("[potato][concurrency] RWLock") {
    using namespace up;
    using namespace up::concurrency;

    DOCTEST_TEST_CASE("writer-reader-writer") {
        RWLock lock;

        lock.writer().lock();
        DOCTEST_CHECK_FALSE(lock.writer().tryLock());
        DOCTEST_CHECK_FALSE(lock.reader().tryLock());
        lock.writer().unlock();

        lock.reader().lock();
        DOCTEST_CHECK_FALSE(lock.writer().tryLock());
        lock.reader().lock();
        DOCTEST_CHECK_FALSE(lock.writer().tryLock());
        lock.reader().unlock();
        DOCTEST_CHECK_FALSE(lock.writer().tryLock());
        lock.reader().unlock();

        lock.writer().lock();
        DOCTEST_CHECK_FALSE(lock.writer().tryLock());
        DOCTEST_CHECK_FALSE(lock.reader().tryLock());
        lock.writer().unlock();
    }
}
