#include "potato/reflect/reflect.h"
#include "potato/reflect/stream_serializer.h"
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
}

DOCTEST_TEST_SUITE("[potato][reflect] serialize") {
    using namespace up;
    using namespace up::reflex;

    DOCTEST_TEST_CASE("serialize int") {
        int i = 1;
        RecordingSerializer s{};
        serialize(i, s);
    }

    DOCTEST_TEST_CASE("serialize to json") {
        nlohmann::json root = nlohmann::json::object();
        JsonStreamSerializer serializer(root);

        Fields xyz{1, 2, 3};

        serialize(xyz, serializer);

        std::ostringstream ostr;
        ostr << root;

        DOCTEST_CHECK_EQ(ostr.str(), R"--({"x":1,"y":2,"z":3})--");

        Bigger big{
            {1, 2, 3},
            42.f,
            "bob"};

        root = nlohmann::json::object();
        serialize(big, serializer);

        ostr = std::ostringstream();
        ostr << root;

        DOCTEST_CHECK_EQ(ostr.str(), R"--({"name":"bob","num":42.0,"xyz":{"x":1,"y":2,"z":3}})--");
    }

    DOCTEST_TEST_CASE("deserialize from json") {
        auto root = nlohmann::json::parse(R"--({"x":1,"y":2,"z":3})--");

        JsonStreamDeserializer serializer(root);

        Fields xyz;
        serialize(xyz, serializer);

        DOCTEST_CHECK_EQ(1, xyz.x);
        DOCTEST_CHECK_EQ(2, xyz.y);
        DOCTEST_CHECK_EQ(3, xyz.z);
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
