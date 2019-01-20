#include "grimm/foundation/blob.h"
#include "doctest.h"

DOCTEST_TEST_SUITE("[grimm][foundation] gm::blob") {
    using namespace gm;

    DOCTEST_TEST_CASE("default") {
        blob b;

        DOCTEST_CHECK(b.empty());
        DOCTEST_CHECK(!b);
        DOCTEST_CHECK_EQ(b.size(), 0);
        DOCTEST_CHECK_EQ(b.data(), nullptr);
    }

    DOCTEST_TEST_CASE("size") {
        blob b(32);

        DOCTEST_CHECK(!b.empty());
        DOCTEST_CHECK(b);
        DOCTEST_CHECK_EQ(b.size(), 32);

        std::memcpy(b.data(), "Hello!", 7);

        DOCTEST_CHECK_EQ(b.data_chars(), "Hello!");
    }

    DOCTEST_TEST_CASE("reset") {
        blob b(32);
        b.reset();

        DOCTEST_CHECK(b.empty());
        DOCTEST_CHECK(!b);
        DOCTEST_CHECK_EQ(b.size(), 0);
        DOCTEST_CHECK_EQ(b.data(), nullptr);
    }
}
