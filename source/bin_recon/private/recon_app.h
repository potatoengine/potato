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

    enum class ReconImportResult { NotFound, UnknownType, UpToDate, Failed, Imported };

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
            Importer* importer = nullptr;
            box<ImporterConfig> config;
        };

        void _registerImporters();

        bool _runOnce();
        bool _runServer();

        void _collectSourceFiles(ReconQueue& queue, bool forceUpdate = false);
        void _collectMissingFiles(ReconQueue& queue);

        ReconImportResult _importFile(zstring_view file, bool force = false);
        bool _forgetFile(zstring_view file);

        bool _processQueue(ReconQueue& queue);

        bool _writeManifest();

        bool _isUpToDate(zstring_view assetPath, uint64 contentHash);

        string _makeMetaFilename(zstring_view basePath, bool directory);

        auto _findConverterMapping(string_view path, bool isFolder) const -> Mapping const*;

        box<Project> _project;
        string_view _programName;
        string _temporaryOutputPath;
        string _manifestPath;
        vector<Mapping> _importers;
        Mapping _folderImporter;
        vector<string> _outputs;
        ReconConfig _config;
        AssetDatabase _library;
        FileHashCache _hashes;
        Logger _logger;
        ImporterFactory _importerFactory;
        bool _manifestDirty = false;
    };
} // namespace up::recon
