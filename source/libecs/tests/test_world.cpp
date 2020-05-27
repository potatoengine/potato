// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/ecs/query.h"
#include "potato/ecs/universe.h"
#include "potato/ecs/world.h"

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
    struct Counter {
        int value;
        char padding[128];
    };

    UP_REFLECT_TYPE(Test1) {}
    UP_REFLECT_TYPE(Second) {}
    UP_REFLECT_TYPE(Another) {}
    UP_REFLECT_TYPE(Counter) {}
} // namespace

DOCTEST_TEST_SUITE("[potato][ecs] World") {
    using namespace up;

    DOCTEST_TEST_CASE("") {
        Universe universe;

        universe.registerComponent<Test1>("Test1");
        universe.registerComponent<Second>("Second");
        universe.registerComponent<Another>("Another");
        universe.registerComponent<Counter>("Counter");

        DOCTEST_SUBCASE("Direct component access") {
            auto world = universe.createWorld();

            world.createEntity(Test1{'f'}, Second{7.f, 'g'});
            world.createEntity(Another{1.f, 2.f}, Second{9.f, 'g'});
            auto id = world.createEntity(Second{-2.f, 'h'}, Another{2.f, 1.f});

            Second* test = world.getComponentSlow<Second>(id);
            DOCTEST_CHECK(test != nullptr);
            DOCTEST_CHECK_EQ('h', test->a);
            DOCTEST_CHECK_EQ(-2.f, test->b);
        }

        DOCTEST_SUBCASE("EntityId management") {
            auto world = universe.createWorld();

            EntityId first = world.createEntity(Test1{'f'}, Second{7.f, 'g'});
            EntityId second = world.createEntity(Test1{'h'}, Second{-1.f, 'i'});

            auto query = universe.createQuery<Test1>();
            query.selectChunks(world, [&](size_t count, EntityId const* entities, Test1*) {
                DOCTEST_CHECK_EQ(2, count);
                DOCTEST_CHECK_EQ(first, entities[0]);
                DOCTEST_CHECK_EQ(second, entities[1]);
            });
        }

        DOCTEST_SUBCASE("Chunks") {
            constexpr int count = 100000;
            auto world = universe.createWorld();

            uint64 expectedSum = 0;
            for (int i = 0; i != count; ++i) {
                expectedSum += i;
                world.createEntity(Counter{i});
            }

            size_t chunks = 0;
            size_t total = 0;
            uint64 sum = 0;

            auto query = universe.createQuery<Counter>();
            query.selectChunks(world, [&](size_t count, EntityId const*, Counter* counters) {
                ++chunks;
                total += count;
                for (size_t i = 0; i != count; ++i) {
                    sum += counters[i].value;
                }
            });

            DOCTEST_CHECK_NE(0, chunks);
            DOCTEST_CHECK_EQ(count, total);
            DOCTEST_CHECK_EQ(expectedSum, sum);
        }

        DOCTEST_SUBCASE("Creates and Deletes") {
            auto world = universe.createWorld();

            // create some dummy entities
            //
            EntityId foo = world.createEntity(Test1{'a'});
            world.createEntity(Test1{'b'});
            EntityId bar = world.createEntity(Test1{'c'});
            world.createEntity(Test1{'d'});
            EntityId last = world.createEntity(Test1{'e'});

            Test1* fooTest = world.getComponentSlow<Test1>(foo);
            DOCTEST_CHECK_NE(nullptr, fooTest);

            // delete some entities (not the last one!)
            //
            world.deleteEntity(foo);
            world.deleteEntity(bar);

            // ensure deleted entities are gone
            //
            DOCTEST_CHECK_EQ(nullptr, world.getComponentSlow<Test1>(foo));
            DOCTEST_CHECK_EQ(nullptr, world.getComponentSlow<Test1>(bar));

            // overwrite emptied locations
            //
            world.createEntity(Test1{'x'});
            world.createEntity(Test1{'x'});

            // ensure that the first deleted entity was overwritten properly
            //
            DOCTEST_CHECK_EQ('e', fooTest->a);

            // ensure that the last entity was moved properly
            //
            DOCTEST_CHECK_EQ('e', world.getComponentSlow<Test1>(last)->a);
        }

        DOCTEST_SUBCASE("Remove Component") {
            bool found = false;
            auto world = universe.createWorld();
            auto queryTest1 = universe.createQuery<Test1>();
            auto querySecond = universe.createQuery<Second>();

            EntityId id = world.createEntity(Test1{}, Second{});

            querySecond.select(world, [&found](EntityId, Second&) { found = true; });
            DOCTEST_CHECK(found);

            world.removeComponent<Second>(id);

            found = false;
            querySecond.select(world, [&found](EntityId, Second&) { found = true; });
            DOCTEST_CHECK_FALSE(found);

            found = false;
            queryTest1.select(world, [&found](EntityId, Test1&) { found = true; });
            DOCTEST_CHECK(found);
        }

        DOCTEST_SUBCASE("Add Component") {
            bool found = false;
            auto world = universe.createWorld();
            auto queryTest1 = universe.createQuery<Test1>();
            auto querySecond = universe.createQuery<Second>();

            EntityId id = world.createEntity(Test1{});

            querySecond.select(world, [&found](EntityId, Second&) { found = true; });
            DOCTEST_CHECK_FALSE(found);

            world.addComponent(id, Second{});

            found = false;
            querySecond.select(world, [&found](EntityId, Second&) { found = true; });
            DOCTEST_CHECK(found);

            found = false;
            queryTest1.select(world, [&found](EntityId, Test1&) { found = true; });
            DOCTEST_CHECK(found);
        }

        DOCTEST_SUBCASE("Interroate") {
            auto world = universe.createWorld();

            auto id = world.createEntity(Test1{'f'}, Another{1.0, 2.f}, Second{7.f, 'g'});

            auto success = world.interrogateEntityUnsafe(id, [&](auto entity, auto archetype, auto component, auto data) {
                if (component->typeHash == typeid(Test1).hash_code()) {
                    DOCTEST_CHECK_EQ('f', static_cast<Test1 const*>(data)->a);
                }
                else if (component->typeHash == typeid(Another).hash_code()) {
                    DOCTEST_CHECK_EQ(1.0, static_cast<Another const*>(data)->a);
                }
                else if (component->typeHash == typeid(Second).hash_code()) {
                    DOCTEST_CHECK_EQ(7.f, static_cast<Second const*>(data)->b);
                }
                else {
                    DOCTEST_FAIL("unknown component");
                }
            });
            DOCTEST_CHECK(success);
        }
    }
}
