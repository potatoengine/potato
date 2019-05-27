// Copyright (C) 2015,2019 Sean Middleditch, all rights reserverd.

#pragma once

#if defined(UP_CORE_EXPORTS)
#    if defined(_WINDOWS)
#        define UP_CORE_API __declspec(dllexport)
#    else
#        define UP_CORE_API [[gnu::visibility("default")]]
#    endif
#else
#    define UP_CORE_API
#endif
