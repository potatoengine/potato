#include "grimm/foundation/delegate_view.h"
#include "doctest.h"

DOCTEST_TEST_SUITE("[grimm][foundation] gm::delegate_view") {
    using namespace gm;

    DOCTEST_TEST_CASE("lambda delegate_view") {
        int (*f)(int) = [](int i) { return i * 2; };
        delegate_view d = f;

        DOCTEST_CHECK_EQ(d(0), 0);
        DOCTEST_CHECK_EQ(d(-1), -2);
        DOCTEST_CHECK_EQ(d(10), 20);
    }

    DOCTEST_TEST_CASE("delegate_view reassignment") {
        int i1 = 2;
        auto f1 = [&i1](int i) { return i1 += i; };
        static_assert(is_invocable_v<decltype(f1), int>);

        int i2 = 2;
        auto f2 = [&i2](int i) { return i2 *= i; };

        delegate_view<int(int)> d(f1);
        d(2);
        DOCTEST_CHECK_EQ(i1, 4);

        d = f2;
        d(4);
        DOCTEST_CHECK_EQ(i2, 8);
    }
}
