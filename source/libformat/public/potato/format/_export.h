// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#if defined(UP_FORMAT_EXPORTS)
#    if defined(_WINDOWS)
#        define UP_FORMAT_API __declspec(dllexport)
#    else
#        define UP_FORMAT_API [[gnu::visibility("default")]]
#    endif
#else
#    define UP_FORMAT_API
#endif
