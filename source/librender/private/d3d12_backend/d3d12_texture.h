// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"
#include "gpu_texture.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/box.h"

namespace up::d3d12 {
    class TextureD3D12 final : public GpuTexture {
    public:
        explicit TextureD3D12();
        virtual ~TextureD3D12();

        TextureD3D12(TextureD3D12&&) = delete;
        TextureD3D12& operator=(TextureD3D12&&) = delete;

        bool create(ID3DDevicePtr device); 

        GpuTextureType type() const noexcept override;
        GpuFormat format() const noexcept override;
        glm::ivec3 dimensions() const noexcept override;

        DXGI_FORMAT nativeFormat() const noexcept;
        ID3DResourcePtr const& get() const { return _texture; }

    private:
        ID3DResourcePtr _texture;
    };
} // namespace up::d3d12
