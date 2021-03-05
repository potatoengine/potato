// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_app.h"
#include "file_hash_cache.h"
#include "meta_file.h"
#include "recon_log_sink.h"
#include "recon_messages_schema.h"
#include "recon_queue.h"
#include "recon_server.h"

#include "potato/format/format.h"
#include "potato/recon/recon_protocol.h"
#include "potato/runtime/concurrent_queue.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/io_loop.h"
#include "potato/runtime/json.h"
#include "potato/runtime/path.h"
#include "potato/runtime/stream.h"
#include "potato/runtime/uuid.h"
#include "potato/spud/overload.h"
#include "potato/spud/std_iostream.h"
#include "potato/spud/string_view.h"
#include "potato/spud/string_writer.h"

#include <nlohmann/json.hpp>

up::recon::ReconApp::ReconApp() : _programName("recon"), _logger("recon"), _server(_logger) {}

up::recon::ReconApp::~ReconApp() = default;

bool up::recon::ReconApp::run(span<char const*> args) {
    zstring_view const configFile = "recon.config.json";
    if (fs::fileExists(configFile)) {
        parseConfigFile(_config, configFile, _logger);
    }

    if (!parseArguments(_config, args, _logger)) {
        _logger.error("Failed to parse arguments");
        return false;
    }

    if (_config.server) {
        _server.start(_loop);
        Logger::root().attach(new_shared<ReconProtocolLogSink>(_server));
    }

    _project = Project::loadFromFile(_config.project);
    if (_project == nullptr) {
        _logger.error("Failed to load project file `{}`", _config.project);
        return false;
    }

    _manifestPath = path::join(path::Separator::Native, _project->libraryPath(), "manifest.txt");

    if (auto const rs = fs::createDirectories(_project->libraryPath()); rs != IOResult::Success) {
        _logger.error("Failed to create library folder `{}`: {}", _project->libraryPath(), rs);
        return false;
    }

    _temporaryOutputPath = path::join(path::Separator::Native, _project->libraryPath(), "temp");

    _registerImporters();

    auto libraryPath = path::join(path::Separator::Native, _project->libraryPath(), "assets.db");
    if (!_library.open(libraryPath)) {
        _logger.error("Failed to open asset library `{}'", libraryPath);
    }
    _logger.info("Opened asset library `{}'", libraryPath);

    auto hashCachePath = path::join(path::Separator::Native, _project->libraryPath(), "hash_cache.db");
    if (!_hashes.open(hashCachePath)) {
        _logger.error("Failed to open hash cache `{}'", hashCachePath);
    }
    _logger.info("Opened hash cache `{}'", hashCachePath);

    bool success = _config.server ? _runServer() : _runOnce();

    if (!_hashes.close()) {
        _logger.error("Failed to close hash cache `{}'", hashCachePath);
        return false;
    }

    if (!_library.close()) {
        _logger.error("Failed to close library `{}'", libraryPath);
        return false;
    }

    return success;
}

bool up::recon::ReconApp::_runOnce() {
    ReconQueue queue;
    _collectSourceFiles(queue);
    _collectMissingFiles(queue);
    _processQueue(queue);

    if (!_writeManifest()) {
        _logger.error("Failed to write manifest");
        return false;
    }

    return true;
}

bool up::recon::ReconApp::_runServer() {
    ReconQueue queue;

    _collectSourceFiles(queue);
    _collectMissingFiles(queue);

    IOWatch watch =
        _loop.createWatch(_project->resourceRootPath(), [&queue](zstring_view filename, IOWatchEvent event) {
            // ignore dot files (except .meta files)
            if (filename != ".meta"_sv && (filename.empty() || filename.front() == '.')) {
                return;
            }

            switch (event) {
                case IOWatchEvent::Rename:
                    if (fs::fileStat(filename)) {
                        queue.enqueImport(string{filename});
                    }
                    else {
                        queue.enqueForget(string{filename});
                    }
                    break;

                case IOWatchEvent::Change:
                    queue.enqueImport(string{filename});
                    break;
            }
        });

    IOEvent event = _loop.createEvent();

    _server.onDisconnect([this, &queue, &event] {
        queue.enqueTerminate();
        event.signal();
        _loop.stop();
    });
    _server.on<ReconImportMessage>([&queue, &event](schema::ReconImportMessage const& msg) {
        queue.enqueImport(msg.uuid, msg.force);
        event.signal();
    });
    _server.on<ReconImportAllMessage>([&queue, &event](schema::ReconImportAllMessage const& msg) {
        queue.enqueImportAll();
        event.signal();
    });

    IOPrepareHook prepHook = _loop.createPrepareHook([this, &queue] {
        _processQueue(queue);
        if (_manifestDirty) {
            _manifestDirty = false;
            _writeManifest();
            _server.send<ReconManifestMessage>({.path = _manifestPath});
        }
    });

    // flush the initially-updated manifest
    _server.send<ReconManifestMessage>({.path = _manifestPath});

    _loop.run(IORun::Default);

    _server.stop();

    // server only "fails" if it can't run at all
    return true;
}

bool up::recon::ReconApp::_processQueue(ReconQueue& queue) {
    bool terminate = false;

    ReconQueue::Command cmd;
    while (queue.tryDeque(cmd)) {
        switch (cmd.type) {
            case ReconQueue::Type::Import:
            case ReconQueue::Type::ForceImport:
                if (path::extension(cmd.filename) == ".meta"_zsv) {
                    cmd.filename = path::changeExtension(cmd.filename, "");
                }

                _importFile(cmd.filename, cmd.type == ReconQueue::Type::ForceImport);
                break;
            case ReconQueue::Type::Update:
                if (path::extension(cmd.filename) == ".meta"_zsv) {
                    cmd.filename = path::changeExtension(cmd.filename, "");
                }

                // re-import the file... and forget the file if it's not there
                if (_importFile(cmd.filename) == ReconImportResult::NotFound) {
                    _forgetFile(cmd.filename);
                }
                break;
            case ReconQueue::Type::Forget:
                // if a .meta file is deleted, reimport the source
                if (path::extension(cmd.filename) == ".meta"_zsv) {
                    cmd.filename = path::changeExtension(cmd.filename, "");
                    _importFile(cmd.filename);
                }
                else {
                    _forgetFile(cmd.filename);

                    // if a directory is deleted, we don't get notifications for files under it,
                    // so explicitly delete children
                    vector<string> children;
                    for (auto path : _library.collectAssetPathsByFolder(cmd.filename)) {
                        children.push_back(string{path});
                    }
                    for (string const& path : children) {
                        _forgetFile(path);
                    }
                }
                break;
            case ReconQueue::Type::ImportAll:
            case ReconQueue::Type::ForceImportAll:
                _collectSourceFiles(queue, cmd.type == ReconQueue::Type::ForceImportAll);
                _collectMissingFiles(queue);
                break;
            case ReconQueue::Type::Delete:
                if (zstring_view const sourcePath = _library.uuidToPath(cmd.uuid); !sourcePath.empty()) {
                    _logger.info("Delete: {}", sourcePath);

                    (void)fs::remove(path::join(path::Separator::Native, _project->resourceRootPath(), sourcePath));
                    _forgetFile(sourcePath);
                }
                break;
            case ReconQueue::Type::Terminate:
                terminate = true;
                break;
        }
    }

    return !terminate;
}

void up::recon::ReconApp::_registerImporters() {
    _importerFactory.registerDefaultImporters();

    _folderImporter.importer = _importerFactory.findImporterByName("folder"_sv);
    _folderImporter.config = new_box<ImporterConfig>();
    UP_ASSERT(_folderImporter.importer != nullptr);

    for (auto const& mapping : _config.mapping) {
        auto const importer = _importerFactory.findImporterByName(mapping.importer);
        if (importer == nullptr) {
            _logger.error("Unknown importer `{}'", mapping.importer);
        }

        auto config = _importerFactory.parseConfig(*importer, mapping.config);

        string_view pattern = mapping.pattern;
        _importers.push_back(
            {[pattern](string_view filename) { return path::extension(filename) == pattern; },
             importer,
             std::move(config)});
    }
}

namespace up::recon {
    static bool loadMetaFile(MetaFile& metaFile, zstring_view osPath) {
        if (Stream stream = fs::openRead(osPath, fs::OpenMode::Text); stream) {
            auto [result, jsonText] = readText(stream);
            if (result == IOResult::Success) {
                if (metaFile.parseJson(jsonText)) {
                    return true;
                }
            }
        }
        return false;
    }
} // namespace up::recon

auto up::recon::ReconApp::_importFile(zstring_view file, bool force) -> ReconImportResult {
    auto osPath = path::join(path::Separator::Native, _project->resourceRootPath(), file.c_str());

    auto const [statRs, stat] = fs::fileStat(osPath);
    if (statRs != IOResult::Success) {
        return ReconImportResult::NotFound;
    }
    bool const isFolder = stat.type == fs::FileType::Directory;

    Mapping const* const mapping = _findConverterMapping(file, isFolder);
    if (mapping == nullptr) {
        _logger.error("{}: unknown file type", file);
        return ReconImportResult::UnknownType;
    }

    auto metaPath = _makeMetaFilename(file, isFolder);
    string metaOsPath = path::join(path::Separator::Native, _project->resourceRootPath(), metaPath);

    MetaFile metaFile;
    bool metaDirty = false;

    if (!loadMetaFile(metaFile, metaOsPath)) {
        metaFile.generate();
        metaDirty = true;
    }

    Importer* const importer = mapping->importer;

    auto const contentHash = isFolder ? 0 : _hashes.hashAssetAtPath(osPath.c_str());
    bool dirty = !_library.checkAssetUpToDate(metaFile.uuid, importer->name(), importer->revision(), contentHash);

    _library.createAsset(metaFile.uuid, file, contentHash);

    if (importer->name() != string_view{metaFile.importerName}) {
        metaFile.importerName = string{importer->name()};
        metaDirty = true;
        dirty = true;
    }

    string_view settings = importer->defaultSettings();
    if (settings != metaFile.importerSettings) {
        metaFile.importerSettings = string{settings};
        metaDirty = true;
        dirty = true;
    }

    for (auto const& dep : _library.assetDependencies(metaFile.uuid)) {
        if (dirty) {
            break;
        }
        dirty = !_isUpToDate(dep.path, dep.contentHash);
    }

    string_writer importedName;
    format_append(importedName, "{{{}} {} ({})", metaFile.uuid, file, importer->name());

    if (!dirty && !force) {
        _logger.info("{}: up-to-date", importedName);
        _library.updateAssetPost(metaFile.uuid, true);
        return ReconImportResult::UpToDate;
    }

    _manifestDirty = true;

    vector<string> dependencies;
    vector<ImporterContext::Output> outputs;
    ImporterContext context(
        metaFile.uuid,
        file,
        _project->resourceRootPath(),
        _temporaryOutputPath,
        importer,
        *mapping->config,
        dependencies,
        outputs,
        _logger);

    _library.updateAssetPre(
        metaFile.uuid,
        importer != nullptr ? importer->name() : string_view{},
        importer->assetType(context),
        importer != nullptr ? importer->revision() : 0);

    if (metaDirty) {
        _logger.info("Writing meta file `{}'", metaOsPath);

        string jsonText = metaFile.toJson();

        auto stream = fs::openWrite(metaOsPath, fs::OpenMode::Text);
        if (!stream || writeAllText(stream, jsonText) != IOResult::Success) {
            _logger.error("Failed to write meta file for {}", metaOsPath);
            _library.updateAssetPost(metaFile.uuid, false);
            return ReconImportResult::Failed;
        }
    }

    if (importer == nullptr) {
        _logger.error("{}: unknown file type", importedName);
        _library.updateAssetPost(metaFile.uuid, false);
        return ReconImportResult::UnknownType;
    }

    _logger.info("{}: importing", importedName);
    if (!importer->import(context)) {
        _logger.error("{}: import failed", importedName);
        _library.updateAssetPost(metaFile.uuid, false);
        return ReconImportResult::Failed;
    }

    // move outputs to CAS
    //
    for (auto const& output : outputs) {
        auto outputOsPath = path::join(path::Separator::Native, _temporaryOutputPath, output.path);
        auto const outputHash = _hashes.hashAssetAtPath(outputOsPath);

        fixed_string_writer<32> casPath;
        format_append(
            casPath,
            "{:02X}/{:04X}/{:016X}.bin",
            (outputHash >> 56) & 0xFF,
            (outputHash >> 40) & 0XFFFF,
            outputHash);

        auto casOsPath = path::join(path::Separator::Native, _project->libraryPath(), "cache", casPath);
        auto casOsFolder = string{path::parent(casOsPath)};

        if (auto const rs = fs::createDirectories(casOsFolder); rs != IOResult::Success) {
            _logger.error("Failed to create directory `{}`", casOsFolder);
            continue;
        }

        if (auto const rs = fs::moveFileTo(outputOsPath, casOsPath); rs != IOResult::Success) {
            _logger.error("Failed to move temp file `{}` to CAS `{}`", outputOsPath, casOsPath);
            continue;
        }
    }

    // Update AssetDatabase with result of import
    _library.transact([&](auto& library, auto&) {
        _library.updateAssetPost(metaFile.uuid, true);

        for (auto const& sourceDepPath : dependencies) {
            auto osPath = path::join(path::Separator::Native, _project->resourceRootPath(), sourceDepPath.c_str());
            auto const contentHash = _hashes.hashAssetAtPath(osPath.c_str());
            library.addDependency(context.uuid(), sourceDepPath, contentHash);
        }

        for (auto const& output : outputs) {
            auto outputOsPath = path::join(path::Separator::Native, _temporaryOutputPath, output.path);
            auto const outputHash = _hashes.hashAssetAtPath(outputOsPath);

            _library.addOutput(context.uuid(), output.logicalAsset, output.type, outputHash);
        }
    });

    return ReconImportResult::Imported;
}

bool up::recon::ReconApp::_forgetFile(zstring_view file) {
    UUID const uuid = _library.pathToUuid(file);
    if (!uuid.isValid()) {
        return true; // let's not consider this an error; we were asked to forget something we didn't know about
    }

    _manifestDirty = true;

    _logger.info("{}: deleted", file);
    _library.deleteAsset(uuid);

    // remove the .meta file if it exists;
    // we don't need to do this for a directory, since they could only be deleted
    // if the .meta file is deleted too
    auto metaPath = _makeMetaFilename(file, false);
    auto metaOsPath = path::join(path::Separator::Native, _project->resourceRootPath(), metaPath);
    (void)fs::remove(metaOsPath);

    return true;
}

bool up::recon::ReconApp::_isUpToDate(zstring_view assetPath, uint64 contentHash) {
    auto osPath = path::join(path::Separator::Native, _project->resourceRootPath(), assetPath);
    return contentHash == _hashes.hashAssetAtPath(osPath.c_str());
}

auto up::recon::ReconApp::_findConverterMapping(string_view path, bool isFolder) const -> Mapping const* {
    if (isFolder) {
        return &_folderImporter;
    }

    for (auto const& mapping : _importers) {
        if (mapping.predicate(path)) {
            return &mapping;
        }
    }

    return nullptr;
}

auto up::recon::ReconApp::_makeMetaFilename(zstring_view basePath, bool directory) -> string {
    if (directory) {
        return path::join(basePath, ".meta");
    }
    string_writer metaFilePath;
    metaFilePath.append(basePath);
    metaFilePath.append(".meta");
    return metaFilePath.to_string();
}

void up::recon::ReconApp::_collectSourceFiles(ReconQueue& queue, bool forceUpdate) {
    if (!fs::directoryExists(_project->resourceRootPath())) {
        _logger.error("`{}' does not exist or is not a directory", _project->resourceRootPath());
        return;
    }

    auto cb = [&queue, forceUpdate](auto const& item, int) {
        // do not recurse into the library folder
        //
        if (item.path.starts_with(".library")) {
            return fs::next;
        }

        // skip .meta files
        //
        if (path::extension(item.path) == ".meta") {
            return fs::next;
        }

        queue.enqueImport(string{item.path}, forceUpdate);

        return fs::recurse;
    };

    (void)fs::enumerate(_project->resourceRootPath(), cb);
};

void up::recon::ReconApp::_collectMissingFiles(ReconQueue& queue) {
    for (zstring_view filename : _library.collectAssetPaths()) {
        string osPath = path::join(path::Separator::Native, _project->resourceRootPath(), filename);

        auto const [rs, _] = fs::fileStat(osPath);
        if (rs == IOResult::FileNotFound) {
            queue.enqueForget(string{filename});
        }
    }
}

bool up::recon::ReconApp::_writeManifest() {
    auto manifestFile = fs::openWrite(_manifestPath.c_str(), fs::OpenMode::Text);
    if (!manifestFile) {
        _logger.error("Failed to open manifest `{}'", _manifestPath);
        return false;
    };
    _library.generateManifest(manifestFile);
    manifestFile.flush();
    manifestFile.close();
    return true;
}
