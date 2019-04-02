#include "grimm/foundation/callstack.h"
#include <doctest/doctest.h>
#include <array>

DOCTEST_TEST_SUITE("[grimm][foundation] gm::callstack") {
    using namespace gm;
    using namespace gm::callstack;

    DOCTEST_TEST_CASE("read callstack") {
        std::array<uintptr, 1> addresses = {};

        auto stack = readTrace(span{addresses.data(), addresses.size()});
        DOCTEST_CHECK(!stack.empty());
    }
}
