// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_pipeline_state.h"
#include "d3d12_command_list.h"
#include "d3d12_desc_heap.h"

#include "potato/runtime/assertion.h"
#include "potato/spud/out_ptr.h"

up::d3d12::PipelineStateD3D12::PipelineStateD3D12()  {
}

up::d3d12::PipelineStateD3D12::~PipelineStateD3D12() {
};

auto up::d3d12::PipelineStateD3D12::createGraphicsPipelineState(ID3D12Device* device, GpuPipelineStateDesc const& desc)
    -> box<PipelineStateD3D12> {
    UP_ASSERT(device != nullptr);
    auto pso = new_box<PipelineStateD3D12>();
    pso->create(device, desc); 
    return pso;
}

static D3D12_BLEND_DESC defaultBlend() {

    D3D12_BLEND_DESC blend = {FALSE, FALSE, {}};

    const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc = {
        FALSE,
        FALSE,
        D3D12_BLEND_ONE,
        D3D12_BLEND_ZERO,
        D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE,
        D3D12_BLEND_ZERO,
        D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };

    for (up::uint32 i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        blend.RenderTarget[i] = defaultRenderTargetBlendDesc;

    return blend;
}

static D3D12_RASTERIZER_DESC defaultRestirizer() {
    D3D12_RASTERIZER_DESC rs = {
        D3D12_FILL_MODE_SOLID,
        D3D12_CULL_MODE_BACK,
        FALSE,
        D3D12_DEFAULT_DEPTH_BIAS,
        D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
        D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
        TRUE,
        FALSE,
        FALSE,
        0,
        D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF};

    return rs;
}


static inline void InitRange(
    _Out_ D3D12_DESCRIPTOR_RANGE1& range,
    D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
    UINT numDescriptors,
    UINT baseShaderRegister,
    UINT registerSpace = 0,
    D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE,
    UINT offsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND) noexcept {
    range.RangeType = rangeType;
    range.NumDescriptors = numDescriptors;
    range.BaseShaderRegister = baseShaderRegister;
    range.RegisterSpace = registerSpace;
    range.Flags = flags;
    range.OffsetInDescriptorsFromTableStart = offsetInDescriptorsFromTableStart;
}

static inline void InitRootDescriptor(
    _Out_ D3D12_ROOT_DESCRIPTOR1& table,
    UINT shaderRegister,
    UINT registerSpace = 0,
    D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE) noexcept {
    table.ShaderRegister = shaderRegister;
    table.RegisterSpace = registerSpace;
    table.Flags = flags;
}

static inline void InitRootDescriptorTable(
    _Out_ D3D12_ROOT_DESCRIPTOR_TABLE1& rootDescriptorTable,
    UINT numDescriptorRanges,
    _In_reads_opt_(numDescriptorRanges) const D3D12_DESCRIPTOR_RANGE1* _pDescriptorRanges) noexcept {
    rootDescriptorTable.NumDescriptorRanges = numDescriptorRanges;
    rootDescriptorTable.pDescriptorRanges = _pDescriptorRanges;
}

static inline void InitRootConstant(
    _Out_ D3D12_ROOT_CONSTANTS& rootConstants,
    UINT num32BitValues,
    UINT shaderRegister,
    UINT registerSpace = 0) noexcept {
    rootConstants.Num32BitValues = num32BitValues;
    rootConstants.ShaderRegister = shaderRegister;
    rootConstants.RegisterSpace = registerSpace;
}

static inline void InitRootDescriptorTable1(
    _Out_ D3D12_ROOT_DESCRIPTOR_TABLE1& rootDescriptorTable,
    UINT numDescriptorRanges,
    _In_reads_opt_(numDescriptorRanges) const D3D12_DESCRIPTOR_RANGE1* _pDescriptorRanges) noexcept {
    rootDescriptorTable.NumDescriptorRanges = numDescriptorRanges;
    rootDescriptorTable.pDescriptorRanges = _pDescriptorRanges;
}
 static inline void InitAsDescriptorTable(
     _Out_ D3D12_ROOT_PARAMETER1& rootParam,
     UINT numDescriptorRanges,
     _In_reads_(numDescriptorRanges) const D3D12_DESCRIPTOR_RANGE1* pDescriptorRanges,
     D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept {
     rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
     rootParam.ShaderVisibility = visibility;
     InitRootDescriptorTable(rootParam.DescriptorTable, numDescriptorRanges, pDescriptorRanges);
 }

    static inline void InitAsConstants(
     _Out_ D3D12_ROOT_PARAMETER1& rootParam,
     UINT num32BitValues,
     UINT shaderRegister,
     UINT registerSpace = 0,
     D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept {
     rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
     rootParam.ShaderVisibility = visibility;
     InitRootConstant(rootParam.Constants, num32BitValues, shaderRegister, registerSpace);
 }

 static inline void InitAsConstantBufferView(
     _Out_ D3D12_ROOT_PARAMETER1& rootParam,
     UINT shaderRegister,
     UINT registerSpace = 0,
     D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
     D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept {
     rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
     rootParam.ShaderVisibility = visibility;
     InitRootDescriptor(rootParam.Descriptor, shaderRegister, registerSpace, flags);
 }

 static inline void InitAsShaderResourceView(
     _Out_ D3D12_ROOT_PARAMETER1& rootParam,
     UINT shaderRegister,
     UINT registerSpace = 0,
     D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
     D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept {
     rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
     rootParam.ShaderVisibility = visibility;
     InitRootDescriptor(rootParam.Descriptor, shaderRegister, registerSpace, flags);
 }

 static inline void InitAsUnorderedAccessView(
     _Out_ D3D12_ROOT_PARAMETER1& rootParam,
     UINT shaderRegister,
     UINT registerSpace = 0,
     D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
     D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept {
     rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
     rootParam.ShaderVisibility = visibility;
     InitRootDescriptor(rootParam.Descriptor, shaderRegister, registerSpace, flags);
 }


static inline void Init_1_1(
    _Out_ D3D12_VERSIONED_ROOT_SIGNATURE_DESC& desc,
    UINT numParameters,
    _In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER1* _pParameters,
    UINT numStaticSamplers = 0,
    _In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = nullptr,
    D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE) noexcept {
    desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    desc.Desc_1_1.NumParameters = numParameters;
    desc.Desc_1_1.pParameters = _pParameters;
    desc.Desc_1_1.NumStaticSamplers = numStaticSamplers;
    desc.Desc_1_1.pStaticSamplers = _pStaticSamplers;
    desc.Desc_1_1.Flags = flags;
}

static inline void InitRootSignatureDesc(
    _Out_ D3D12_ROOT_SIGNATURE_DESC& desc,
    UINT numParameters,
    _In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER* _pParameters,
    UINT numStaticSamplers = 0,
    _In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = nullptr,
    D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE) noexcept {
    desc.NumParameters = numParameters;
    desc.pParameters = _pParameters;
    desc.NumStaticSamplers = numStaticSamplers;
    desc.pStaticSamplers = _pStaticSamplers;
    desc.Flags = flags;
}

//------------------------------------------------------------------------------------------------
// D3D12 exports a new method for serializing root signatures in the Windows 10 Anniversary Update.
// To help enable root signature 1.1 features when they are available and not require maintaining
// two code paths for building root signatures, this helper method reconstructs a 1.0 signature when
// 1.1 is not supported.
inline HRESULT D3DX12SerializeVersionedRootSignature(
    _In_ const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignatureDesc,
    D3D_ROOT_SIGNATURE_VERSION MaxVersion,
    _Outptr_ ID3DBlob** ppBlob,
    _Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob) noexcept {
    if (ppErrorBlob != nullptr) {
        *ppErrorBlob = nullptr;
    }

    switch (MaxVersion) {
        case D3D_ROOT_SIGNATURE_VERSION_1_0:
            switch (pRootSignatureDesc->Version) {
                case D3D_ROOT_SIGNATURE_VERSION_1_0:
                    return D3D12SerializeRootSignature(
                        &pRootSignatureDesc->Desc_1_0,
                        D3D_ROOT_SIGNATURE_VERSION_1,
                        ppBlob,
                        ppErrorBlob);

                case D3D_ROOT_SIGNATURE_VERSION_1_1: {
                    HRESULT hr = S_OK;
                    const D3D12_ROOT_SIGNATURE_DESC1& desc_1_1 = pRootSignatureDesc->Desc_1_1;

                    const SIZE_T ParametersSize = sizeof(D3D12_ROOT_PARAMETER) * desc_1_1.NumParameters;
                    void* pParameters = (ParametersSize > 0) ? HeapAlloc(GetProcessHeap(), 0, ParametersSize) : nullptr;
                    if (ParametersSize > 0 && pParameters == nullptr) {
                        hr = E_OUTOFMEMORY;
                    }
                    auto pParameters_1_0 = static_cast<D3D12_ROOT_PARAMETER*>(pParameters);

                    if (SUCCEEDED(hr)) {
                        for (UINT n = 0; n < desc_1_1.NumParameters; n++) {
                            __analysis_assume(ParametersSize == sizeof(D3D12_ROOT_PARAMETER) * desc_1_1.NumParameters);
                            pParameters_1_0[n].ParameterType = desc_1_1.pParameters[n].ParameterType;
                            pParameters_1_0[n].ShaderVisibility = desc_1_1.pParameters[n].ShaderVisibility;

                            switch (desc_1_1.pParameters[n].ParameterType) {
                                case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
                                    pParameters_1_0[n].Constants.Num32BitValues =
                                        desc_1_1.pParameters[n].Constants.Num32BitValues;
                                    pParameters_1_0[n].Constants.RegisterSpace =
                                        desc_1_1.pParameters[n].Constants.RegisterSpace;
                                    pParameters_1_0[n].Constants.ShaderRegister =
                                        desc_1_1.pParameters[n].Constants.ShaderRegister;
                                    break;

                                case D3D12_ROOT_PARAMETER_TYPE_CBV:
                                case D3D12_ROOT_PARAMETER_TYPE_SRV:
                                case D3D12_ROOT_PARAMETER_TYPE_UAV:
                                    pParameters_1_0[n].Descriptor.RegisterSpace =
                                        desc_1_1.pParameters[n].Descriptor.RegisterSpace;
                                    pParameters_1_0[n].Descriptor.ShaderRegister =
                                        desc_1_1.pParameters[n].Descriptor.ShaderRegister;
                                    break;

                                case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
                                    const D3D12_ROOT_DESCRIPTOR_TABLE1& table_1_1 =
                                        desc_1_1.pParameters[n].DescriptorTable;

                                    const SIZE_T DescriptorRangesSize =
                                        sizeof(D3D12_DESCRIPTOR_RANGE) * table_1_1.NumDescriptorRanges;
                                    void* pDescriptorRanges = (DescriptorRangesSize > 0 && SUCCEEDED(hr))
                                        ? HeapAlloc(GetProcessHeap(), 0, DescriptorRangesSize)
                                        : nullptr;
                                    if (DescriptorRangesSize > 0 && pDescriptorRanges == nullptr) {
                                        hr = E_OUTOFMEMORY;
                                    }
                                    auto pDescriptorRanges_1_0 =
                                        static_cast<D3D12_DESCRIPTOR_RANGE*>(pDescriptorRanges);

                                    if (SUCCEEDED(hr)) {
                                        for (UINT x = 0; x < table_1_1.NumDescriptorRanges; x++) {
                                            __analysis_assume(
                                                DescriptorRangesSize ==
                                                sizeof(D3D12_DESCRIPTOR_RANGE) * table_1_1.NumDescriptorRanges);
                                            pDescriptorRanges_1_0[x].BaseShaderRegister =
                                                table_1_1.pDescriptorRanges[x].BaseShaderRegister;
                                            pDescriptorRanges_1_0[x].NumDescriptors =
                                                table_1_1.pDescriptorRanges[x].NumDescriptors;
                                            pDescriptorRanges_1_0[x].OffsetInDescriptorsFromTableStart =
                                                table_1_1.pDescriptorRanges[x].OffsetInDescriptorsFromTableStart;
                                            pDescriptorRanges_1_0[x].RangeType =
                                                table_1_1.pDescriptorRanges[x].RangeType;
                                            pDescriptorRanges_1_0[x].RegisterSpace =
                                                table_1_1.pDescriptorRanges[x].RegisterSpace;
                                        }
                                    }

                                    D3D12_ROOT_DESCRIPTOR_TABLE& table_1_0 = pParameters_1_0[n].DescriptorTable;
                                    table_1_0.NumDescriptorRanges = table_1_1.NumDescriptorRanges;
                                    table_1_0.pDescriptorRanges = pDescriptorRanges_1_0;
                            }
                        }
                    }

                    if (SUCCEEDED(hr)) {
                        D3D12_ROOT_SIGNATURE_DESC desc_1_0;
                        InitRootSignatureDesc(
                            desc_1_0,
                            desc_1_1.NumParameters,
                            pParameters_1_0,
                            desc_1_1.NumStaticSamplers,
                            desc_1_1.pStaticSamplers,
                            desc_1_1.Flags);
                        hr = D3D12SerializeRootSignature(&desc_1_0, D3D_ROOT_SIGNATURE_VERSION_1, ppBlob, ppErrorBlob);
                    }

                    if (pParameters) {
                        for (UINT n = 0; n < desc_1_1.NumParameters; n++) {
                            if (desc_1_1.pParameters[n].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
                                HeapFree(
                                    GetProcessHeap(),
                                    0,
                                    reinterpret_cast<void*>(const_cast<D3D12_DESCRIPTOR_RANGE*>(
                                        pParameters_1_0[n].DescriptorTable.pDescriptorRanges)));
                            }
                        }
                        HeapFree(GetProcessHeap(), 0, pParameters);
                    }
                    return hr;
                }
            }
            break;

        case D3D_ROOT_SIGNATURE_VERSION_1_1:
            return D3D12SerializeVersionedRootSignature(pRootSignatureDesc, ppBlob, ppErrorBlob);
    }

    return E_INVALIDARG;
}

auto up::d3d12::PipelineStateD3D12::createRootSignature(ID3D12Device* device) -> bool {
    ID3DRootSignaturePtr root;
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

    // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned
    // will not be greater than this.
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))) {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }
    
    up::com_ptr<ID3DBlob> signature;
    up::com_ptr<ID3DBlob> error;

    D3D12_DESCRIPTOR_RANGE1 ranges[3];
    D3D12_ROOT_PARAMETER1 parameters[RootParamIndex::RootParamCount];

    InitRange(ranges[0], D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0);
    InitRange(ranges[1], D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
    InitRange(ranges[2], D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0);

    InitAsConstantBufferView(parameters[RootParamIndex::ConstantBuffer], 0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE);
    //InitAsDescriptorTable(parameters[RootParamIndex::ConstantBuffer], 1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);
    InitAsDescriptorTable(parameters[RootParamIndex::TextureSRV], 1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
    InitAsDescriptorTable(parameters[RootParamIndex::TextureSampler], 1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);
 
    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    Init_1_1(rootSignatureDesc, _countof(parameters), parameters, 0, nullptr, rootSignatureFlags);

   if (FAILED(D3DX12SerializeVersionedRootSignature(
            &rootSignatureDesc,
            featureData.HighestVersion,
            out_ptr(signature),
            out_ptr(error)))) {

       auto msg = static_cast<const char*>(error->GetBufferPointer());
       UP_ASSERT(0, msg);
       return false; 
   };
    device->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        __uuidof(ID3D12RootSignature),
        out_ptr(_signature));

    UP_ASSERT(signature.get());
    return true;
}
bool up::d3d12::PipelineStateD3D12::create(ID3D12Device* device, GpuPipelineStateDesc const& desc) {

    std::vector<D3D12_INPUT_ELEMENT_DESC> elements;
    uint32 offset = 0;
    for (auto i : desc.inputLayout) {
        D3D12_INPUT_ELEMENT_DESC desc = {
            toNative(i.semantic).c_str(),
            0,
            toNative(i.format),
            0,
            offset,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0};

        offset += toByteSize(i.semantic);
        elements.push_back(desc);
    }

    auto blend = defaultBlend();
    auto rs = defaultRestirizer(); 

    createRootSignature(device);
    
    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = {elements.data(), static_cast<UINT>(elements.size())};
    psoDesc.pRootSignature = _signature.get();
    psoDesc.VS = {desc.vertShader.data(), desc.vertShader.size()};
    psoDesc.PS = {desc.pixelShader.data(), desc.pixelShader.size()};
    psoDesc.RasterizerState = rs;
    psoDesc.BlendState = blend;
    psoDesc.DepthStencilState.DepthEnable = desc.enableDepthTest;
    psoDesc.DepthStencilState.StencilEnable = desc.enableDepthWrite;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 2;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    device->CreateGraphicsPipelineState(&psoDesc, __uuidof(ID3D12PipelineState), out_ptr(_state));

    _srvHeap = new_box<DescriptorHeapD3D12>(
        device,
        1,
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

    return true;
}

void up::d3d12::PipelineStateD3D12::bindPipeline(ID3D12GraphicsCommandList* cmd) {
    UP_ASSERT(cmd!=nullptr);

    cmd->SetGraphicsRootSignature(_signature.get());
    cmd->SetPipelineState(_state.get());
    
    ID3D12DescriptorHeap* ppHeaps[] = {_srvHeap->heap(), _samplerHeap};
    cmd->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
}

void up::d3d12::PipelineStateD3D12::bindTexture(
    ID3D12GraphicsCommandList* cmd,
    D3D12_GPU_DESCRIPTOR_HANDLE srv,
    D3D12_GPU_DESCRIPTOR_HANDLE sampler) {
    UP_ASSERT(cmd != nullptr);

    cmd->SetGraphicsRootDescriptorTable(RootParamIndex::TextureSRV, srv);
    cmd->SetGraphicsRootDescriptorTable(RootParamIndex::TextureSampler, sampler);
}

void up::d3d12::PipelineStateD3D12::bindConstBuffer(ID3D12GraphicsCommandList* cmd, D3D12_GPU_VIRTUAL_ADDRESS cbv) {
    UP_ASSERT(cmd != nullptr);
    cmd->SetGraphicsRootConstantBufferView(RootParamIndex::ConstantBuffer, cbv);
}
