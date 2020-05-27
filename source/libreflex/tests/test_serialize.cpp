// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/reflex/json_serializer.h"
#include "potato/reflex/reflect.h"
#include "potato/reflex/serializer.h"
#include "potato/spud/vector.h"

#include <doctest/doctest.h>
#include <sstream>

namespace {
    struct RecordingSerializer {
        template <typename T> constexpr void value(T&) noexcept {}
    };

    struct RecordingReflector {
        template <typename T> constexpr void value() noexcept {}

        template <typename T, typename F> constexpr void field(up::zstring_view, F T::*) noexcept {}
    };

    struct Fields {
        int x, y, z;
    };

    struct Complex {
        Fields xyz;
        float num = 0;
        up::string name;
        up::vector<int> vec;
    };

    UP_REFLECT_TYPE(Fields) {
        reflect("x", &Fields::x);
        reflect("y", &Fields::y);
        reflect("z", &Fields::z);
    }

    UP_REFLECT_TYPE(Complex) {
        reflect("xyz", &Complex::xyz);
        reflect("num", &Complex::num, up::reflex::JsonName("custom"));
        reflect("name", &Complex::name);
        reflect("vec", &Complex::vec);
    }
} // namespace

DOCTEST_TEST_SUITE("[potato][reflect] serialize") {
    using namespace up;
    using namespace up::reflex;

    DOCTEST_TEST_CASE("serialize struct to json") {
        auto root = nlohmann::json::object();
        auto serializer = JsonStreamSerializer{root};

        auto const big = Complex{{1, 2, 3}, 42.f, "bob", {4, 5, 6}};

        serialize(big, serializer);

        std::ostringstream ostr;
        ostr << root;

        DOCTEST_CHECK_EQ(ostr.str(), R"--({"custom":42.0,"name":"bob","vec":[4,5,6],"xyz":{"x":1,"y":2,"z":3}})--");
    }

    DOCTEST_TEST_CASE("deserialize struct from json") {
        auto root = nlohmann::json::parse(R"--({"name":"bob","custom":42.0,"ignore":{"num":7},"xyz":{"x":1,"y":2,"z":3},"vec":[4,6]})--");
        auto serializer = JsonStreamDeserializer{root};

        Complex big;
        serialize(big, serializer);

        DOCTEST_CHECK_EQ(1, big.xyz.x);
        DOCTEST_CHECK_EQ(2, big.xyz.y);
        DOCTEST_CHECK_EQ(3, big.xyz.z);

        DOCTEST_CHECK_EQ(big.num, 42.f);

        DOCTEST_CHECK_EQ(big.name, "bob");

        DOCTEST_CHECK_EQ(big.vec.size(), 2);
        DOCTEST_CHECK_EQ(big.vec.front(), 4);
        DOCTEST_CHECK_EQ(big.vec.back(), 6);
    }
}

DOCTEST_TEST_SUITE("[potato][reflect] reflect") {
    using namespace up;
    using namespace up::reflex;

    DOCTEST_TEST_CASE("reflect int") {
        RecordingReflector ref{};
        reflect<int>(ref);
    }

    DOCTEST_TEST_CASE("reflect struct") {
        RecordingReflector ref{};

        reflect<Fields>(ref);
    }
}
