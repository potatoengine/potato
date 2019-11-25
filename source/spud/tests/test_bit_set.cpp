#include "potato/spud/bit_set.h"
#include <doctest/doctest.h>

DOCTEST_TEST_SUITE("[potato][spud] up::bit_set") {
    using namespace up;

    DOCTEST_TEST_CASE("empty") {
        bit_set bs;

        DOCTEST_CHECK(!bs.test(0));
    }

    DOCTEST_TEST_CASE("set") {
        bit_set bs;

        bs.set(0);
        DOCTEST_CHECK(bs.test(0));
        DOCTEST_CHECK(!bs.test(1));
        DOCTEST_CHECK_EQ(bs.capacity(), 64);

        bs.set(1000);
        DOCTEST_CHECK(bs.test(1000));
        DOCTEST_CHECK(!bs.test(999));
        DOCTEST_CHECK(!bs.test(1001));
        DOCTEST_CHECK_EQ(bs.capacity(), 1024);
    }

    DOCTEST_TEST_CASE("reset") {
        bit_set bs;

        bs.set(0);
        bs.reset(0);
        DOCTEST_CHECK(!bs.test(0));
        DOCTEST_CHECK(!bs.test(1));

        bs.set(1000);
        bs.reset(1000);
        DOCTEST_CHECK(!bs.test(1000));
        DOCTEST_CHECK(!bs.test(999));
        DOCTEST_CHECK(!bs.test(1001));
    }

    DOCTEST_TEST_CASE("equality") {
        bit_set bs1;
        bit_set bs2;

        bs1.set(0);
        bs1.set(7);
        bs1.set(109);
        bs1.set(555);

        bs2.set(0);
        bs2.set(7);

        DOCTEST_CHECK_FALSE(bs1 == bs2);

        bs2.set(109);
        bs2.set(555);

        DOCTEST_CHECK(bs1 == bs2);

        bs2.set(909);

        DOCTEST_CHECK_FALSE(bs1 == bs2);
    }

    DOCTEST_TEST_CASE("has_all") {
        bit_set bs1;
        bit_set bs2;

        bs1.set(0);
        bs1.set(7);
        bs1.set(109);
        bs1.set(555);

        bs2.set(0);
        bs2.set(7);

        DOCTEST_CHECK(bs1.has_all(bs2));

        bs2.set(109);
        bs2.set(555);

        DOCTEST_CHECK(bs1.has_all(bs2));

        bs2.set(909);

        DOCTEST_CHECK_FALSE(bs1.has_all(bs2));
    }
}
