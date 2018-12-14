// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/gpu/factory.h"
#include "grimm/foundation/box.h"

namespace gm
{
    GM_GPU_D3D12_API box<IGPUFactory> CreateD3d12Factory();
}
