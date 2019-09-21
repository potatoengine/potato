// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#define STB_IMAGE_IMPLEMENTATION
#define STB_DXT_IMPLEMENTATION

// STB has some code issues
//
#if defined(UP_COMPILER_GCC)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#    pragma GCC diagnostic ignored "-Wuninitialized"
#endif

#include <stb_image.h>
#include <stb_dxt.h>

#if defined(UP_COMPILER_GCC)
#    pragma GCC diagnostic pop
#endif
