// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "component.h"
#include "shared_context.h"
#include "world.h"

#include "potato/spud/box.h"
#include "potato/spud/hash.h"
#include "potato/spud/zstring_view.h"

namespace up {

    class ComponentRegistry;
    class World;
    template <typename...>
    class Query;

    /// @brief Manages all of the state and data for all worlds in the ECS implementation
    class Universe {
    public:
        UP_ECS_API Universe();
        UP_ECS_API ~Universe();

        auto createWorld() noexcept -> World { return World(_context); }

        template <typename... Components>
        auto createQuery() -> Query<Components...> {
            return Query<Components...>(_context);
        }

        template <typename Component>
        void registerComponent(zstring_view name);

        auto components() const noexcept -> view<ComponentMeta> { return _context->components; }

    private:
        UP_ECS_API void _registerComponent(ComponentMeta const& meta);

        rc<EcsSharedContext> _context;
    };

    template <typename Component>
    void Universe::registerComponent(zstring_view name) {
        auto const meta = ComponentMeta{
            .name = name,
            .ops =
                {
                    .defaultConstruct = _detail::ComponentDefaultMetaOps<Component>::defaultConstruct,
                    .copyConstruct = _detail::ComponentDefaultMetaOps<Component>::copyConstruct,
                    .moveAssign = _detail::ComponentDefaultMetaOps<Component>::moveAssign,
                    .destruct = _detail::ComponentDefaultMetaOps<Component>::destruct,
                    .serialize = _detail::ComponentDefaultMetaOps<Component>::serialize,
                },
            .id = to_enum<ComponentId>(hash_value(name)),
            .typeHash = typeid(Component).hash_code(),
            .size = sizeof(Component),
            .alignment = alignof(Component)};
        _registerComponent(meta);
    }
} // namespace up
