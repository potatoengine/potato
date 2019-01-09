#include "grimm/foundation/string_writer.h"
#include "doctest.h"

DOCTEST_TEST_SUITE("[grimm][foundation] gm::string_writer") {
    using namespace gm;

    DOCTEST_TEST_CASE("default initialization") {
        string_writer sw;

        DOCTEST_CHECK(sw.empty());
        DOCTEST_CHECK_EQ(sw.size(), 0);
        DOCTEST_CHECK_EQ(sw.c_str(), "");
    }

    DOCTEST_TEST_CASE("reserve initialization") {
        string_writer sw(64);

        DOCTEST_CHECK(sw.empty());
        DOCTEST_CHECK_EQ(sw.size(), 0);
        DOCTEST_CHECK_EQ(sw.c_str(), "");
    }

    DOCTEST_TEST_CASE("write") {
        string_writer sw;

        sw.write("hello");
        sw.write(',');
        sw.write(' ');
        sw.write("world");

        DOCTEST_CHECK(!sw.empty());
        DOCTEST_CHECK_EQ(sw.size(), 12);
        DOCTEST_CHECK_EQ(sw.c_str(), "hello, world");
    }

    DOCTEST_TEST_CASE("clear") {
        string_writer sw;

        sw.write("test");
        sw.clear();

        DOCTEST_CHECK(sw.empty());
        DOCTEST_CHECK_EQ(sw.size(), 0);
        DOCTEST_CHECK_EQ(sw.c_str(), "");
    }
}
