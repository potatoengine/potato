// Copyright (C) 2015 Sean Middleditch, all rights reserverd.

#pragma once

#if defined(GM_LIBRARY_EXPORTS)
#    if defined(_WINDOWS)
#        define GM_LIBRARY_API __declspec(dllexport)
#    else
#        define GM_LIBRARY_API [[gnu::visibility("default")]]
#    endif
#else
#    define GM_LIBRARY_API
#endif
