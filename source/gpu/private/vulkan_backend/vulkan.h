// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/assert.h"

#define VULKAN_HPP_ASSERT(expr) GM_ASSERT((expr))

#include <vulkan/vulkan.hpp>

#if _WIN32
#    include <grimm/foundation/platform_windows.h>
#    include <vulkan/vulkan_win32.h>
#endif
