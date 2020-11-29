// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"
#include "gpu_resource_view.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/box.h"

namespace up::d3d12 {
    class ResourceViewD3D12 final : public GpuResourceView {
    public:
        explicit ResourceViewD3D12(GpuViewType type);
        virtual ~ResourceViewD3D12();

        ResourceViewD3D12(ResourceViewD3D12&&) = delete;
        ResourceViewD3D12& operator=(ResourceViewD3D12&&) = delete;

        GpuViewType type() const override { return _type; }
     

    private:
        GpuViewType _type;
    };
} // namespace up::d3d12
