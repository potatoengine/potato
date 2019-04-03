// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/filesystem/stream.h"
#include "grimm/filesystem/common.h"
#include "grimm/foundation/vector.h"
#include "grimm/foundation/string.h"
#include "grimm/foundation/int_types.h"

namespace up::fs {
    [[nodiscard]] UP_FILESYSTEM_API Result readBinary(Stream& stream, vector<up::byte>& out);
    [[nodiscard]] UP_FILESYSTEM_API Result readText(Stream& stream, string& out);
} // namespace up::fs
