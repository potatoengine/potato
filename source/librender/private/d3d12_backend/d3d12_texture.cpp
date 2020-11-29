// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_texture.h"
#include "d3d12_platform.h"
#include "d3d12_utils.h"
#include "d3d12_context.h"

#include "potato/runtime/assertion.h"
#include "potato/runtime/com_ptr.h"
#include "potato/spud/out_ptr.h"

#include <d3d12.h>

up::d3d12::TextureD3D12::TextureD3D12() {
}

up::d3d12::TextureD3D12::TextureD3D12(ID3DResourcePtr buffer)
    : _texture(buffer) {

}

up::d3d12::TextureD3D12::~TextureD3D12() {
    _allocation->Release();
    _allocation = nullptr;
}

bool up::d3d12::TextureD3D12::create(ContextD3D12 const& ctx, GpuTextureDesc const& desc, span<up::byte const> data) {

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = desc.width;
    resourceDesc.Height = desc.height;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = toNative(desc.format);
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    D3D12MA::ALLOCATION_DESC allocDesc = {};
    allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

    
    HRESULT hr = ctx._allocator->CreateResource(
        &allocDesc,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        NULL,
        &_allocation,
        __uuidof(ID3D12Resource),
        out_ptr(_texture));

    //ctx._cmdList->ResourceBarrier(
    //    1,
    //    &CD3DX12_RESOURCE_BARRIER::Transition(
    //        _texture.Get(),
    //        D3D12_RESOURCE_STATE_COPY_DEST,
    //        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    //// Describe and create a SRV for the texture.
    //D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    //srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    //srvDesc.Format = Convert(desc.format);
    //srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    //srvDesc.Texture2D.MipLevels = 1;
    //ctx._device->CreateShaderResourceView(_texture.Get(), &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());

    return true;
}

auto up::d3d12::TextureD3D12::type() const noexcept -> GpuTextureType {
    // @dx12 @todo integrate proper texture types
    return GpuTextureType::Texture2D;
}

auto up::d3d12::TextureD3D12::format() const noexcept -> GpuFormat {
    return fromNative(nativeFormat());
}

DXGI_FORMAT up::d3d12::TextureD3D12::nativeFormat() const noexcept {
    // @dx12 @todo
    return DXGI_FORMAT_UNKNOWN;
}

auto up::d3d12::TextureD3D12::dimensions() const noexcept -> glm::ivec3 {
    // @dx12 @todo
    return {0, 0, 0};
}
