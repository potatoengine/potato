// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/recon/converter.h"

namespace up::recon {
    class IgnoreConverter : public Converter {
    public:
        IgnoreConverter() = default;
        ~IgnoreConverter() = default;

        bool convert(Context& ctx) override { return true; }
        string generateSettings(Context& ctd) override { return ""; }


        string_view name() const noexcept override { return "ignore"; }
        uint64 revision() const noexcept override { return 0; }
    };
} // namespace up::recon
