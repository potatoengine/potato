// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/component.h"
#include "potato/ecs/entity.h"

UP_DEFINE_COMPONENT(up::Entity);

auto up::ComponentMeta::allocateId() noexcept -> ComponentMeta& {
    static std::atomic<std::underlying_type_t<ComponentId>> _next{0};
    id = static_cast<ComponentId>(++_next);
    return *this;
}
