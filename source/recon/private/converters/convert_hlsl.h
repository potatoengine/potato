// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/recon/converter.h"

namespace gm::recon {
    class HlslConverter : public Converter {
    public:
        HlslConverter();
        ~HlslConverter();

        bool convert(Context& ctx) override;

        string_view name() const noexcept override { return "hlsl"; }
        uint64 revision() const noexcept override { return 6; }
    };
} // namespace gm::recon
