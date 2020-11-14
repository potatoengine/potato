// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_texture.h"
#include "d3d12_platform.h"

#include "potato/runtime/assertion.h"
#include "potato/runtime/com_ptr.h"
#include "potato/spud/out_ptr.h"

up::d3d12::TextureD3D12::TextureD3D12(ID3DResourcePtr texture) : _texture(std::move(texture)) {}

up::d3d12::TextureD3D12::~TextureD3D12() = default;

bool up::d3d12::create(ID3DDevicePtr device, GpuTextureDesc const& desc, span<up::byte const> data) {

    ID3DResourcePtr texture; 
    // Describe and create a Texture2D.
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Width = desc.width;
    textureDesc.Height = desc.height;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        out_ptr(texture));

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.get(), 0, 1);

    // Create the GPU upload buffer.
    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&textureUploadHeap));

    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = &data[0];
    textureData.RowPitch = desc.width * desc.height;
    textureData.SlicePitch = data.RowPitch * desc.width;

    UpdateSubresources(m_commandList.Get(), m_texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
    m_commandList->ResourceBarrier(
        1,
        &CD3DX12_RESOURCE_BARRIER::Transition(
            m_texture.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    // Describe and create a SRV for the texture.
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    device->CreateShaderResourceView(m_texture.Get(), &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());
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
