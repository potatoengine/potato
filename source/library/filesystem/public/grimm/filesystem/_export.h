// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#if defined(GM_FILESYSTEM_EXPORTS)
#    if defined(_WINDOWS)
#        define GM_FILESYSTEM_API __declspec(dllexport)
#    else
#        define GM_FILESYSTEM_API [[gnu::visibility("default")]]
#    endif
#else
#    define GM_FILESYSTEM_API
#endif
