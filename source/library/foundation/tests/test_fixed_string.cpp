#include "grimm/foundation/fixed_string.h"
#include "doctest.h"
#include <iostream>

DOCTEST_TEST_SUITE("[grimm][foundation] gm::fixed_string") {
    using namespace gm;

    DOCTEST_TEST_CASE("empty fixed_string") {
        fixed_string<32> fs;

        DOCTEST_CHECK(fs.empty());

        DOCTEST_CHECK_EQ(fs.size(), 0);
        DOCTEST_CHECK_EQ(fs.capacity(), 31);
        DOCTEST_CHECK_EQ(*fs.data(), '\0');
    }

    DOCTEST_TEST_CASE("fixed_string from string_view") {
        fixed_string<32> fs = string_view("hello world");

        DOCTEST_CHECK(!fs.empty());

        DOCTEST_CHECK_EQ(fs.size(), 11);
        DOCTEST_CHECK_EQ(fs.capacity(), 31);
        DOCTEST_CHECK(std::strcmp(fs.c_str(), "hello world") == 0);
    }

    DOCTEST_TEST_CASE("fixed_string from overlong string_view") {
        fixed_string<5> ss = string_view("hello world");

        DOCTEST_CHECK(!ss.empty());

        DOCTEST_CHECK_EQ(ss.size(), 4);
        DOCTEST_CHECK_EQ(ss.capacity(), 4);
        DOCTEST_CHECK(std::strcmp(ss.c_str(), "hell") == 0);
    }
}
