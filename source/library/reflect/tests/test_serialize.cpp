#include "potato/reflect/reflect.h"
#include "potato/reflect/stream_serializer.h"
#include <doctest/doctest.h>

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

    DOCTEST_TEST_CASE("serialize to xml") {
        string_writer xml;
        XmlStreamSerializer serializer(xml);

        Fields xyz{1, 2, 3};

        serialize(xyz, serializer);

        DOCTEST_CHECK_EQ(xml.c_str(), "<field name=\"x\"><int>1</int></field><field name=\"y\"><int>2</int></field><field name=\"z\"><int>3</int></field>");

        Bigger big{
            {1, 2, 3},
            42.f,
            "bob"};

        xml.clear();

        serialize(big, serializer);

        DOCTEST_CHECK_EQ(xml.c_str(), "<field name=\"xyz\"><field name=\"x\"><int>1</int></field><field name=\"y\"><int>2</int></field><field name=\"z\"><int>3</int></field></field><field name=\"num\"><float>42.000000</float></field><field name=\"name\"><string>bob</string></field>");
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
