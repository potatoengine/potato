#include "potato/ecs/world.h"
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

DOCTEST_TEST_SUITE("[potato][ecs] World") {
    using namespace up;

    DOCTEST_TEST_CASE("Archetype selects") {
        World world;

        world.createEntity(Test1{'f'}, Second{'g', 7.f});
        world.createEntity(Another{1.f, 2.f}, Second{'g', 9.f});
        world.createEntity(Second{'h', -2.f}, Another{2.f, 1.f});
        world.createEntity(Test1{'j'}, Another{3.f, 4.f});

        // Exactly two of the entities should be in the same archetype
        DOCTEST_CHECK_EQ(3, world.archetypes().size());

        size_t invokeCount = 0;
        size_t entityCount = 0;
        float weight = 0;

        world.select<Second>([&](size_t count, Second* second) {
            ++invokeCount;
            entityCount += count;

            for (size_t index = 0; index != count; ++index) {
                weight += second[index].b;
            }
        });

        // Only two archetypes should have matches
        DOCTEST_CHECK_EQ(2, invokeCount);

        // Three total entities between the two archetypes should exist that match
        DOCTEST_CHECK_EQ(3, entityCount);

        // Ensure we're storing/retrieving correct values
        DOCTEST_CHECK_EQ(14.f, weight);
    }

    DOCTEST_TEST_CASE("Direct component access") {
        World world;

        world.createEntity(Test1{'f'}, Second{'g', 7.f});
        world.createEntity(Another{1.f, 2.f}, Second{'g', 9.f});
        auto id = world.createEntity(Second{'h', -2.f}, Another{2.f, 1.f});
        world.createEntity(Test1{'j'}, Another{3.f, 4.f});

        Second* test = world.getComponentSlow<Second>(id);
        DOCTEST_CHECK(test != nullptr);
        DOCTEST_CHECK_EQ('h', test->a);
        DOCTEST_CHECK_EQ(-2.f, test->b);
    }

    DOCTEST_TEST_CASE("Creates and Deletes") {
        World world;

        EntityId foo = world.createEntity(Test1{'a'});
        world.createEntity(Test1{'b'});
        EntityId bar = world.createEntity(Test1{'c'});
        world.createEntity(Test1{'d'});
        world.createEntity(Test1{'e'});

        world.deleteEntity(foo);
        world.deleteEntity(bar);
    }
}
