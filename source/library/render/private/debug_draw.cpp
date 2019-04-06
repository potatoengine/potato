// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/render/debug_draw.h"
#include "potato/foundation/vector.h"
#include "potato/gpu/buffer.h"
#include "potato/gpu/command_list.h"
#include "potato/gpu/device.h"
#include "potato/gpu/pipeline_state.h"
#include <mutex>

static std::mutex debugLock;
static up::vector<up::DebugDrawVertex> debugVertices;

void UP_VECTORCALL up::drawDebugLine(glm::vec3 start, glm::vec3 end, glm::vec4 color, float lingerSeconds) {
    std::unique_lock _(debugLock);

    debugVertices.push_back({start, color, lingerSeconds});
    debugVertices.push_back({end, color, lingerSeconds});
}

//gpu::Device &device, gpu::CommandList &commandList, gpu::Buffer &buffer

void up::dumpDebugDraw(delegate_ref<void(view<DebugDrawVertex>)> callback) {
    std::unique_lock _(debugLock);
    callback(span{debugVertices.data(), debugVertices.size()});
}

void up::flushDebugDraw(float frameTime) {
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
