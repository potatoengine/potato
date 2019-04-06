// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/filesystem/stream.h"
#include "potato/filesystem/common.h"
#include "potato/foundation/vector.h"
#include "potato/foundation/string.h"
#include "potato/foundation/int_types.h"

namespace up::fs {
    [[nodiscard]] UP_FILESYSTEM_API Result readBinary(Stream& stream, vector<up::byte>& out);
    [[nodiscard]] UP_FILESYSTEM_API Result readText(Stream& stream, string& out);

    [[nodiscard]] UP_FILESYSTEM_API Result writeAllText(Stream& stream, string_view text);
    } // namespace up::fs
