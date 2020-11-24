// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/generator.h"

#include <catch2/catch.hpp>
#include <sstream>

static up::generator<int> count(int last) {
    for (int i = 0; i != last; ++i) {
        co_yield i;
    }
}

TEST_CASE("[potato][spud] up::generator") {
    using namespace up;

    SECTION("generator") {
        generator<int> g = count(3);

        CHECK(g() == 0);
        CHECK(g() == 1);
        CHECK(g() == 2);
        CHECK_FALSE(g);
    }

    SECTION("generator range") {
        std::ostringstream buf;
        for (int i : count(5)) {
            buf << i;
        }
        CHECK(buf.str() == "01234");
    }
}
