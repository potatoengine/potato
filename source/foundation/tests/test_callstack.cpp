#include "grimm/foundation/callstack.h"
#include "doctest.h"

DOCTEST_TEST_SUITE("[grimm][foundation] gm::CallStackReader") {
    using namespace gm;

    DOCTEST_TEST_CASE("read callstack") {
        CallStackBuffer<> addresses = {};

        auto stack = CallStackReader::readCallstack(addresses);
        DOCTEST_CHECK(!stack.empty());
    }
}
