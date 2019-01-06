// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "convert_hlsl.h"

gm::recon::HlslConverter::HlslConverter() = default;

gm::recon::HlslConverter::~HlslConverter() = default;

bool gm::recon::HlslConverter::convert(Context& ctx) {
    return CopyConverter::convert(ctx);
}
