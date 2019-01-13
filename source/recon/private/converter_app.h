// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/box.h"
#include "grimm/foundation/delegate.h"
#include "grimm/foundation/span.h"
#include "grimm/foundation/string_view.h"
#include "grimm/foundation/vector.h"
#include "grimm/recon/converter.h"
#include "grimm/recon/converter_config.h"
#include "grimm/library/asset_library.h"
#include <filesystem>

namespace gm::recon {
    class Converter;

    class ConverterApp {
    public:
        ConverterApp();
        ~ConverterApp();

        ConverterApp(ConverterApp const&) = delete;
        ConverterApp& operator=(ConverterApp const&) = delete;

        bool run(span<char const*> args);

    private:
        void registerConverters();

        vector<std::filesystem::path> collectSourceFiles();
        bool convertFiles(vector<std::filesystem::path> files);

        Converter* findConverter(path const& path) const;

        struct Mapping {
            delegate<bool(std::filesystem::path const&) const> predicate;
            box<Converter> conveter;
        };

        string_view _programName;
        vector<Mapping> _converters;
        ConverterConfig _config;
        AssetLibrary _library;
    };
} // namespace gm::recon
