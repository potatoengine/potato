#include "grimm/foundation/delegate.h"
#include "doctest.h"

DOCTEST_TEST_SUITE("[grimm][foundation] gm::delegate") {
    using namespace gm;

    DOCTEST_TEST_CASE("empty delegate") {
        gm::delegate<int()> d;

        DOCTEST_CHECK(d.empty());
    }

    DOCTEST_TEST_CASE("lambda delegate") {
        gm::delegate d = +[](int i) { return i * 2; };

        DOCTEST_CHECK(!d.empty());
        DOCTEST_CHECK_EQ(d(0), 0);
        DOCTEST_CHECK_EQ(d(-1), -2);
        DOCTEST_CHECK_EQ(d(10), 20);
    }
}
