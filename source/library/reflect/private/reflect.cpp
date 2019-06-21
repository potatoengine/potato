// Copyright (C) 2019 Potato Engine authors and contributors, all rights reserverd.

#include "potato/reflect/reflect.h"

// temporarily here because SHARED libraries on Windows must have
// at least one symbol; remove this when we add a real one or when
// we decide to just make this an INTERFACE library
static auto something(int x) noexcept -> int {
    return x;
}
