// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "reflex_test_schema.h"

#include <catch2/catch.hpp>

TEST_CASE("TestSchema", "[potato][reflex]") {
    using namespace up;
    using namespace up::schema;

    SECTION("type") {
        TestEnum e{TestEnum::First};
        TestStruct s{e};
    }

    SECTION("enumToString") {
        CHECK("First"_sv == reflex::enumToString(TestEnum::First));
        CHECK("Third"_sv == reflex::enumToString(TestEnum::Third));
    }

    SECTION("enumToValue") {
        CHECK(TestEnum::First == reflex::enumToValue<TestEnum>("First"_sv));
        CHECK(TestEnum::Third == reflex::enumToValue<TestEnum>("Third"_sv));
    }
}
