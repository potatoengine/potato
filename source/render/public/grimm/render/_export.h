// Copyright (C) 2015 Sean Middleditch, all rights reserverd.

#pragma once

#if defined(GM_RENDER_EXPORTS)
#    if defined(_WINDOWS)
#        define GM_RENDER_API __declspec(dllexport)
#    else
#        define GM_RENDER_API [[gnu::visibility("default")]]
#    endif
#else
#    define GM_RENDER_API
#endif
