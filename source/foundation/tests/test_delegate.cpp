#include "grimm/foundation/delegate.h"
#include "doctest.h"

namespace {
    struct Test {
        int& r;

        int next() { return ++r; }
        int add(int i) const { return r + i; }
    };
}

DOCTEST_TEST_SUITE("[grimm][foundation] gm::delegate") {
    using namespace gm;

    DOCTEST_TEST_CASE("empty delegate") {
        delegate<int()> d;

        DOCTEST_CHECK(d.empty());

        d.reset();
    }

    DOCTEST_TEST_CASE("lambda delegate") {
        delegate d = +[](int i) { return i * 2; };

        DOCTEST_CHECK(!d.empty());
        DOCTEST_CHECK_EQ(d(0), 0);
        DOCTEST_CHECK_EQ(d(-1), -2);
        DOCTEST_CHECK_EQ(d(10), 20);

        d.reset();

        DOCTEST_CHECK(d.empty());
    }

    DOCTEST_TEST_CASE("object delegate") {
        int i = 1;
        Test test{i};

        DOCTEST_CHECK_EQ(delegate(test, &Test::next)(), 2);
        DOCTEST_CHECK_EQ(delegate(test, &Test::next)(), 3);
        DOCTEST_CHECK_EQ(delegate(test, &Test::add)(4), 7);
    }

    DOCTEST_TEST_CASE("delegate reassignment") {
        int i1 = 1;
        Test t1{i1};

        int i2 = 1;
        Test t2{i2};

        delegate<int()> d;
        d = delegate(&t1, &Test::next);
        DOCTEST_CHECK(!d.empty());
        d();
        DOCTEST_CHECK_EQ(i1, 2);

        d = delegate(&t2, &Test::next);
        DOCTEST_CHECK(!d.empty());
        d();
        DOCTEST_CHECK_EQ(i2, 2);
    }
}
