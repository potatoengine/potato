// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/reflex/json_serializer.h"
#include "potato/reflex/reflect.h"
#include "potato/reflex/serializer.h"
#include "potato/spud/vector.h"

#include <catch2/catch.hpp>
#include <sstream>

namespace {
    struct RecordingSerializer {
        template <typename T>
        constexpr void value(T&) noexcept {}
    };

    struct RecordingReflector {
        template <typename T>
        constexpr void value() noexcept {}

        template <typename T, typename F>
        constexpr void field(up::zstring_view, F T::*) noexcept {}
    };

    struct Fields {
        int x, y, z;
    };

    struct Complex {
        Fields xyz = {0, 0, 0};
        float num = 0;
        up::string name;
        up::vector<int> vec;
    };

    UP_REFLECT_TYPE(Fields) {
        reflect("x", &Type::x);
        reflect("y", &Type::y);
        reflect("z", &Type::z);
    }

    UP_REFLECT_TYPE(Complex) {
        reflect("xyz", &Type::xyz);
        reflect("num", &Type::num, up::reflex::JsonName("custom"));
        reflect("name", &Type::name);
        reflect("vec", &Type::vec);
    }
} // namespace

TEST_CASE("serialize", "[potato][reflect]") {
    using namespace up;
    using namespace up::reflex;

    SECTION("serialize struct to json") {
        auto root = nlohmann::json::object();
        auto serializer = JsonStreamSerializer{root};

        auto const big = Complex{{1, 2, 3}, 42.f, "bob", {4, 5, 6}}; // NOLINT

        serialize(big, serializer);

        std::ostringstream ostr;
        ostr << root;

        CHECK(ostr.str() == R"--({"custom":42.0,"name":"bob","vec":[4,5,6],"xyz":{"x":1,"y":2,"z":3}})--");
    }

    SECTION("deserialize struct from json") {
        auto root = nlohmann::json::parse(
            R"--({"name":"bob","custom":42.0,"ignore":{"num":7},"xyz":{"x":1,"y":2,"z":3},"vec":[4,6]})--");
        auto serializer = JsonStreamDeserializer{root};

        Complex big;
        serialize(big, serializer);

        CHECK(big.xyz.x == 1);
        CHECK(big.xyz.y == 2);
        CHECK(big.xyz.z == 3);

        CHECK(big.num == 42.f); // NOLINT

        CHECK(big.name == "bob");

        REQUIRE(big.vec.size() == 2); // NOLINT
        CHECK(big.vec.front() == 4); // NOLINT
        CHECK(big.vec.back() == 6); // NOLINT
    }
}

TEST_CASE("reflect", "[potato][reflect]") {
    using namespace up;
    using namespace up::reflex;

    SECTION("reflect int") {
        RecordingReflector ref{};
        reflect<int>(ref);
    }

    SECTION("reflect struct") {
        RecordingReflector ref{};

        reflect<Fields>(ref);
    }
}
