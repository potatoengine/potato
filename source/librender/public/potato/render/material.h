// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/spud/box.h"
#include "potato/spud/rc.h"
#include "potato/spud/vector.h"

namespace up {
    class CommandList;
    class GpuDevice;
    class GpuPipelineState;
    class GpuResourceView;
    class GpuSampler;
    class RenderContext;
    class Shader;
    class Texture;
    class Loader;

    class Material : public shared<Material> {
    public:
        UP_RENDER_API explicit Material(rc<Shader> vertexShader, rc<Shader> pixelShader, vector<rc<Texture>> textures);
        UP_RENDER_API ~Material();

        static UP_RENDER_API auto createFromBuffer(view<byte> buffer, Loader& loader) -> rc<Material>;

        UP_RENDER_API void bindMaterialToRender(RenderContext& ctx);

    private:
        box<GpuPipelineState> _state;
        rc<Shader> _vertexShader;
        rc<Shader> _pixelShader;
        vector<rc<Texture>> _textures;
        vector<box<GpuResourceView>> _srvs;
        vector<box<GpuSampler>> _samplers;
    };
} // namespace up
