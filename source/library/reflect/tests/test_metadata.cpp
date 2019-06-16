#include "potato/reflect/metadata.h"
#include "potato/reflect/reflect.h"
#include <doctest/doctest.h>

namespace {
    struct Fields {
        int x, y, z;
    };

    UP_REFLECT_TYPE(Fields) {}
} // namespace

DOCTEST_TEST_SUITE("[potato][reflect] metadata") {
    using namespace up;
    using namespace up::reflex;

    DOCTEST_TEST_CASE("TypeInfo<int>") {
        auto info = getTypeInfo<int>();

        DOCTEST_CHECK_EQ(sizeof(int), info.size);
        DOCTEST_CHECK_EQ(alignof(int), info.alignment);
    }

    DOCTEST_TEST_CASE("TypeInfo<Fields>") {
        auto info = getTypeInfo<Fields>();

        DOCTEST_CHECK_EQ("Fields", info.name);
        DOCTEST_CHECK_EQ(sizeof(Fields), info.size);
        DOCTEST_CHECK_EQ(alignof(Fields), info.alignment);
    }
}
