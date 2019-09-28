// Copyright (C) 2015 Sean Middleditch, all rights reserverd.

#pragma once

#if defined(UP_RENDER_EXPORTS)
#    if defined(_WINDOWS)
#        define UP_RENDER_API __declspec(dllexport)
#    else
#        define UP_RENDER_API [[gnu::visibility("default")]]
#    endif
#else
#    define UP_RENDER_API
#endif

#define UP_GPU_API UP_RENDER_API
