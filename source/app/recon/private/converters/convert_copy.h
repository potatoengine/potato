// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/recon/converter.h"

namespace up::recon {
    class CopyConverter : public Converter {
    public:
        CopyConverter();
        ~CopyConverter();

        bool convert(Context& ctx) override;

        string_view name() const noexcept override { return "copy"; }
        uint64 revision() const noexcept override { return 0; }
    };
} // namespace up::recon
