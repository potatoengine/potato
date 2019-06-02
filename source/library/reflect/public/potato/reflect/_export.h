#pragma once

#if defined(UP_REFLECT_EXPORTS)
#    if defined(_WINDOWS)
#        define UP_REFLECT_API __declspec(dllexport)
#    else
#        define UP_REFLECT_API [[gnu::visibility("default")]]
#    endif
#else
#    define UP_REFLECT_API
#endif
