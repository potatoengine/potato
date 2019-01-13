// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/recon/converter.h"

namespace gm::recon {
    class JsonConverter : public Converter {
    public:
        JsonConverter();
        ~JsonConverter();

        bool convert(Context& ctx) override;
    };
} // namespace gm::recon
