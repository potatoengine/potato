// Copyright (C) 2016 Sean Middleditch, all rights reserverd.

#pragma once

#include <cstdlib>

#define GM_DEFAULT_ALLOC(bytes, align) (::std::malloc((bytes)))
#define GM_DEFAULT_FREE(ptr, bytes, align) (::std::free((ptr)))
#define GM_STRING_ALLOC(bytes, align) GM_DEFAULT_ALLOC((bytes), (align))
#define GM_STRING_FREE(ptr, bytes, align) GM_DEFAULT_FREE((ptr), (bytes), (align))
