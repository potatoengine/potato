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

static void UP_VECTORCALL _drawDebugLine(glm::vec3 start, glm::vec3 end, glm::vec4 color, float lingerSeconds = 0) {
    debugVertices.push_back({start, color, lingerSeconds});
    debugVertices.push_back({end, color, lingerSeconds});
}

static void UP_VECTORCALL _drawDebugRay(glm::vec3 start, glm::vec3 direction, float length, glm::vec4 color, float lingerSeconds = 0) {
    debugVertices.push_back({start, color, lingerSeconds});
    debugVertices.push_back({start + direction * length, color, lingerSeconds});
}

void UP_VECTORCALL up::drawDebugLine(glm::vec3 start, glm::vec3 end, glm::vec4 color, float lingerSeconds) {
    std::unique_lock _(debugLock);
    _drawDebugLine(start, end, color, lingerSeconds);
}

void UP_VECTORCALL up::drawDebugGrid(DebugDrawGrid const& grid) {
    std::unique_lock _(debugLock);

    auto const xStart = grid.offset - grid.xAxis * static_cast<float>(grid.halfWidth);
    auto const yStart = grid.offset - grid.yAxis * static_cast<float>(grid.halfWidth);
    auto const width = static_cast<float>(grid.halfWidth + grid.halfWidth);

    for (int i = -grid.halfWidth; i <= grid.halfWidth; i += grid.spacing) {
        auto const color = i % grid.guidelineSpacing != 0 ? grid.lineColor : grid.guidelineColor;
        _drawDebugRay(xStart + grid.yAxis * static_cast<float>(i), grid.xAxis, width, color);
        _drawDebugRay(yStart + grid.xAxis * static_cast<float>(i), grid.yAxis, width, color);
    }
}

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
