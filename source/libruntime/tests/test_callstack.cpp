#include "potato/runtime/callstack.h"

#include <doctest/doctest.h>

#include <array>

DOCTEST_TEST_SUITE("[potato][runtime] up::callstack") {
    using namespace up;
    using namespace up::callstack;

    DOCTEST_TEST_CASE("read callstack") {
        std::array<uintptr, 1> addresses = {};

        auto stack = readTrace(span{addresses.data(), addresses.size()});
        DOCTEST_CHECK(!stack.empty());
    }
}
