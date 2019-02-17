// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include <grimm/foundation/box.h>
#include <grimm/foundation/platform.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace gm::gpu {
    class CommandList;
    class Device;
    class PipelineState;
    class Buffer;
} // namespace gm::gpu

namespace gm {

    GM_RENDER_API void GM_VECTORCALL drawDebugLine(glm::vec3 start, glm::vec3 end, glm::vec4 color = {1, 0, 0, 1}, float lingerSeconds = 0.f);
    GM_RENDER_API void flushDebugDraw(gpu::Device& device, gpu::CommandList& commandList, gpu::Buffer& buffer, float frameTime);

} // namespace gm
