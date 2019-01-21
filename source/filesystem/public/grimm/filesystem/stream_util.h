// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/filesystem/stream.h"
#include "grimm/filesystem/common.h"
#include "grimm/foundation/blob.h"
#include "grimm/foundation/string_blob.h"

namespace gm::fs {
    [[nodiscard]] GM_FILESYSTEM_API Result readBlob(Stream& stream, blob& out);
    [[nodiscard]] GM_FILESYSTEM_API Result readText(Stream& stream, string& out);
} // namespace gm::fs
