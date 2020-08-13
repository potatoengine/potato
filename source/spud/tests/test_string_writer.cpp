// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/string.h"
#include "potato/spud/string_writer.h"

#include <catch2/catch.hpp>

TEST_CASE("[potato][spud] up::string_writer") {
    using namespace up;

    SECTION("default initialization") {
        string_writer sw;

        CHECK(sw.empty());
        CHECK(sw.size() == 0);
        CHECK(sw.c_str() == "");
    }

    SECTION("reserve initialization") {
        string_writer sw(64);

        CHECK(sw.empty());
        CHECK(sw.size() == 0);
        CHECK(sw.c_str() == "");
    }

    SECTION("write") {
        string_writer sw;

        sw.append("hello");
        sw.append(',');
        sw.append(' ');
        sw.append("world");

        CHECK_FALSE(sw.empty());
        CHECK(sw.size() == 12);
        CHECK(sw.c_str() == "hello, world"_sv);
    }

    SECTION("clear") {
        string_writer sw;

        sw.append("test");
        sw.clear();

        CHECK(sw.empty());
        CHECK(sw.size() == 0);
        CHECK(sw.c_str() == "");
    }

    SECTION("request and commit") {
        string_writer sw;

        sw.append("initial text");

        auto mem = sw.acquire(32);

        CHECK(mem.size() >= 32);
        CHECK((void*)mem.data() == (void*)(sw.data() + sw.size()));

        std::memcpy(mem.data(), "data", 4);

        sw.commit(mem.first(4));

        sw.append("footer");

        CHECK(sw.c_str() == "initial textdatafooter");
    }

    SECTION("resize") {
        string_writer sw;

        sw.append("initial text");

        sw.resize(4);
        CHECK(sw.c_str() == "init");

        sw.resize(6);
        CHECK(sw.c_str() == "init  ");

        sw.resize(8, 'x');
        CHECK(sw.c_str() == "init  xx");

        sw.append("yy");
        CHECK(sw.c_str() == "init  xxyy");
    }

    SECTION("to_string") {
        string_writer sw;

        sw.append("some text here");

        string s = sw.to_string();

        CHECK(sw.c_str() == "some text here");
        CHECK(s.c_str() == "some text here");

        s = std::move(sw).to_string();

        CHECK(sw.empty());
        CHECK(s.c_str() == "some text here");
    }
}
