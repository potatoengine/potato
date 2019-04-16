#include "potato/ecs/archetype.h"
#include <doctest/doctest.h>

struct Test1 {
    char a;
};
struct Second {
    char a;
    float b;
};
struct Another {
    float a;
    float b;
};

DOCTEST_TEST_SUITE("[potato][ecs] Archetype") {
    using namespace up;

    DOCTEST_TEST_CASE("Archetype queries") {
        Archetype arch1{vector{getComponentId<Test1>(), getComponentId<Second>()}};
        Archetype arch2{vector{getComponentId<Second>(), getComponentId<Another>()}};

        DOCTEST_CHECK(arch1.matches(view<ComponentId>({getComponentId<Test1>()})));
        DOCTEST_CHECK(arch1.matches(view<ComponentId>({getComponentId<Test1>(), getComponentId<Second>()})));
        DOCTEST_CHECK(arch1.matches(view<ComponentId>({getComponentId<Second>()})));
        DOCTEST_CHECK_FALSE(arch1.matches(view<ComponentId>({getComponentId<Another>()})));

        DOCTEST_CHECK_FALSE(arch2.matches(view<ComponentId>({getComponentId<Test1>()})));
        DOCTEST_CHECK_FALSE(arch2.matches(view<ComponentId>({getComponentId<Test1>(), getComponentId<Second>()})));
        DOCTEST_CHECK(arch2.matches(view<ComponentId>({getComponentId<Second>()})));
        DOCTEST_CHECK(arch2.matches(view<ComponentId>({getComponentId<Another>()})));
    }

    DOCTEST_TEST_CASE("Archetype selects") {
        Archetype arch{vector{getComponentId<Test1>(), getComponentId<Second>()}};

        arch.allocateEntity();
        arch.allocateEntity();
        arch.allocateEntity();
        arch.allocateEntity();
        arch.allocateEntity();

        size_t total = 0;
        arch.unsafeSelect(view<ComponentId>({getComponentId<Test1>(), getComponentId<Second>()}), [&total](size_t count, view<void*> arrays) {
            total += count;
        });
        DOCTEST_CHECK_EQ(5, total);
    }
}
