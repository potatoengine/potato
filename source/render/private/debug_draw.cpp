// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "debug_draw.h"
#include <mutex>
#include <grimm/foundation/vector.h>
#include <grimm/gpu/buffer.h>
#include <grimm/gpu/command_list.h>
#include <grimm/gpu/device.h>
#include <grimm/gpu/pipeline_state.h>

struct DebugVert {
    glm::vec3 position;
    glm::vec4 color;
    float linger;
};

static std::mutex debugLock;
static gm::vector<DebugVert> debugVertices;

void GM_VECTORCALL gm::drawDebugLine(glm::vec3 start, glm::vec3 end, glm::vec4 color, float lingerSeconds) {
    std::unique_lock _(debugLock);

    debugVertices.push_back({start, color, lingerSeconds});
    debugVertices.push_back({end, color, lingerSeconds});
}

void gm::flushDebugDraw(gpu::Device& device, gpu::CommandList& commandList, gpu::Buffer& buffer, float frameTime) {
    uint32 drawCount = 0;

    {
        std::unique_lock _(debugLock);
        commandList.update(&buffer, span{debugVertices.data(), debugVertices.size()}.as_bytes());
        drawCount = static_cast<uint32>(debugVertices.size());

        decltype(debugVertices.size()) outCount = 0;
        for (auto const& vert : debugVertices) {
            if (vert.linger >= frameTime) {
                debugVertices[outCount] = vert;
                debugVertices[outCount++].linger -= frameTime;
            }
        }

        debugVertices.resize(outCount);
    }

    commandList.bindVertexBuffer(0, &buffer, sizeof(DebugVert));
    commandList.setPrimitiveTopology(gpu::PrimitiveTopology::Lines);
    commandList.draw(drawCount);
}
