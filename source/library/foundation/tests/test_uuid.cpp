#include "potato/foundation/uuid.h"
#include <doctest/doctest.h>
#include <array>

DOCTEST_TEST_SUITE("[potato][foundation] up::uuid") {
    using namespace up;

    DOCTEST_TEST_CASE("basic uuid") {
        uuid a = uuid::generate();
        DOCTEST_CHECK(a.isValid());

        uuid zero = uuid({0});
        DOCTEST_CHECK(zero == uuid::zero());
    }
}
