// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/component.h"

auto up::ComponentMeta::allocateId() noexcept -> ComponentId {
    static std::atomic<std::underlying_type_t<ComponentId>> _next{0};
    return static_cast<ComponentId>(++_next);
}
