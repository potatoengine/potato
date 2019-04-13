// Copyright (C) 2015 Sean Middleditch, all rights reserverd.

#pragma once

#if defined(UP_AUDIO_EXPORTS)
#    if defined(_WINDOWS)
#        define UP_AUDIO_API __declspec(dllexport)
#    else
#        define UP_AUDIO_API [[gnu::visibility("default")]]
#    endif
#else
#    define UP_AUDIO_API
#endif
