// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/runtime/asset_loader.h"
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

    class Material : public Asset {
    public:
        static constexpr zstring_view assetTypeName = "potato.asset.material"_zsv;

        UP_RENDER_API explicit Material(
            ResourceId id,
            rc<Shader> vertexShader,
            rc<Shader> pixelShader,
            vector<rc<Texture>> textures);
        UP_RENDER_API ~Material() override;

        static UP_RENDER_API auto createFromBuffer(ResourceId id, view<byte> buffer, AssetLoader& assetLoader)
            -> rc<Material>;

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
