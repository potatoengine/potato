#include "potato/runtime/uuid.h"
#include <doctest/doctest.h>
#include <array>

DOCTEST_TEST_SUITE("[potato][foundation] up::uuid") {
    using namespace up;

    DOCTEST_TEST_CASE("basic uuid") {
        uuid a = uuid::generate();
        DOCTEST_CHECK(a.isValid());

        string str = uuid::toString(a);

        uuid zero = uuid();
        DOCTEST_CHECK(zero == uuid::zero());
        DOCTEST_CHECK_EQ(uuid::toString(zero).c_str(), "00000000-0000-0000-0000-000000000000");
    }
}
