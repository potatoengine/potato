#include "potato/spud/zstring_view.h"
#include <doctest/doctest.h>
#include <ostream>
#include <cstring>

DOCTEST_TEST_SUITE("[potato][spud] up::zstring_view") {
    using namespace up;

    DOCTEST_TEST_CASE("default initialization") {
        zstring_view sv;

        DOCTEST_CHECK(sv.empty());
        DOCTEST_CHECK_EQ(sv.size(), 0);
    }

    DOCTEST_TEST_CASE("std::string initialization") {
        std::string s = "this is a test";
        zstring_view sv(s.c_str());

        DOCTEST_CHECK(!sv.empty());
        DOCTEST_CHECK_EQ(sv.size(), s.size());
        DOCTEST_CHECK_EQ(sv, s.c_str());
    }

    DOCTEST_TEST_CASE("literal initialization") {
        zstring_view sv = "this is a test";

        DOCTEST_CHECK(!sv.empty());
        DOCTEST_CHECK_EQ(sv.size(), 14);
        DOCTEST_CHECK_EQ(sv, "this is a test");
    }

    DOCTEST_TEST_CASE("C string initialization") {
        char const* cs = "this is a test";
        zstring_view sv = cs;

        DOCTEST_CHECK(!sv.empty());
        DOCTEST_CHECK_EQ(sv.size(), std::strlen(cs));
        DOCTEST_CHECK_EQ(sv, cs);
    }

    DOCTEST_TEST_CASE("slicing") {
        zstring_view sv = "this is a test";

        DOCTEST_CHECK_EQ(sv.first(7), "this is");
        DOCTEST_CHECK_EQ(sv.substr(8), "a test");
        DOCTEST_CHECK_EQ(sv.substr(8, 1), "a");

        DOCTEST_CHECK_EQ(sv.front(), 't');
    }

    DOCTEST_TEST_CASE("searching") {
        zstring_view sv = "this is a test";

        DOCTEST_CHECK_EQ(sv.find('a'), 8);
        DOCTEST_CHECK_EQ(sv.find('z'), zstring_view::npos);

        DOCTEST_CHECK_EQ(sv.find_first_of("ea"), 8);
        DOCTEST_CHECK_EQ(sv.find_first_of("ez"), 11);
        DOCTEST_CHECK_EQ(sv.find_first_of("fz"), zstring_view::npos);

        DOCTEST_CHECK_EQ(sv.find_last_of("ih"), 5);
        DOCTEST_CHECK_EQ(sv.find_last_of("zh"), 1);
        DOCTEST_CHECK_EQ(sv.find_last_of("fz"), zstring_view::npos);
    }
}
