// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#if defined(UP_FOUNDATION_ASSERT_HEADER)
#   include UP_FOUNDATION_ASSERT_HEADER
#endif

#if !defined(UP_FOUNDATION_ASSERT)
#    include <cassert>
#    define UP_FOUNDATION_ASSERT(condition, message) assert((condition) && (message))
#endif
