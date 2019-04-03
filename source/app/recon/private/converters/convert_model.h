// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/recon/converter.h"

namespace up::recon {
    class ModelConverter : public Converter {
    public:
        ModelConverter();
        ~ModelConverter();

        bool convert(Context& ctx) override;

        string_view name() const noexcept override { return "model"; }
        uint64 revision() const noexcept override { return 0; }
    };
} // namespace up::recon
