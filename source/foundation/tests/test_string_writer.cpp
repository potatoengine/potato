#include "grimm/foundation/string_writer.h"
#include "grimm/foundation/string_blob.h"
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

    DOCTEST_TEST_CASE("request and commit") {
        string_writer sw;

        sw.write("initial text");

        auto mem = sw.acquire(32);

        DOCTEST_CHECK_GE(mem.size(), 32);
        DOCTEST_CHECK_EQ((void*)mem.data(), (void*)(sw.data() + sw.size()));

        std::memcpy(mem.data(), "data", 4);

        sw.commit(mem.first(4));

        sw.write("footer");

        DOCTEST_CHECK_EQ(sw.c_str(), "initial textdatafooter");
    }

    DOCTEST_TEST_CASE("resize") {
        string_writer sw;

        sw.write("initial text");

        sw.resize(4);
        DOCTEST_CHECK_EQ(sw.c_str(), "init");

        sw.resize(6);
        DOCTEST_CHECK_EQ(sw.c_str(), "init  ");

        sw.resize(8, 'x');
        DOCTEST_CHECK_EQ(sw.c_str(), "init  xx");

        sw.write("yy");
        DOCTEST_CHECK_EQ(sw.c_str(), "init  xxyy");
    }

    DOCTEST_TEST_CASE("to_string") {
        string_writer sw;

        sw.write("some text here");

        string s = sw.to_string();

        DOCTEST_CHECK_EQ(sw.c_str(), "some text here");
        DOCTEST_CHECK_EQ(s.c_str(), "some text here");

        s = std::move(sw).to_string();

        DOCTEST_CHECK(sw.empty());
        DOCTEST_CHECK_EQ(s.c_str(), "some text here");
    }
}
