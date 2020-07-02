// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "converter_config.h"

#include "potato/tools/asset_library.h"
#include "potato/tools/converter.h"
#include "potato/tools/converter_factory.h"
#include "potato/tools/file_hash_cache.h"
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

        void checkMetafile(ConverterContext& ctx, string_view filename);

        Converter* findConverter(string_view path) const;

        struct Mapping {
            delegate<bool(string_view) const> predicate;
            Converter* conveter = nullptr;
        };

        string_view _programName;
        vector<Mapping> _converters;
        vector<string> _outputs;
        ConverterConfig _config;
        box<FileSystem> _fileSystem;
        AssetLibrary _library;
        FileHashCache _hashes;
        Logger _logger;
        ConverterFactory _converterFactory;
    };
} // namespace up::recon
