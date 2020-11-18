// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "reflex_test_schema.h"

#include "potato/reflex/json.h"

#include <catch2/catch.hpp>

TEST_CASE("TestJson", "[potato][reflex]") {
    using namespace up;
    using namespace up::schema;

    SECTION("encode test") {
        reflex::JsonEncoder encoder;
        TestStruct s{TestEnum::First};
        CHECK(encoder.encodeObject(s));
        auto const json = encoder.document().dump();
        CHECK(json == R"({"$schema":"TestStruct","test":"First"})");
    }

    SECTION("encode complex") {
        reflex::JsonEncoder encoder;
        TestComplex comp;
        comp.name = "Frederick"_s;
        comp.values.push_back(42.f);
        comp.values.push_back(-7.f);
        comp.values.push_back(6'000'000'000.f);
        comp.test.test = TestEnum::Second;

        CHECK(encoder.encodeObject(comp));
        auto const json = encoder.document().dump();
        CHECK(
            json ==
            R"({"$schema":"TestComplex","name":"Frederick","test":{"$schema":"TestStruct","test":"Second"},"values":[42.0,-7.0,6000000000.0]})");
    }
}
