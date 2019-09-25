// Copyright (C) 2015 Sean Middleditch, all rights reserverd.

#pragma once

#if defined(UP_ASSETDB_EXPORTS)
#    if defined(_WINDOWS)
#        define UP_ASSETDB_API __declspec(dllexport)
#    else
#        define UP_ASSETDB_API [[gnu::visibility("default")]]
#    endif
#else
#    define UP_ASSETDB_API
#endif
