// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_app.h"
#include "file_hash_cache.h"
#include "meta_file.h"
#include "recon_messages_schema.h"

#include "potato/format/format.h"
#include "potato/recon/recon_log_sink.h"
#include "potato/recon/recon_protocol.h"
#include "potato/runtime/concurrent_queue.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/json.h"
#include "potato/runtime/path.h"
#include "potato/runtime/stream.h"
#include "potato/runtime/uuid.h"
#include "potato/spud/overload.h"
#include "potato/spud/std_iostream.h"
#include "potato/spud/string_view.h"
#include "potato/spud/string_writer.h"

#include <nlohmann/json.hpp>

up::recon::ReconApp::ReconApp() : _programName("recon"), _logger("recon") {}

up::recon::ReconApp::~ReconApp() = default;

bool up::recon::ReconApp::run(span<char const*> args) {
    zstring_view const configFile = "recon.config.json";
    if (fs::fileExists(configFile)) {
        _logger.info("Loading config file `{}'", configFile);
        parseConfigFile(_config, configFile, _logger);
    }

    if (!parseArguments(_config, args, _logger)) {
        _logger.error("Failed to parse arguments");
        return false;
    }

    if (_config.server) {
        Logger::root().attach(new_shared<ReconProtocolLogSink>());
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
    // collect all files in the source directory for conversion
    auto sources = _collectSourceFiles();

    if (sources.empty()) {
        _logger.error("No source files found");
        return false;
    }

    bool success = _importFiles(sources);
    if (!success) {
        _logger.error("Import failed");
    }

    if (!_writeManifest()) {
        _logger.error("Failed to write manifest");
        return false;
    }

    return true;
}

namespace up::recon {
    namespace {
        enum ReconCommandType { Watch, Import, ImportAll, Delete, Quit };
        struct ReconCommand {
            ReconCommandType type = ReconCommandType::Watch;
            fs::WatchAction watchAction = fs::WatchAction::Modify;
            UUID uuid;
            string path;
            string renamedFromPath;
            bool force = false;
        };
    } // namespace
} // namespace up::recon

bool up::recon::ReconApp::_runServer() {
    // import all the initial files
    _runOnce();

    ConcurrentQueue<ReconCommand> commands;

    auto receiver = overload(
        [&](schema::ReconImportMessage const& msg) {
            ReconCommand cmd;
            cmd.type = ReconCommandType::Import;
            cmd.uuid = msg.uuid;
            cmd.force = msg.force;
            commands.enqueWait(std::move(cmd));
        },

        [&](schema::ReconImportAllMessage const& msg) {
            ReconCommand cmd;
            cmd.type = ReconCommandType::ImportAll;
            cmd.force = msg.force;
            commands.enqueWait(std::move(cmd));
        });

    // watch the target resource root and auto-convert any items that come in
    auto [rs, watchHandle] = fs::watchDirectory(_project->resourceRootPath(), [&commands](auto const& watch) {
        // ignore dot files
        if (watch.path.empty() || watch.path.front() == '.') {
            return;
        }
        ReconCommand cmd{
            .type = ReconCommandType::Watch,
            .watchAction = watch.action,
            .path = string{watch.path},
            .renamedFromPath = string{watch.renamedFromPath}};
        commands.enqueWait(cmd);
    });
    if (rs != IOResult::Success) {
        return false;
    }

    // handle processing input from the client
    fs::WatchHandle& handle = *watchHandle;
    std::thread waitParent([this, &handle, &commands, &receiver] {
        nlohmann::json doc;
        std::string line;
        while (std::getline(std::cin, line) && !std::cin.eof()) {
            doc = nlohmann::json::parse(line);
            if (!decodeReconMessage(doc, receiver)) {
                _logger.error("Unhandled JSON message");
                break;
            }
        }
        handle.close();
        commands.enqueWait(ReconCommand{.type = ReconCommandType::Quit});
    });

    ReconCommand cmd;
    bool quit = false;
    bool dirty = false;
    for (;;) {
        // ensure we drain out the remaining queued items
        if (quit) {
            if (!commands.tryDeque(cmd)) {
                break;
            }
        }
        if (commands.dequeWait(cmd)) {
            do {
                switch (cmd.type) {
                    case ReconCommandType::Watch:
                        // changes to meta-files should be processed as changes to the real file
                        if (cmd.path.ends_with(".meta")) {
                            cmd.path = path::changeExtension(cmd.path, "");
                        }

                        _importFile(cmd.path);
                        dirty = true;
                        break;
                    case ReconCommandType::Import:
                        if (auto const* record = _library.findRecordByUuid(cmd.uuid); record != nullptr) {
                            _logger.info("Import: {} (force={})", record->sourcePath, cmd.force);

                            _importFile(record->sourcePath, cmd.force);
                            dirty = true;
                        }
                        break;
                    case ReconCommandType::ImportAll:
                        _logger.info("Import All (force={})", cmd.force);

                        _importFiles(_collectSourceFiles());
                        dirty = true;
                        break;
                    case ReconCommandType::Delete:
                        if (auto const* record = _library.findRecordByUuid(cmd.uuid); record != nullptr) {
                            _logger.info("Delete: {}", record->sourcePath);

                            (void)fs::remove(
                                path::join(path::Separator::Native, _project->resourceRootPath(), record->sourcePath));
                            _importFile(record->sourcePath, false);
                            dirty = true;
                        }
                        break;
                    case ReconCommandType::Quit:
                        handle.close();
                        quit = true;
                        break;
                }
            } while (commands.tryDeque(cmd));
        }
        else {
            handle.close();
        }

        if (dirty) {
            dirty = false;
            _writeManifest();

            schema::ReconManifestMessage msg;
            msg.path = _manifestPath;
            nlohmann::json doc;
            encodeReconMessage(doc, msg);
            std::cout << doc.dump() << "\r\n";
            std::cout.flush();
        }

        if (!handle.isOpen()) {
            quit = true;
        }
    }

    waitParent.join();

    // server only "fails" if it can't run at all
    return true;
}

void up::recon::ReconApp::_registerImporters() {
    _importerFactory.registerDefaultImporters();

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

bool up::recon::ReconApp::_importFiles(view<string> files, bool force) {
    bool failed = false;

    for (auto const& path : files) {
        if (!_importFile(path, force)) {
            failed = true;
        }
    }

    return !failed;
}

bool up::recon::ReconApp::_importFile(zstring_view file, bool force) {
    auto osPath = path::join(path::Separator::Native, _project->resourceRootPath(), file.c_str());

    auto const [statRs, stat] = fs::fileStat(osPath);
    bool const deleted = statRs == IOResult::FileNotFound;
    bool const isFolder = stat.type == fs::FileType::Directory;

    auto metaPath = _makeMetaFilename(file, isFolder);
    auto const contentHash = isFolder ? 0 : _hashes.hashAssetAtPath(osPath.c_str());

    static const ImporterConfig defaultConfig;

    Mapping const* const mapping = isFolder ? nullptr : _findConverterMapping(file);
    Importer* const importer = mapping != nullptr ? mapping->conveter : nullptr;
    ImporterConfig const& importerConfig = mapping != nullptr ? *mapping->config : defaultConfig;

    ImporterContext
        context(file, _project->resourceRootPath(), _temporaryOutputPath, importer, importerConfig, _logger);
    bool dirty = deleted;
    dirty |= !_checkMetafile(context, metaPath, !deleted);

    auto const* record = _library.findRecordByUuid(context.uuid());
    if (!deleted && importer != nullptr) {
        dirty |=
            record == nullptr || !_isUpToDate(*record, contentHash, *importer) || !_isUpToDate(record->dependencies);
    }

    string_writer importedName;
    {
        if (context.uuid().isValid()) {
            format_append(importedName, "{{{}} ", context.uuid());
        }
        importedName.append(file);
        if (importer != nullptr) {
            format_append(importedName, " ({})", importer->name());
        }
    }

    if (deleted) {
        if (record != nullptr) {
            _logger.info("{}: deleted", importedName);
            _library.deleteRecordByUuid(record->uuid);

            auto metaOsPath = path::join(path::Separator::Native, _project->resourceRootPath(), metaPath);
            if (fs::fileExists(metaOsPath)) {
                (void)fs::remove(metaOsPath);
            }
        }
        return true;
    }

    if (importer == nullptr && !isFolder) {
        _logger.error("{}: unknown file type", importedName);
        return false;
    }

    if (!dirty && !force) {
        _logger.info("{}: up-to-date", importedName);
        return true;
    }

    if (importer != nullptr) {
        _logger.info("{}: importing", importedName);

        if (!importer->import(context)) {
            _logger.error("{}: import failed", importedName);
            return false;
        }
    }

    AssetDatabase::Imported newRecord;
    newRecord.uuid = context.uuid();
    newRecord.sourcePath = string(file);
    newRecord.sourceContentHash = contentHash;
    if (importer != nullptr) {
        newRecord.importerName = string(importer->name());
        newRecord.importerRevision = importer->revision();
    }

    string_writer logicalAssetName;

    // move outputs to CAS
    //
    for (auto const& output : context.outputs()) {
        auto outputOsPath = path::join(path::Separator::Native, _temporaryOutputPath, output.path);
        auto const outputHash = _hashes.hashAssetAtPath(outputOsPath);

        logicalAssetName.clear();
        format_append(
            logicalAssetName,
            "{}{}{}",
            newRecord.sourcePath,
            output.logicalAsset.empty() ? "" : ":",
            output.logicalAsset);
        auto const logicalAssetId = AssetDatabase::createLogicalAssetId(context.uuid(), output.logicalAsset);

        newRecord.outputs.push_back(
            {.name = output.logicalAsset,
             .type = output.type,
             .logicalAssetId = logicalAssetId,
             .contentHash = outputHash});

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
            _logger.error("Failed to move temp file `{}` to CAAS `{}`", outputOsPath, casOsPath);
            continue;
        }
    }

    for (auto const& sourceDepPath : context.sourceDependencies()) {
        auto osPath = path::join(_project->resourceRootPath(), sourceDepPath.c_str());
        auto const contentHash = _hashes.hashAssetAtPath(osPath.c_str());
        newRecord.dependencies.push_back({string(sourceDepPath), contentHash});
    }

    _library.insertRecord(std::move(newRecord));
    return true;
}

bool up::recon::ReconApp::_isUpToDate(
    AssetDatabase::Imported const& record,
    up::uint64 contentHash,
    Importer const& importer) const noexcept {
    return record.sourceContentHash == contentHash && string_view(record.importerName) == importer.name() &&
        record.importerRevision == importer.revision();
}

bool up::recon::ReconApp::_isUpToDate(span<AssetDatabase::Dependency const> records) {
    for (auto const& rec : records) {
        auto osPath = path::join(path::Separator::Native, _project->resourceRootPath(), rec.path.c_str());
        auto const contentHash = _hashes.hashAssetAtPath(osPath.c_str());
        if (contentHash != rec.contentHash) {
            return false;
        }
    }
    return true;
}

auto up::recon::ReconApp::_findConverterMapping(string_view path) const -> Mapping const* {
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

auto up::recon::ReconApp::_checkMetafile(ImporterContext& ctx, zstring_view metaPath, bool autoCreate) -> bool {
    string metaOsPath = path::join(path::Separator::Native, _project->resourceRootPath(), metaPath);

    MetaFile metaFile;
    bool dirty = false;

    if (Stream stream = fs::openRead(metaOsPath, fs::OpenMode::Text); stream) {
        auto [result, jsonText] = readText(stream);
        if (result == IOResult::Success) {
            if (!metaFile.parseJson(jsonText)) {
                metaFile.generate();
                dirty = true;
            }
        }
        else {
            metaFile.generate();
            dirty = true;
        }
    }
    else {
        metaFile.generate();
        dirty = true;
    }

    if (ctx.importer() != nullptr) {
        if (ctx.importer()->name() != string_view{metaFile.importerName}) {
            metaFile.importerName = string{ctx.importer()->name()};
            dirty = true;
        }

        string_view settings = ctx.importer()->generateSettings(ctx);
        if (settings != metaFile.importerSettings) {
            metaFile.importerSettings = string{settings};
            dirty = true;
        }
    }

    ctx.setUuid(metaFile.uuid);

    if (dirty && autoCreate) {
        _logger.info("Writing meta file `{}'", metaOsPath);

        string jsonText = metaFile.toJson();

        auto stream = fs::openWrite(metaOsPath, fs::OpenMode::Text);
        if (!stream || writeAllText(stream, jsonText) != IOResult::Success) {
            _logger.error("Failed to write meta file for {}", metaOsPath);
            return false;
        }
    }

    // adding meta files to source deps to ensure proper rebuild when meta files change for any reason
    // (like convert settings for a file)
    ctx.addSourceDependency(metaPath);
    return true;
}

auto up::recon::ReconApp::_collectSourceFiles() -> vector<string> {
    if (!fs::directoryExists(_project->resourceRootPath())) {
        _logger.error("`{}' does not exist or is not a directory", _project->resourceRootPath());
        return {};
    }

    vector<string> files;
    auto cb = [&files](auto const& item, int) {
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

        files.push_back(item.path);

        return fs::recurse;
    };

    (void)fs::enumerate(_project->resourceRootPath(), cb);
    return files;
};

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
