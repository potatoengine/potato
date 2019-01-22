// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/recon/converter.h"
#include "grimm/filesystem/filesystem.h"

namespace gm::recon {
    class HlslConverter : public Converter {
    public:
        HlslConverter();
        ~HlslConverter();

        bool convert(Context& ctx) override;

        string_view name() const noexcept override { return "hlsl"; }
        uint64 revision() const noexcept override { return 8; }

    private:
        bool compile(Context& ctx, fs::FileSystem& fileSys, zstring_view absoluteSourcePath, string_view source, zstring_view entryName, zstring_view targetProfileName);
    };
} // namespace gm::recon
