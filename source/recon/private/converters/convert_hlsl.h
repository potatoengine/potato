// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/recon/converter.h"
#include "convert_copy.h"

namespace gm::recon {
    class HlslConverter : public CopyConverter {
    public:
        HlslConverter();
        ~HlslConverter();

        bool convert(Context& ctx) override;
    };
} // namespace gm::recon
