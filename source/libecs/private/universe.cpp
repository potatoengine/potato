// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "universe.h"
#include "shared_context.h"
#include "world.h"

#include "potato/runtime/assertion.h"

up::Universe::Universe() : _context(new_shared<EcsSharedContext>()) {}
up::Universe::~Universe() = default;

void up::Universe::_registerComponent(reflex::TypeInfo const& typeInfo) {
    UP_ASSERT(_context->_findComponentByTypeHash(typeInfo.hash) == nullptr);
    auto& newMeta = _context->components.push_back(&typeInfo);
}
