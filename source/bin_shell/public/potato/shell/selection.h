// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#include <potato/ecs/common.h>

namespace up::shell {
    class Selection {
    public:
        void select(EntityId entityId) noexcept { _selected = entityId; }
        auto hasSelection() const noexcept -> bool { return _selected != EntityId::None; }
        auto selected() const noexcept -> EntityId { return _selected; }

    private:
        EntityId _selected = EntityId::None;
    };
} // namespace up::shell
