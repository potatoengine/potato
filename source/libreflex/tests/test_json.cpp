// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "reflex_test_schema.h"

#include "potato/reflex/json.h"

#include <catch2/catch.hpp>

TEST_CASE("TestJson", "[potato][reflex]") {
    using namespace up;
    using namespace up::schema;

    SECTION("encode") {
        reflex::JsonEncoder encoder;
        TestStruct s{TestEnum::First};
        CHECK(encoder.encodeObject(s));
        auto const json = encoder.document().dump();
        CHECK(json == R"({"$schema":"TestStruct","test":"First"})");
    }
}
