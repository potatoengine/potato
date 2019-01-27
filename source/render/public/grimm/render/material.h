// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/rc.h"
#include "grimm/foundation/blob.h"

namespace gm::gpu {
    class CommandList;
    class Device;
    class PipelineState;
} // namespace gm::gpu

namespace gm {
    class Material : public shared<Material> {
    public:
        GM_RENDER_API explicit Material(blob vertexShader, blob pixelShader);
        GM_RENDER_API ~Material();

        GM_RENDER_API void bindMaterialToRender(gpu::CommandList& commandList, gpu::Device& device);

    private:
        box<gpu::PipelineState> _state;
        blob _vertexShader;
        blob _pixelShader;
    };
} // namespace gm
