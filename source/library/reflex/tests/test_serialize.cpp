#include "potato/reflex/reflect.h"
#include "potato/reflex/serializer.h"
#include "potato/reflex/json_serializer.h"
#include <doctest/doctest.h>
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

    struct Bigger {
        Fields xyz;
        float num;
        up::string_view name;
    };

    struct Complex {
        Fields xyz;
        Fields abc;
        up::string name;
    };

    UP_REFLECT_TYPE(Fields) {
        reflect("x", &Fields::x);
        reflect("y", &Fields::y);
        reflect("z", &Fields::z);
    }

    UP_REFLECT_TYPE(Bigger) {
        reflect("xyz", &Bigger::xyz);
        reflect("num", &Bigger::num);
        reflect("name", &Bigger::name);
    }

    UP_REFLECT_TYPE(Complex) {
        reflect("xyz", &Complex::xyz);
        reflect("abc", &Complex::abc);
        reflect("name", &Complex::name);
    }
} // namespace

DOCTEST_TEST_SUITE("[potato][reflect] serialize") {
    using namespace up;
    using namespace up::reflex;

    DOCTEST_TEST_CASE("serialize int") {
        int i = 1;
        RecordingSerializer s{};
        serialize(i, s);
    }

    DOCTEST_TEST_CASE("serialize simple struct to json") {
        auto root = nlohmann::json::object();
        auto serializer = JsonStreamSerializer{root};

        auto const xyz = Fields{1, 2, 3};

        serialize(xyz, serializer);

        std::ostringstream ostr;
        ostr << root;

        DOCTEST_CHECK_EQ(ostr.str(), R"--({"x":1,"y":2,"z":3})--");
    }

    DOCTEST_TEST_CASE("serialize bigger struct to json") {
        auto root = nlohmann::json::object();
        auto serializer = JsonStreamSerializer{root};

        auto const big = Bigger{
            {1, 2, 3},
            42.f,
            "bob"};

        serialize(big, serializer);

        std::ostringstream ostr;
        ostr << root;

        DOCTEST_CHECK_EQ(ostr.str(), R"--({"name":"bob","num":42.0,"xyz":{"x":1,"y":2,"z":3}})--");
    }

    DOCTEST_TEST_CASE("deserialize simple struct from json") {
        auto root = nlohmann::json::parse(R"--({"x":1,"y":2,"z":3})--");
        auto serializer = JsonStreamDeserializer{root};

        Fields xyz;
        serialize(xyz, serializer);

        DOCTEST_CHECK_EQ(1, xyz.x);
        DOCTEST_CHECK_EQ(2, xyz.y);
        DOCTEST_CHECK_EQ(3, xyz.z);

        root = nlohmann::json::parse(R"--({"z":7,"x":3})--");

        serialize(xyz, serializer);

        DOCTEST_CHECK_EQ(3, xyz.x);
        DOCTEST_CHECK_EQ(2, xyz.y);
        DOCTEST_CHECK_EQ(7, xyz.z);
    }

    DOCTEST_TEST_CASE("deserialize bigger struct from json") {
        auto root = nlohmann::json::parse(R"--({"name":"bob","num":42.0,"xyz":{"x":1,"y":2,"z":3}})--");
        auto serializer = JsonStreamDeserializer{root};

        Bigger big;
        serialize(big, serializer);

        DOCTEST_CHECK_EQ(big.xyz.z, 3);
        DOCTEST_CHECK_EQ(big.num, 42.f);
        DOCTEST_CHECK(big.name.empty());
    }

    DOCTEST_TEST_CASE("deserialize complex struct from json") {
        auto root = nlohmann::json::parse(R"--({"name":"bob","abc":{"x":-4,"y":-5,"z":-6},"xyz":{"x":1,"y":2,"z":3}})--");
        auto serializer = JsonStreamDeserializer{root};

        Complex cmp;
        serialize(cmp, serializer);

        DOCTEST_CHECK_EQ(cmp.xyz.z, 3);
        DOCTEST_CHECK_EQ(cmp.abc.z, -6);
        DOCTEST_CHECK_EQ(cmp.name, "bob");
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