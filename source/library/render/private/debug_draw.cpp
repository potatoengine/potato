// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/render/debug_draw.h"
#include <grimm/foundation/vector.h>
#include <grimm/gpu/buffer.h>
#include <grimm/gpu/command_list.h>
#include <grimm/gpu/device.h>
#include <grimm/gpu/pipeline_state.h>
#include <mutex>

static std::mutex debugLock;
static gm::vector<gm::DebugDrawVertex> debugVertices;

void GM_VECTORCALL gm::drawDebugLine(glm::vec3 start, glm::vec3 end, glm::vec4 color, float lingerSeconds) {
    std::unique_lock _(debugLock);

    debugVertices.push_back({start, color, lingerSeconds});
    debugVertices.push_back({end, color, lingerSeconds});
}

//gpu::Device &device, gpu::CommandList &commandList, gpu::Buffer &buffer

void gm::dumpDebugDraw(delegate_ref<void(view<DebugDrawVertex>)> callback) {
    std::unique_lock _(debugLock);
    callback(span{debugVertices.data(), debugVertices.size()});
}

void gm::flushDebugDraw(float frameTime) {
    std::unique_lock _(debugLock);

    decltype(debugVertices.size()) outCount = 0;
    for (auto const& vert : debugVertices) {
        if (vert.linger >= frameTime) {
            debugVertices[outCount] = vert;
            debugVertices[outCount++].linger -= frameTime;
        }
    }

    debugVertices.resize(outCount);
}
