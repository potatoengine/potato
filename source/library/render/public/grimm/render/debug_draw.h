// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include <grimm/foundation/box.h>
#include <grimm/foundation/platform.h>
#include <grimm/foundation/delegate_ref.h>
#include <grimm/foundation/span.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace gm {

    struct DebugDrawVertex {
        glm::vec3 position;
        glm::vec4 color;
        float linger;
    };

    GM_RENDER_API void GM_VECTORCALL drawDebugLine(glm::vec3 start, glm::vec3 end, glm::vec4 color = {1, 0, 0, 1}, float lingerSeconds = 0.f);
    GM_RENDER_API void dumpDebugDraw(delegate_ref<void(view<DebugDrawVertex>)> callback);
    GM_RENDER_API void flushDebugDraw(float frameTime);

} // namespace gm
