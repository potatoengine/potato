#include "grimm/foundation/fixed_string_writer.h"
#include "doctest.h"

DOCTEST_TEST_SUITE("[grimm][foundation] gm::fixed_string_writer") {
    using namespace gm;

    DOCTEST_TEST_CASE("default initialization") {
        fixed_string_writer<32> sw;

        DOCTEST_CHECK(sw.empty());
        DOCTEST_CHECK_EQ(sw.size(), 0);
        DOCTEST_CHECK_EQ(sw.capacity(), 31);
        DOCTEST_CHECK_EQ(sw.c_str(), "");
    }

    DOCTEST_TEST_CASE("write") {
        fixed_string_writer<32> sw;

        sw.write("hello");
        sw.write(',');
        sw.write(' ');
        sw.write("world");

        DOCTEST_CHECK(!sw.empty());
        DOCTEST_CHECK_EQ(sw.size(), 12);
        DOCTEST_CHECK_EQ(sw.c_str(), "hello, world");
    }

    DOCTEST_TEST_CASE("clear") {
        fixed_string_writer<32> sw;

        sw.write("test");
        sw.clear();

        DOCTEST_CHECK(sw.empty());
        DOCTEST_CHECK_EQ(sw.size(), 0);
        DOCTEST_CHECK_EQ(sw.c_str(), "");
    }

    DOCTEST_TEST_CASE("overflow write") {
        fixed_string_writer<32> sw;

        sw.write("initial text");
        sw.write("more text");
        sw.write("yet more text");
        sw.write("and some more text");

        DOCTEST_CHECK_EQ(sw.size(), sw.capacity());
        DOCTEST_CHECK_EQ(sw.c_str(), "initial textmore textyet more t");
    }
}
