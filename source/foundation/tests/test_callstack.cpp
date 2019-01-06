#include "grimm/foundation/callstack.h"
#include "doctest.h"

DOCTEST_TEST_SUITE("[grimm][foundation] gm::callstack") {
    using namespace gm::callstack;

    DOCTEST_TEST_CASE("read callstack") {
        std::array<uintptr, 1> addresses = {};

        auto stack = readTrace(addresses);
        DOCTEST_CHECK(!stack.empty());
    }
}
