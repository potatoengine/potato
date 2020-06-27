// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "converter.h"

#include "potato/runtime/filesystem.h"

namespace up {
    class HlslConverter : public Converter {
    public:
        HlslConverter();
        ~HlslConverter() override;

        bool convert(ConverterContext& ctx) override;
        string_view generateSettings(ConverterContext& ctd) override { return {}; }

        string_view name() const noexcept override { return "hlsl"; }
        uint64 revision() const noexcept override { return 8; }

    private:
        bool _compile(ConverterContext& ctx,
            FileSystem& fileSys,
            zstring_view absoluteSourcePath,
            string_view source,
            zstring_view entryName,
            zstring_view targetProfileName);
    };
} // namespace up
