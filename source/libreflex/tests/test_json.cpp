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

    SECTION("decode complex") {
        reflex::JsonDecoder decoder;

        string_view const json =
            R"({"$schema":"TestComplex","name":"Frederick","values":[42.0,-7.0,6000000000.0],"test":{"$schema":"TestStruct","test":"Second"}})";

        TestComplex comp;
        CHECK(decoder.decode(nlohmann::json::parse(json), comp));

        CHECK(comp.name == "Frederick"_s);
        REQUIRE(comp.values.size() == 3);
        CHECK(comp.values[0] == 42.f);
        CHECK(comp.values[1] == -7.f);
        CHECK(comp.values[2] == 6'000'000'000.f);
        CHECK(comp.test.test == TestEnum::Second);
    }
}
