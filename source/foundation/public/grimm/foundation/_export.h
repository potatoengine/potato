// Copyright (C) 2015,2019 Sean Middleditch, all rights reserverd.

#pragma once

#if defined(GM_FOUNDATION_EXPORTS)
#    if defined(_WINDOWS)
#        define GM_FOUNDATION_API __declspec(dllexport)
#    else
#        define GM_FOUNDATION_API [[gnu::visibility("default")]]
#    endif
#else
#    define GM_FOUNDATION_API
#endif
