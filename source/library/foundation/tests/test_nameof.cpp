#include "potato/foundation/nameof.h"
#include <doctest/doctest.h>

static_assert(up::nameof<int>() == "int");

template <typename T>
struct as_template {};

DOCTEST_TEST_SUITE("[potato][foundation] up::nameof") {
    using namespace up;

    DOCTEST_TEST_CASE("builtin types") {
        DOCTEST_CHECK_EQ("int", nameof<int>());
        DOCTEST_CHECK_EQ("float", nameof<float>());
        DOCTEST_CHECK_EQ("char", nameof<char>());
    }

    DOCTEST_TEST_CASE("class types") {
        DOCTEST_CHECK_EQ("class up::string_view", nameof<string_view>());
    }

    DOCTEST_TEST_CASE("template types") {
        DOCTEST_CHECK_EQ("struct as_template<int>", nameof<as_template<int>>());
    }
}