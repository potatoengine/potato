// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/format/format.h"
#include "potato/format/std_string.h"
#include "potato/spud/string_view.h"

#include <catch2/catch.hpp>
#include <ostream>
#include <vector>

TEST_CASE("writer") {
    using namespace up;

    SECTION("fixed") {
        // can hold 9 characters and a NUL byte
        char buffer[10];
        fixed_writer writer(buffer);

        // should not truncate
        writer.write("test");
        CHECK(buffer == std::string("test"));

        // should truncate
        writer = fixed_writer(buffer);
        format_to(writer, "test {0}", /*too big*/ 1234567890LL);
        CHECK(buffer == std::string("test 1234"));
    }

    SECTION("append") {
        std::string tmp;
        append_writer writer(tmp);

        format_to(writer, "1{}3", "2");
        CHECK(tmp == std::string("123"));

        tmp.clear();
        format_to(writer, "1{}3{}5{}7{}9{}", 2, 4, 6, 8, 0);
        CHECK(tmp == std::string("1234567890"));
    }
}
