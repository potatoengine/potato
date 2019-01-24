// Copyright (C) 2015 Sean Middleditch, all rights reserverd.

#pragma once

#if defined(GM_GRUI_EXPORTS)
#    if defined(_WINDOWS)
#        define GM_IMGRUI_API __declspec(dllexport)
#    else
#        define GM_IMGRUI_API [[gnu::visibility("default")]]
#    endif
#else
#    define GM_IMGRUI_API
#endif
