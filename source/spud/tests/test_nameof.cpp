// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/nameof.h"

#include <doctest/doctest.h>

static_assert(up::nameof<int>() == "int");

template <typename T> struct as_template {};

DOCTEST_TEST_SUITE("[potato][spud] up::nameof") {
    using namespace up;

    DOCTEST_TEST_CASE("builtin types") {
        DOCTEST_CHECK_EQ("int", nameof<int>());
        DOCTEST_CHECK_EQ("float", nameof<float>());
        DOCTEST_CHECK_EQ("char", nameof<char>());
    }

    DOCTEST_TEST_CASE("class types") {
#if defined(UP_COMPILER_MICROSOFT)
        DOCTEST_CHECK_EQ("class up::string_view", nameof<string_view>());
#else
        DOCTEST_CHECK_EQ("up::string_view", nameof<string_view>());
#endif
    }

    DOCTEST_TEST_CASE("template types") {
#if defined(UP_COMPILER_MICROSOFT)
        DOCTEST_CHECK_EQ("struct as_template<int>", nameof<as_template<int>>());
#else
        DOCTEST_CHECK_EQ("as_template<int>", nameof<as_template<int>>());
#endif
    }
}
