// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/string.h"

#include <catch2/catch.hpp>
#include <ostream>

TEST_CASE("[potato][spud] up::string") {
    using namespace up;

    SECTION("default initialization") {
        string s;

        CHECK(s.empty());
        CHECK(s.size() == 0);
    }

    SECTION("std::string initialization") {
        std::string ss = "this is a test";
        string s(ss.data(), ss.size());

        CHECK_FALSE(s.empty());
        CHECK(s.size() == s.size());
        CHECK(s.c_str() == ss.c_str());
    }

    SECTION("initialize from string_view") {
        string s("test"_sv);

        CHECK_FALSE(s.empty());
        CHECK(s.size() == 4);
    }

    SECTION("move constructor") {
        string s("wrong");
        string s2("correct");

        s = std::move(s2);

        CHECK_FALSE(s.empty());
        CHECK(s.size() == s.size());
        CHECK(s.c_str() == "correct");

        CHECK(s2.empty());
    }

    SECTION("literal initialization") {
        string s("this is a test");

        CHECK_FALSE(s.empty());
        CHECK(s.size() == 14);
        CHECK(s.c_str() == "this is a test");
    }

    SECTION("C string initialization") {
        char const* cs = "this is a test";
        string s(cs);

        CHECK_FALSE(s.empty());
        CHECK(s.size() == std::strlen(cs));
        CHECK(s.c_str() == cs);
    }

    SECTION("slicing") {
        string s("this is a test");

        CHECK(s.first(7) == "this is");
        CHECK(s.last(6) == "a test");
        CHECK(s.substr(8) == "a test");
        CHECK(s.substr(8, 1) == "a"_sv);

        CHECK(s.front() == 't');
        CHECK(s.back() == 't');
    }

    SECTION("searching") {
        string s("this is a test");

        CHECK(s.find('a') == 8);
        CHECK(s.find('z') == string::npos);

        CHECK(s.find_first_of("ea") == 8);
        CHECK(s.find_first_of("ez") == 11);
        CHECK(s.find_first_of("fz") == string::npos);

        CHECK(s.find_last_of("ih") == 5);
        CHECK(s.find_last_of("zh") == 1);
        CHECK(s.find_last_of("fz") == string::npos);
    }

    SECTION("take_ownership") {
        char* cs = new char[12];
        std::memcpy(cs, "hello world", 12);

        string s = string::take_ownership(cs, 11);
        CHECK(s.c_str() == "hello world");

        s.reset();
    }
}
