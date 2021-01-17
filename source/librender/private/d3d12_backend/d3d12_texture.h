// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"
#include "gpu_texture.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/box.h"

namespace up::d3d12 {
    class ContextD3D12;
    class DescriptorHeapD3D12;

    class TextureD3D12 final : public GpuTexture {
    public:
        explicit TextureD3D12();
        explicit TextureD3D12(ID3DResourcePtr buffer);
        virtual ~TextureD3D12();

        TextureD3D12(TextureD3D12&&) = delete;
        TextureD3D12& operator=(TextureD3D12&&) = delete;

        bool create(ContextD3D12 const& ctx, GpuTextureDesc const& desc, span<up::byte const> data); 

        GpuTextureType type() const noexcept override;
        GpuFormat format() const noexcept override;
        glm::ivec3 dimensions() const noexcept override;

        DXGI_FORMAT nativeFormat() const noexcept;
        ID3DResourcePtr const& get() const { return _texture; }

        DescriptorHeapD3D12* desc() const { return _cbvHeap.get(); }

    private:
        ID3DResourcePtr _texture;
        com_ptr<D3D12MA::Allocation> _allocation;

        ID3DResourcePtr _uploadTexture;
        com_ptr<D3D12MA::Allocation> _uploadAlloc;

         // const buffer bits for now here while testing
        box<DescriptorHeapD3D12> _cbvHeap;

        DXGI_FORMAT _format; 
    
    };
} // namespace up::d3d12
