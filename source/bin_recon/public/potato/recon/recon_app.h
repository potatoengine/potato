// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "recon_config.h"

#include "potato/tools/asset_library.h"
#include "potato/tools/file_hash_cache.h"
#include "potato/tools/importer.h"
#include "potato/tools/importer_factory.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/logger.h"
#include "potato/spud/box.h"
#include "potato/spud/delegate.h"
#include "potato/spud/span.h"
#include "potato/spud/string.h"
#include "potato/spud/string_view.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up {
    class ResourceManifest;
}

namespace up::recon {
    class ReconApp {
    public:
        ReconApp();
        ~ReconApp();

        ReconApp(ReconApp const&) = delete;
        ReconApp& operator=(ReconApp const&) = delete;

        bool run(span<char const*> args);

    private:
        void _registerImporters();

        auto _collectSourceFiles() -> vector<string>;
        bool _importFiles(view<string> files);
        bool _deleteUnusedFiles(view<string> files, bool dryRun = true);

        bool _isUpToDate(AssetImportRecord const& record, uint64 contentHash, Importer const& importer) const noexcept;
        bool _isUpToDate(span<AssetDependencyRecord const> records);

        bool _checkMetafile(ImporterContext& ctx, string_view filename);

        auto _findConverter(string_view path) const -> Importer*;

        auto _generateManifest() -> ResourceManifest;

        struct Mapping {
            delegate<bool(string_view) const> predicate;
            Importer* conveter = nullptr;
        };

        string_view _programName;
        string _libraryPath;
        vector<Mapping> _importers;
        vector<string> _outputs;
        ReconConfig _config;
        box<FileSystem> _fileSystem;
        AssetLibrary _library;
        FileHashCache _hashes;
        Logger _logger;
        ImporterFactory _importerFactory;
    };
} // namespace up::recon
