// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/box.h"
#include "grimm/foundation/delegate.h"
#include "grimm/foundation/span.h"
#include "grimm/foundation/gmstring.h"
#include "grimm/foundation/string_view.h"
#include "grimm/foundation/zstring_view.h"
#include "grimm/foundation/vector.h"
#include "grimm/filesystem/filesystem.h"
#include "grimm/recon/converter.h"
#include "grimm/recon/converter_config.h"
#include "grimm/assetdb/asset_library.h"
#include "grimm/assetdb/hash_cache.h"

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

        vector<string> collectSourceFiles();
        bool convertFiles(vector<string> const& files);
        bool deleteUnusedFiles(vector<string> const& files, bool dryRun = true);

        bool isUpToDate(AssetImportRecord const& record, uint64 contentHash, Converter const& converter) const noexcept;
        bool isUpToDate(span<AssetDependencyRecord const> records);

        Converter* findConverter(string_view path) const;

        struct Mapping {
            delegate<bool(string_view) const> predicate;
            box<Converter> conveter;
        };

        string_view _programName;
        vector<Mapping> _converters;
        vector<string> _outputs;
        ConverterConfig _config;
        fs::FileSystem _fileSystem;
        AssetLibrary _library;
        HashCache _hashes;
    };
} // namespace gm::recon
