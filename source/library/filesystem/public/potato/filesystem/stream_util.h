// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/filesystem/stream.h"
#include "potato/filesystem/common.h"
#include "potato/foundation/vector.h"
#include "potato/foundation/string.h"
#include "potato/foundation/int_types.h"

namespace up {
    [[nodiscard]] UP_FILESYSTEM_API IOResult readBinary(Stream& stream, vector<up::byte>& out);
    [[nodiscard]] UP_FILESYSTEM_API IOResult readText(Stream& stream, string& out);

    [[nodiscard]] UP_FILESYSTEM_API IOResult writeAllText(Stream& stream, string_view text);
    } // namespace up
