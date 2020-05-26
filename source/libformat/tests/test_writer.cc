#include "potato/format/format.h"
#include "potato/format/std_string.h"

#include "potato/spud/string_view.h"

#include <doctest/doctest.h>

#include <ostream>
#include <vector>

DOCTEST_TEST_CASE("writer") {
    using namespace up;

    DOCTEST_SUBCASE("fixed") {
        // can hold 9 characters and a NUL byte
        char buffer[10];
        fixed_writer writer(buffer);

        // should not truncate
        writer.write("test");
        DOCTEST_CHECK_EQ(std::string("test"), buffer);

        // should truncate
        writer = fixed_writer(buffer);
        format_to(writer, "test {0}", /*too big*/ 1234567890LL);
        DOCTEST_CHECK_EQ(std::string("test 1234"), buffer);
    }

    DOCTEST_SUBCASE("append") {
        std::string tmp;
        append_writer writer(tmp);

        format_to(writer, "1{}3", "2");
        DOCTEST_CHECK_EQ(std::string("123"), tmp.c_str());

        tmp.clear();
        format_to(writer, "1{}3{}5{}7{}9{}", 2, 4, 6, 8, 0);
        DOCTEST_CHECK_EQ(std::string("1234567890"), tmp.c_str());
    }
}
