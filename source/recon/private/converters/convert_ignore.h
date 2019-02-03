// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/recon/converter.h"

namespace gm::recon {
    class IgnoreConverter : public Converter {
    public:
        IgnoreConverter() = default;
        ~IgnoreConverter() = default;

        bool convert(Context& ctx) override { return true; }

        string_view name() const noexcept override { return "ignore"; }
        uint64 revision() const noexcept override { return 0; }
    };
} // namespace gm::recon
