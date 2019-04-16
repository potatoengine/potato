#include "potato/ecs/component.h"
#include <doctest/doctest.h>

struct Test1 {
    char a;
};
static_assert(sizeof(Test1) == 1);

struct Second {
    char a;
    float b;
};
static_assert(sizeof(Second) == 8);

struct Another {
    float a;
    float b;
};
static_assert(sizeof(Another) == 8);

DOCTEST_TEST_SUITE("[potato][ecs] Component") {
    using namespace up;

    DOCTEST_TEST_CASE("ComponentInfo<Test1>") {
        ComponentInfo info(getComponentId<Test1>());
        DOCTEST_CHECK_EQ(info.size, 1);
        DOCTEST_CHECK_EQ(info.alignment, 1);
    }

    DOCTEST_TEST_CASE("ComponentInfo<Second>") {
        ComponentInfo info(getComponentId<Second>());
        DOCTEST_CHECK_EQ(info.size, 8);
        DOCTEST_CHECK_EQ(info.alignment, 4);
    }

    DOCTEST_TEST_CASE("ComponentInfo<Another>") {
        ComponentInfo info(getComponentId<Another>());
        DOCTEST_CHECK_EQ(info.size, 8);
        DOCTEST_CHECK_EQ(info.alignment, 4);
    }
}
