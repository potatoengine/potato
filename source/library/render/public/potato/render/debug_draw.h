// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/box.h"
#include "potato/foundation/platform.h"
#include "potato/foundation/delegate_ref.h"
#include "potato/foundation/span.h"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace up {

    struct DebugDrawVertex {
        glm::vec3 position;
        glm::vec4 color;
        float linger;
    };

    UP_RENDER_API void UP_VECTORCALL drawDebugLine(glm::vec3 start, glm::vec3 end, glm::vec4 color = {1, 0, 0, 1}, float lingerSeconds = 0.f);
    UP_RENDER_API void dumpDebugDraw(delegate_ref<void(view<DebugDrawVertex>)> callback);
    UP_RENDER_API void flushDebugDraw(float frameTime);

} // namespace up
