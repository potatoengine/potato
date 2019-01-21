#include "grimm/foundation/string_blob.h"
#include "doctest.h"
#include <ostream>

DOCTEST_TEST_SUITE("[grimm][foundation] gm::string") {
    using namespace gm;

    DOCTEST_TEST_CASE("default initialization") {
        string s;

        DOCTEST_CHECK(s.empty());
        DOCTEST_CHECK_EQ(s.size(), 0);
    }

    DOCTEST_TEST_CASE("std::string initialization") {
        std::string ss = "this is a test";
        string s(ss);

        DOCTEST_CHECK(!s.empty());
        DOCTEST_CHECK_EQ(s.size(), s.size());
        DOCTEST_CHECK_EQ(s.c_str(), ss.c_str());
    }

    DOCTEST_TEST_CASE("move constructor") {
        string s("wrong");
        string s2("correct");

        s = std::move(s2);

        DOCTEST_CHECK(!s.empty());
        DOCTEST_CHECK_EQ(s.size(), s.size());
        DOCTEST_CHECK_EQ(s.c_str(), "correct");

        DOCTEST_CHECK(s2.empty());
    }

    DOCTEST_TEST_CASE("literal initialization") {
        string s("this is a test");

        DOCTEST_CHECK(!s.empty());
        DOCTEST_CHECK_EQ(s.size(), 14);
        DOCTEST_CHECK_EQ(s.c_str(), "this is a test");
    }

    DOCTEST_TEST_CASE("C string initialization") {
        char const* cs = "this is a test";
        string s(cs);

        DOCTEST_CHECK(!s.empty());
        DOCTEST_CHECK_EQ(s.size(), std::strlen(cs));
        DOCTEST_CHECK_EQ(s.c_str(), cs);
    }

    DOCTEST_TEST_CASE("slicing") {
        string s("this is a test");

        DOCTEST_CHECK_EQ(s.first(7), "this is");
        DOCTEST_CHECK_EQ(s.last(6), "a test");
        DOCTEST_CHECK_EQ(s.substr(8), "a test");
        DOCTEST_CHECK_EQ(s.substr(8, 1), "a");

        DOCTEST_CHECK_EQ(s.front(), 't');
        DOCTEST_CHECK_EQ(s.back(), 't');
    }

    DOCTEST_TEST_CASE("searching") {
        string s("this is a test");

        DOCTEST_CHECK_EQ(s.find('a'), 8);
        DOCTEST_CHECK_EQ(s.find('z'), string::npos);

        DOCTEST_CHECK_EQ(s.find_first_of("ea"), 8);
        DOCTEST_CHECK_EQ(s.find_first_of("ez"), 11);
        DOCTEST_CHECK_EQ(s.find_first_of("fz"), string::npos);

        DOCTEST_CHECK_EQ(s.find_last_of("ih"), 5);
        DOCTEST_CHECK_EQ(s.find_last_of("zh"), 1);
        DOCTEST_CHECK_EQ(s.find_last_of("fz"), string::npos);
    }
}
