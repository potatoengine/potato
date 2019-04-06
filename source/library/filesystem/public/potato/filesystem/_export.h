// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#if defined(UP_FILESYSTEM_EXPORTS)
#    if defined(_WINDOWS)
#        define UP_FILESYSTEM_API __declspec(dllexport)
#    else
#        define UP_FILESYSTEM_API [[gnu::visibility("default")]]
#    endif
#else
#    define UP_FILESYSTEM_API
#endif
