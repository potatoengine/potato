#include "potato/spud/hash.h"
#include "potato/spud/string_view.h"
#include "potato/spud/zstring_view.h"
#include "potato/spud/span.h"
#include <doctest/doctest.h>

DOCTEST_TEST_SUITE("[potato][foundation] up::hash") {
    using namespace up;

    DOCTEST_TEST_CASE("default hash_value") {
        DOCTEST_CHECK_EQ(hash_value<fnv1a>('x'), 0xaf63f54c86021707);
        DOCTEST_CHECK_EQ(hash_value<fnv1a>(string_view("hello world")), 0x779a65e7023cd2e7);
        DOCTEST_CHECK_EQ(hash_value<fnv1a>(zstring_view("hello world")), 0x779a65e7023cd2e7);
        DOCTEST_CHECK_EQ(hash_value<fnv1a>(span<int const>{{1, 2, 3, 4, 5}}), 0x1916ceffaf539564);
    }

    DOCTEST_TEST_CASE("hash_combine") {
        uint64 hash1 = hash_value(7);
        uint64 hash2 = hash_value(-99);

        DOCTEST_CHECK_NE(hash1, hash_combine(hash1, hash2));
        DOCTEST_CHECK_NE(hash2, hash_combine(hash1, hash2));
        DOCTEST_CHECK_NE(hash_combine(hash1, hash2), hash_combine(hash2, hash1));
    }
}
