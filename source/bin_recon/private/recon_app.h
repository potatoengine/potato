// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "asset_database.h"
#include "file_hash_cache.h"
#include "recon_config.h"

#include "potato/tools/importer.h"
#include "potato/tools/importer_factory.h"
#include "potato/tools/project.h"
#include "potato/runtime/logger.h"
#include "potato/spud/box.h"
#include "potato/spud/delegate.h"
#include "potato/spud/span.h"
#include "potato/spud/string.h"
#include "potato/spud/string_view.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up::recon {
    class ReconQueue;

    class ReconApp {
    public:
        ReconApp();
        ~ReconApp();

        ReconApp(ReconApp const&) = delete;
        ReconApp& operator=(ReconApp const&) = delete;

        bool run(span<char const*> args);

    private:
        struct Mapping {
            delegate<bool(string_view) const> predicate;
            Importer* conveter = nullptr;
            box<ImporterConfig> config;
        };

        void _registerImporters();

        bool _runOnce();
        bool _runServer();

        void _collectSourceFiles(ReconQueue& queue, bool forceUpdate = false);
        void _collectMissingFiles(ReconQueue& queue);

        bool _importFile(zstring_view file, bool force = false);
        bool _forgetFile(zstring_view file);

        bool _processQueue(ReconQueue& queue);

        bool _writeManifest();

        bool _isUpToDate(AssetDatabase::Imported const& record, uint64 contentHash, Importer const& importer)
            const noexcept;
        bool _isUpToDate(span<AssetDatabase::Dependency const> records);

        string _makeMetaFilename(zstring_view basePath, bool directory);
        bool _checkMetafile(ImporterContext& ctx, zstring_view metaPath, bool autoCreate = true);

        auto _findConverterMapping(string_view path) const -> Mapping const*;

        box<Project> _project;
        string_view _programName;
        string _temporaryOutputPath;
        string _manifestPath;
        vector<Mapping> _importers;
        vector<string> _outputs;
        ReconConfig _config;
        AssetDatabase _library;
        FileHashCache _hashes;
        Logger _logger;
        ImporterFactory _importerFactory;
    };
} // namespace up::recon
