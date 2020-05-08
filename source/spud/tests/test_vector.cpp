#include "potato/spud/vector.h"
#include "potato/spud/string.h"
#include <doctest/doctest.h>

DOCTEST_TEST_SUITE("[potato][spud] vector") {
    using namespace up;

    DOCTEST_TEST_CASE("vector default initialization") {
        vector<int> vec;

        DOCTEST_CHECK_EQ(vec.size(), 0);
        DOCTEST_CHECK_EQ(vec.capacity(), 0);
        DOCTEST_CHECK(vec.empty());
    }

    DOCTEST_TEST_CASE("vector initializer list") {
        vector vec{1, 2, 3, 4};

        DOCTEST_CHECK_EQ(vec.size(), 4);
        DOCTEST_CHECK_GE(vec.capacity(), 4);
        DOCTEST_CHECK_FALSE(vec.empty());

        DOCTEST_CHECK_EQ(vec.front(), 1);
        DOCTEST_CHECK_EQ(vec.back(), 4);
    }

    DOCTEST_TEST_CASE("vector push_back") {
        vector<int> vec;

        for (int i = 1; i <= 10000; ++i) {
            vec.push_back(i * i);
        }

        DOCTEST_CHECK_EQ(vec.size(), 10000);
        DOCTEST_CHECK_GE(vec.capacity(), 10000);
        DOCTEST_CHECK_FALSE(vec.empty());

        DOCTEST_CHECK_EQ(vec.begin() + 10000, vec.end());
        DOCTEST_CHECK_EQ(&*vec.begin(), vec.data());

        DOCTEST_CHECK_EQ(vec.front(), 1);
        DOCTEST_CHECK_EQ(*vec.begin(), 1);
        DOCTEST_CHECK_EQ(*vec.data(), 1);

        DOCTEST_CHECK_EQ(vec.back(), 10000 * 10000);
    }

    DOCTEST_TEST_CASE("vector resize") {
        vector vec{1, 2, 3, 4};

        vec.resize(5);

        DOCTEST_CHECK_EQ(vec.size(), 5);
        DOCTEST_CHECK_EQ(vec.back(), 0);

        vec.resize(6, 7);

        DOCTEST_CHECK_EQ(vec.size(), 6);
        DOCTEST_CHECK_EQ(vec.front(), 1);
        DOCTEST_CHECK_EQ(vec.back(), 7);
    }

    DOCTEST_TEST_CASE("vector erase") {
        vector vec{1, 2, 3, 4};

        vec.erase(vec.begin());

        DOCTEST_CHECK_EQ(vec.size(), 3);
        DOCTEST_CHECK_EQ(vec.front(), 2);
        DOCTEST_CHECK_EQ(vec.back(), 4);

        vec.erase(vec.begin() + 1, vec.end());

        DOCTEST_CHECK_EQ(vec.size(), 1);
        DOCTEST_CHECK_EQ(vec.front(), 2);
    }

    DOCTEST_TEST_CASE("vector<string>") {
        vector<string> vec1{
            "first"_sv,
            "second"_sv,
            "third"_sv,
            "fourth"_sv};
    }
}
