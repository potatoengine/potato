// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "component.h"
#include "common.h"

namespace up {
    struct Entity {
        EntityId id = EntityId::None;
    };
} // namespace up

UP_DECLARE_COMPONENT(up::Entity, UP_ECS_API);
