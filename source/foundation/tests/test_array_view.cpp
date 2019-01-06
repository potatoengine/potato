#include "grimm/foundation/span.h"
#include "doctest.h"

DOCTEST_TEST_SUITE("[grimm][foundation] gm::span") {
    using namespace gm;

    DOCTEST_TEST_CASE("span default initialization") {
        span<int> av{};

        DOCTEST_CHECK(av.empty());
        DOCTEST_CHECK_EQ(av.size(), 0);
        DOCTEST_CHECK_EQ(av.begin(), av.end());
    }

    DOCTEST_TEST_CASE("span array") {
        int a[] = {1, 3, 2, 5, 4};
        span av(a);
        auto p = av.data();

        DOCTEST_CHECK(!av.empty());
        DOCTEST_CHECK_EQ(av.size(), 5);
        DOCTEST_CHECK_EQ(av.data(), a);
        DOCTEST_CHECK_EQ(av.front(), 1);
        DOCTEST_CHECK_EQ(av.back(), 4);

        av.pop_front();

        DOCTEST_CHECK_EQ(av.size(), 4);
        DOCTEST_CHECK_EQ(av.front(), 3);

        av.pop_back();

        DOCTEST_CHECK_EQ(av.size(), 3);
        DOCTEST_CHECK_EQ(av.back(), 5);
    }
}
