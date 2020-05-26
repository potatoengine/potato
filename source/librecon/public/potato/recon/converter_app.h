// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "converter.h"
#include "converter_config.h"

#include "potato/assetdb/asset_library.h"
#include "potato/assetdb/hash_cache.h"

#include "potato/runtime/filesystem.h"
#include "potato/runtime/logger.h"

#include "potato/spud/box.h"
#include "potato/spud/delegate.h"
#include "potato/spud/span.h"
#include "potato/spud/string.h"
#include "potato/spud/string_view.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up::recon {
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

        void checkMetafile(Context& ctx, string_view filename);

        Converter* findConverter(string_view path) const;

        struct Mapping {
            delegate<bool(string_view) const> predicate;
            box<Converter> conveter;
        };

        string_view _programName;
        vector<Mapping> _converters;
        vector<string> _outputs;
        ConverterConfig _config;
        box<FileSystem> _fileSystem;
        AssetLibrary _library;
        HashCache _hashes;
        Logger _logger;
    };
} // namespace up::recon
