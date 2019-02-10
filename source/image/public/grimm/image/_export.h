// Copyright (C) 2015 Sean Middleditch, all rights reserverd.

#pragma once

#if defined(GM_IMAGE_EXPORTS)
#    if defined(_WINDOWS)
#        define GM_IMAGE_API __declspec(dllexport)
#    else
#        define GM_IMAGE_API [[gnu::visibility("default")]]
#    endif
#else
#    define GM_IMAGE_API
#endif
