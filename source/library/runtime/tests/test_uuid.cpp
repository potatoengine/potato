#include "potato/runtime/uuid.h"
#include <doctest/doctest.h>
#include <array>

DOCTEST_TEST_SUITE("[potato][foundation] up::uuid") {
    using namespace up;

    DOCTEST_TEST_CASE("basic uuid") {
        uuid zero = uuid();
        DOCTEST_CHECK(zero == uuid::zero());
        DOCTEST_CHECK_EQ(uuid::toString(zero).c_str(), "00000000-0000-0000-0000-000000000000");

        uuid a = uuid::generate();
        DOCTEST_CHECK(a.isValid());

        string str = uuid::toString(a);
        uuid b = uuid::fromString(str);
        DOCTEST_CHECK(b == a);

        const uuid c = uuid::fromString("00000000-0000-0000-0000-000000000000");
        DOCTEST_CHECK(c == uuid::zero());

        // check invalid length of string
        const uuid d = uuid::fromString("9554084e-4100-4098-b470-2125f5eed13");
        DOCTEST_CHECK(d == uuid::zero());
        const uuid e = uuid::fromString("9554084e-4100-4098-b470-2125f5eed133f");
        DOCTEST_CHECK(e == uuid::zero());
        const uuid f = uuid::fromString("955408-4e4100-4098-b470-2125f5eed133f");
        DOCTEST_CHECK(f == uuid::zero());
        const uuid g = uuid::fromString("9554084e-41K0-4098-PODR-25f5eed133fN");
        DOCTEST_CHECK(g == uuid::zero());

    }
}
