// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/recon/converter.h"
#include "potato/filesystem/filesystem.h"

namespace up::recon {
    class HlslConverter : public Converter {
    public:
        HlslConverter();
        ~HlslConverter();

        bool convert(Context& ctx) override;
        string generateSettings(Context& ctd) { return ""; }

        string_view name() const noexcept override { return "hlsl"; }
        uint64 revision() const noexcept override { return 8; }

    private:
        bool compile(Context& ctx, FileSystem& fileSys, zstring_view absoluteSourcePath, string_view source, zstring_view entryName, zstring_view targetProfileName);
    };
} // namespace up::recon
