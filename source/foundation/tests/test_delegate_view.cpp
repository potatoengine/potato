#include "grimm/foundation/delegate_view.h"
#include "doctest.h"

namespace {
    struct Test {
        int& r;

        int next() { return ++r; }
        int add(int i) const { return r + i; }
    };
} // namespace

DOCTEST_TEST_SUITE("[grimm][foundation] gm::delegate_view") {
    using namespace gm;

    DOCTEST_TEST_CASE("lambda delegate_view") {
        delegate_view d = +[](int i) { return i * 2; };

        DOCTEST_CHECK_EQ(d(0), 0);
        DOCTEST_CHECK_EQ(d(-1), -2);
        DOCTEST_CHECK_EQ(d(10), 20);
    }

    DOCTEST_TEST_CASE("delegate_view reassignment") {
        int i1 = 1;
        Test t1{i1};

        int i2 = 1;
        Test t2{i2};

        delegate_view<int(Test&)> d = delegate_view(&Test::next);
        d(t1);
        DOCTEST_CHECK_EQ(i1, 2);

        d = delegate_view(&Test::next);
        d(t2);
        DOCTEST_CHECK_EQ(i2, 2);
    }
}
