#include "potato/foundation/find.h"
#include "potato/foundation/vector.h"
#include <doctest/doctest.h>
#include <iostream>

namespace {
    constexpr bool is_even(int val) noexcept { return val % 2 == 0; }
    constexpr bool is_negative(int val) noexcept { return val < 0; }
} // namespace

DOCTEST_TEST_SUITE("[potato][foundation] find algorithms") {
    using namespace up;

    int array[] = {4, 7, 9, 2, 1, 0, 900};
    vector vec{-88, 4, 0, 7};

    DOCTEST_TEST_CASE("find") {
        DOCTEST_CHECK_EQ(find(array, 4), std::begin(array));
        DOCTEST_CHECK_EQ(find(vec, -88), std::begin(vec));

        DOCTEST_CHECK_EQ(find(array, 900), std::end(array) - 1);
        DOCTEST_CHECK_EQ(find(vec, 7), std::end(vec) - 1);

        DOCTEST_CHECK_EQ(find(array, 12948734), std::end(array));
        DOCTEST_CHECK_EQ(find(vec, 12948734), std::end(vec));
    }

    DOCTEST_TEST_CASE("find_if") {
        DOCTEST_CHECK_EQ(find_if(array, is_even), std::begin(array));
        DOCTEST_CHECK_EQ(find_if(vec, is_negative), std::begin(vec));

        DOCTEST_CHECK_EQ(find_if(array, [](int i) { return i > 100; }), std::end(array) - 1);
        DOCTEST_CHECK_EQ(find_if(vec, [](int i) { return !is_even(i); }), std::end(vec) - 1);

        DOCTEST_CHECK_EQ(find_if(array, [](int) { return false; }), std::end(array));
        DOCTEST_CHECK_EQ(find_if(vec, [](int) { return false; }), std::end(vec));
    }

    DOCTEST_TEST_CASE("any") {
        DOCTEST_CHECK_EQ(any(array, is_negative), false);
        DOCTEST_CHECK_EQ(any(vec, is_negative), true);
    }

    DOCTEST_TEST_CASE("all") {
        DOCTEST_CHECK_EQ(all(array, [](int i) { return !is_negative(i); }), true);
        DOCTEST_CHECK_EQ(all(vec, is_negative), false);
    }
}
