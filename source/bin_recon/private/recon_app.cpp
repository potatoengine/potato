// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_app.h"
#include "recon_messages_schema.h"

#include "potato/format/format.h"
#include "potato/recon/file_hash_cache.h"
#include "potato/recon/recon_log_sink.h"
#include "potato/recon/recon_protocol.h"
#include "potato/tools/meta_file.h"
#include "potato/runtime/concurrent_queue.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/json.h"
#include "potato/runtime/path.h"
#include "potato/runtime/stream.h"
#include "potato/runtime/uuid.h"
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

    _manifestPath = path::join(_project->libraryPath(), "manifest.txt");

    if (auto const rs = fs::createDirectories(_project->libraryPath()); rs != IOResult::Success) {
        _logger.error("Failed to create library folder `{}`: {}", _project->libraryPath(), rs);
        return false;
    }

    _temporaryOutputPath = path::join(_project->libraryPath(), "temp");

    _registerImporters();

    auto libraryPath = path::join(_project->libraryPath(), "assets.db");
    if (!_library.open(libraryPath)) {
        _logger.error("Failed to open asset library `{}'", libraryPath);
    }
    _logger.info("Opened asset library `{}'", libraryPath);

    auto hashCachePath = path::join(_project->libraryPath(), "hash_cache.db");
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
        enum ReconCommandType { Watch, Import, ImportAll, Quit };
        struct ReconCommand {
            ReconCommandType type = ReconCommandType::Watch;
            fs::WatchAction watchAction = fs::WatchAction::Modify;
            string path;
            bool force = false;
        };
    } // namespace
} // namespace up::recon

bool up::recon::ReconApp::_runServer() {
    // import all the initial files
    _runOnce();

    ConcurrentQueue<ReconCommand> commands;

    // watch the target resource root and auto-convert any items that come in
    auto [rs, watchHandle] = fs::watchDirectory(_project->resourceRootPath(), [&commands](auto const& watch) {
        ReconCommand cmd{.type = ReconCommandType::Watch, .watchAction = watch.action, .path = watch.path};
        commands.enqueWait(cmd);
    });
    if (rs != IOResult::Success) {
        return false;
    }

    // handle processing input from the client
    fs::WatchHandle& handle = *watchHandle;
    std::thread waitParent([&handle, &commands] {
        nlohmann::json doc;
        std::string line;
        box<schema::ReconMessage> msg;
        reflex::Schema const* schema = nullptr;
        while (std::getline(std::cin, line) && !std::cin.eof()) {
            doc = nlohmann::json::parse(line);
            if (!decodeReconMessage(doc, msg, schema)) {
                break;
            }
            if (schema == &reflex::getSchema<schema::ReconImportMessage>()) {
                auto const& importMsg = static_cast<schema::ReconImportMessage const&>(*msg);
                ReconCommand cmd{.type = ReconCommandType::Import, .path = importMsg.path, .force = importMsg.force};
                commands.enqueWait(cmd);
                continue;
            }
            if (schema == &reflex::getSchema<schema::ReconImportAllMessage>()) {
                auto const& importMsg = static_cast<schema::ReconImportAllMessage const&>(*msg);
                ReconCommand cmd{.type = ReconCommandType::ImportAll, .force = importMsg.force};
                commands.enqueWait(cmd);
            }
        }
        handle.close();
        commands.enqueWait(ReconCommand{.type = ReconCommandType::Quit});
    });

    ReconCommand cmd;
    bool quit = false;
    for (;;) {
        if (!handle.isOpen()) {
            quit = true;
        }

        // ensure we drain out the remaining queued items
        if (quit) {
            if (!commands.tryDeque(cmd)) {
                break;
            }
        }
        else if (!commands.dequeWait(cmd)) {
            handle.close();
            continue;
        }

        switch (cmd.type) {
            case ReconCommandType::Watch:
                if (cmd.path.starts_with(".library")) {
                    continue;
                }

                if (cmd.path.ends_with(".meta")) {
                    cmd.path = path::changeExtension(cmd.path, "");
                }

                _logger.info("Change: {}", cmd.path);

                _importFile(cmd.path, cmd.force);
                _writeManifest();
                break;
            case ReconCommandType::Import:
                _logger.info("Import: {} (force={})", cmd.path, cmd.force);

                _importFile(cmd.path, cmd.force);
                _writeManifest();
                break;
            case ReconCommandType::ImportAll:
                _logger.info("Import All (force={})", cmd.force);

                _runOnce();
                break;
            case ReconCommandType::Quit:
                handle.close();
                quit = true;
                break;
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
    auto assetId = _library.pathToAssetId(file);
    auto record = _library.findRecord(assetId);

    auto osPath = path::join(_project->resourceRootPath(), file.c_str());
    auto const contentHash = _hashes.hashAssetAtPath(osPath.c_str());

    Mapping const* const mapping = _findConverterMapping(file);
    if (mapping == nullptr) {
        _logger.error("Importer not found for `{}'", file);
        return false;
    }
    Importer* const importer = mapping->conveter;

    bool const upToDate =
        record != nullptr && _isUpToDate(*record, contentHash, *importer) && _isUpToDate(record->dependencies);
    if (upToDate && !force) {
        _logger.info("Up-to-date: {}", file);
        return true;
    }

    bool const deleted = !fs::fileExists(osPath);
    if (deleted) {
        _logger.info("Deleted: {}", file);
        return true;
    }

    _logger.info("Importing: {} (importer={} revision={})", file, importer->name(), importer->revision());

    ImporterContext context(file, _project->resourceRootPath(), _temporaryOutputPath, *mapping->config, _logger);
    if (!_checkMetafile(context, file)) {
        return true;
    }

    if (!importer->import(context)) {
        _logger.error("Failed import for `{}'", file);
        return false;
    }

    AssetLibrary::Imported newRecord;
    newRecord.assetId = assetId;
    newRecord.sourcePath = string(file);
    newRecord.sourceContentHash = contentHash;
    newRecord.importerName = string(importer->name());
    newRecord.importerRevision = importer->revision();

    string_writer logicalAssetName;

    // move outputs to CAS
    //
    for (auto const& output : context.outputs()) {
        auto outputOsPath = path::join(_temporaryOutputPath, output.path);
        auto const outputHash = _hashes.hashAssetAtPath(outputOsPath);

        logicalAssetName.clear();
        format_append(
            logicalAssetName,
            "{}{}{}",
            newRecord.sourcePath,
            output.logicalAsset.empty() ? "" : ":",
            output.logicalAsset);
        auto const logicalAssetId = _library.pathToAssetId(logicalAssetName);

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

        auto casOsPath = path::join(_project->libraryPath(), "cache", casPath);
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
    AssetLibrary::Imported const& record,
    up::uint64 contentHash,
    Importer const& importer) const noexcept {
    return record.sourceContentHash == contentHash && string_view(record.importerName) == importer.name() &&
        record.importerRevision == importer.revision();
}

bool up::recon::ReconApp::_isUpToDate(span<AssetLibrary::Dependency const> records) {
    for (auto const& rec : records) {
        auto osPath = path::join(_project->resourceRootPath(), rec.path.c_str());
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

auto up::recon::ReconApp::_checkMetafile(ImporterContext& ctx, zstring_view filename) -> bool {
    string_writer metaFilePath;
    metaFilePath.append(filename);
    metaFilePath.append(".meta");

    string metaFileOsPath = path::join(_project->resourceRootPath(), metaFilePath);

    MetaFile metaFile;
    bool dirty = false;

    if (Stream stream = fs::openRead(metaFileOsPath, fs::OpenMode::Text); stream) {
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

    Mapping const* const mapping = _findConverterMapping(filename);
    if (mapping != nullptr) {
        Importer* const importer = mapping->conveter;
        if (importer->name() != string_view{metaFile.importerName}) {
            metaFile.importerName = string{importer->name()};
            dirty = true;
        }

        ImporterContext
            context(filename, _project->resourceRootPath(), _temporaryOutputPath, *mapping->config, _logger);
        string_view settings = importer->generateSettings(context);
        if (settings != metaFile.importerSettings) {
            metaFile.importerSettings = string{settings};
            dirty = true;
        }
    }

    if (dirty) {
        _logger.info("Writing meta file `{}'", metaFilePath);

        string jsonText = metaFile.toJson();

        auto stream = fs::openWrite(metaFileOsPath, fs::OpenMode::Text);
        if (!stream || writeAllText(stream, jsonText) != IOResult::Success) {
            _logger.error("Failed to write meta file for {}", metaFilePath);
            return false;
        }
    }

    // adding meta files to source deps to ensure proper rebuild when meta files change for any reason
    // (like convert settings for a file)
    ctx.addSourceDependency(metaFilePath);
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

        if (item.type == fs::FileType::Regular) {
            // skip .meta files
            //
            if (path::extension(item.path) == ".meta") {
                return fs::next;
            }

            files.push_back(item.path);
        }

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

    if (_config.server) {
        schema::ReconManifestMessage msg;
        msg.path = _manifestPath;
        nlohmann::json doc;
        encodeReconMessage(doc, msg);
        std::cout << doc.dump() << "\r\n";
    }

    return true;
}
