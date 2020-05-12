// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#include "potato/ecs/_detail/ecs_context.h"
#include "potato/ecs/universe.h"
#include "potato/ecs/world.h"
#include <potato/runtime/assertion.h>

up::Universe::Universe() : _context(new_box<_detail::EcsContext>()) {}
up::Universe::~Universe() = default;

void up::Universe::_registerComponent(ComponentMeta const& meta) {
    UP_ASSERT(_context->findComponentById(meta.id) == nullptr);
    auto& newMeta = _context->components.push_back(meta);
    newMeta.index = static_cast<uint32>(_context->components.size()) - 1;
}
