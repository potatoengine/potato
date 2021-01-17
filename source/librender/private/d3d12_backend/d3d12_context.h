// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"
#include <d3d12.h>

namespace up {
    class GpuCommandList;
}

namespace up::d3d12 {

    // container for all common data passed around dx12 api
    class ContextD3D12 {
    public: 
        ID3DDeviceType* _device;
        GpuCommandList* _cmdList;
        D3D12MA::Allocator* _allocator; 
    };

} // namespace up::d3d12
