#include "potato/ecs/world.h"
#include "potato/ecs/query.h"
#include <doctest/doctest.h>

namespace {
    struct Test1 {
        char a;
    };
    struct Second {
        float b;
        char a;
    };
    struct Another {
        double a;
        float b;
    };

    UP_REFLECT_TYPE(Test1) {}
    UP_REFLECT_TYPE(Second) {}
    UP_REFLECT_TYPE(Another) {}
} // namespace

UP_DECLARE_COMPONENT(Test1);
UP_DECLARE_COMPONENT(Second);
UP_DECLARE_COMPONENT(Another);

UP_DEFINE_COMPONENT(Test1);
UP_DEFINE_COMPONENT(Second);
UP_DEFINE_COMPONENT(Another);

DOCTEST_TEST_SUITE("[potato][ecs] Query") {
    using namespace up;

    DOCTEST_TEST_CASE("Select chunks") {
        World world;

        world.createEntity(Test1{'f'}, Second{7.f, 'g'});
        world.createEntity(Another{1.f, 2.f}, Second{9.f, 'g'});
        world.createEntity(Second{-2.f, 'h'}, Another{2.f, 1.f});
        world.createEntity(Test1{'j'}, Another{3.f, 4.f});

        // Exactly two of the entities should be in the same archetype; and the empty Archetype, for 4
        DOCTEST_CHECK_EQ(4, world.archetypes().archetypes());

        size_t invokeCount = 0;
        size_t entityCount = 0;
        float weight = 0;

        Query<Second> query;
        query.selectChunks(world, [&](size_t count, EntityId const*, Second* second) {
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

    DOCTEST_TEST_CASE("Select entities") {
        World world;

        world.createEntity(Second{1.f, 'g'});
        world.createEntity(Second{2.f, 'g'});
        world.createEntity(Second{3.f, 'g'});
        world.createEntity(Second{4.f, 'g'});

        Query<Second> query;
        float sum = 0;
        int count = 0;
        query.select(world, [&](EntityId, Second const& second) {
            ++count;
            sum += second.b;
        });

        DOCTEST_CHECK_EQ(4, count);
        DOCTEST_CHECK_EQ(10.f, sum);
    }
}
