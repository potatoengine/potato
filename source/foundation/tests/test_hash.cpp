#include "grimm/foundation/hash.h"
#include "grimm/foundation/string_view.h"
#include "grimm/foundation/zstring_view.h"
#include "grimm/foundation/span.h"
#include "doctest.h"

DOCTEST_TEST_SUITE("[grimm][foundation] gm::hash") {
    using namespace gm;

    DOCTEST_TEST_CASE("default hash_value") {
        DOCTEST_CHECK_EQ(hash_value<fnv1a>('x'), 0xaf63f54c86021707);
        DOCTEST_CHECK_EQ(hash_value<fnv1a>(string_view("hello world")), 0x779a65e7023cd2e7);
        DOCTEST_CHECK_EQ(hash_value<fnv1a>(zstring_view("hello world")), 0x779a65e7023cd2e7);
        DOCTEST_CHECK_EQ(hash_value<fnv1a>(span<int const>{1, 2, 3, 4, 5}), 0x1916ceffaf539564);
    }
}
