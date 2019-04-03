// Copyright (C) 2015,2019 Sean Middleditch, all rights reserverd.

#pragma once

#if defined(UP_FOUNDATION_EXPORTS)
#    if defined(_WINDOWS)
#        define UP_FOUNDATION_API __declspec(dllexport)
#    else
#        define UP_FOUNDATION_API [[gnu::visibility("default")]]
#    endif
#else
#    define UP_FOUNDATION_API
#endif
