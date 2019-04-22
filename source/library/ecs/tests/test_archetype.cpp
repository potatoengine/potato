#include "potato/ecs/archetype.h"
#include "potato/ecs/domain.h"
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
        EntityDomain domain;
        Archetype arch1{domain, vector{getComponentId<Test1>(), getComponentId<Second>()}};
        Archetype arch2{domain, vector{getComponentId<Second>(), getComponentId<Another>()}};

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
        EntityDomain domain;
        Archetype arch{domain, vector{getComponentId<Test1>(), getComponentId<Second>()}};

        arch.allocate<Test1, Second>(makeEntityId(0, 0), {}, {});
        arch.allocate<Test1, Second>(makeEntityId(1, 0), {}, {});
        arch.allocate<Test1, Second>(makeEntityId(2, 0), {}, {});
        arch.allocate<Test1, Second>(makeEntityId(3, 0), {}, {});
        arch.allocate<Test1, Second>(makeEntityId(4, 0), {}, {});

        size_t total = 0;
        arch.unsafeSelect(view<ComponentId>({getComponentId<Test1>(), getComponentId<Second>()}), [&total](size_t count, view<void*> arrays) {
            total += count;
        });
        DOCTEST_CHECK_EQ(5, total);
    }

    DOCTEST_TEST_CASE("Chunks") {
        int const count = 100000;
        EntityDomain domain;
        Archetype arch{domain, vector{getComponentId<Test1>(), getComponentId<Second>(), getComponentId<Another>()}};

        for (int i = 0; i != count; ++i) {
            arch.allocate<Test1, Second, Another>(makeEntityId(i, 0), {}, {}, {});
        }

        size_t chunks = 0;
        size_t total = 0;
        arch.unsafeSelect(view<ComponentId>({getComponentId<Second>()}), [&](size_t count, view<void*> arrays) {
            ++chunks;
            total += count;
        });
        DOCTEST_CHECK_NE(0, chunks);
        DOCTEST_CHECK_EQ(count, total);
    }

    DOCTEST_TEST_CASE("Deletes") {
        EntityDomain domain;
        Archetype arch{domain, vector{getComponentId<Test1>()}};

        arch.allocate<Test1>(makeEntityId(0, 0), {'a'});
        arch.allocate<Test1>(makeEntityId(1, 0), {'b'});
        arch.allocate<Test1>(makeEntityId(2, 0), {'c'});
        arch.allocate<Test1>(makeEntityId(3, 0), {'d'});
        arch.allocate<Test1>(makeEntityId(4, 0), {'e'});

        arch.unsafeRemoveEntity(2);
        arch.unsafeRemoveEntity(0);

        DOCTEST_CHECK_EQ('d', static_cast<Test1*>(arch.unsafeComponentPointer(0, getComponentId<Test1>()))->a);
        DOCTEST_CHECK_EQ('b', static_cast<Test1*>(arch.unsafeComponentPointer(1, getComponentId<Test1>()))->a);
        DOCTEST_CHECK_EQ('e', static_cast<Test1*>(arch.unsafeComponentPointer(2, getComponentId<Test1>()))->a);
    }
}
