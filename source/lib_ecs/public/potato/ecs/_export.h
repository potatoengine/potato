// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#if defined(UP_ECS_EXPORTS)
#    if defined(_WINDOWS)
#        define UP_ECS_API __declspec(dllexport)
#    else
#        define UP_ECS_API [[gnu::visibility("default")]]
#    endif
#else
#    define UP_ECS_API
#endif
