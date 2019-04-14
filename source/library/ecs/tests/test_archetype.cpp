#include "potato/ecs/archetype.h"
#include <doctest/doctest.h>

struct Test1 {};
struct Second {};
struct Another {};

DOCTEST_TEST_SUITE("[potato][ecs] Archetype") {
    using namespace up;

    DOCTEST_TEST_CASE("Archetype queries") {
        Archetype arch1{getComponentId<Test1>(), getComponentId<Second>()};
        Archetype arch2{getComponentId<Second>(), getComponentId<Another>()};

        Query queryTest1{getComponentId<Test1>()};
        Query queryTest1Second{getComponentId<Test1>(), getComponentId<Second>()};
        Query querySecond{getComponentId<Second>()};
        Query queryAnother{getComponentId<Another>()};

        DOCTEST_CHECK(arch1.matches(queryTest1));
        DOCTEST_CHECK(arch1.matches(queryTest1Second));
        DOCTEST_CHECK(arch1.matches(querySecond));
        DOCTEST_CHECK_FALSE(arch1.matches(queryAnother));

        DOCTEST_CHECK_FALSE(arch2.matches(queryTest1));
        DOCTEST_CHECK_FALSE(arch2.matches(queryTest1Second));
        DOCTEST_CHECK(arch2.matches(querySecond));
        DOCTEST_CHECK(arch2.matches(queryAnother));
    }
}
