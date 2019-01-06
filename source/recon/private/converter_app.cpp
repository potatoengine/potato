// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "converter_app.h"
#include "grimm/foundation/string_view.h"
#include "converters/convert_hlsl.h"
#include "converters/convert_copy.h"
#include <iostream>

gm::recon::ConverterApp::ConverterApp()
    : _programName("recon"), _sourceFolderPath("./resources"), _destinationFolderPath("./converted"), _cacheFolderPath("./cache") {}
gm::recon::ConverterApp::~ConverterApp() = default;

bool gm::recon::ConverterApp::run(span<char const*> args) {
    if (!parseArguments(args)) {
        std::cerr << "Failed to parse arguments\n";
        return false;
    }

    registerConverters();

    auto sources = collectSourceFiles();

    if (sources.empty()) {
        std::cerr << "No source files found\n";
        return false;
    }

    if (!std::filesystem::is_directory(_destinationFolderPath)) {
        std::error_code rs;
        if (!std::filesystem::create_directories(_destinationFolderPath, rs)) {
            std::cerr << "Failed to create `" << _destinationFolderPath << "': " << rs.message() << '\n';
            return false;
        }
    }

    if (!std::filesystem::is_directory(_cacheFolderPath)) {
        std::error_code rs;
        if (!std::filesystem::create_directories(_cacheFolderPath, rs)) {
            std::cerr << "Failed to create `" << _cacheFolderPath << "': " << rs.message() << '\n';
            return false;
        }
    }

    if (!convertFiles(std::move(sources))) {
        std::cerr << "Conversion failed\n";
        return false;
    }

    return true;
}

void gm::recon::ConverterApp::registerConverters() {
    _converters.push_back({[](std::filesystem::path const& path) { return path.extension() == ".hlsl"; },
                           make_box<HlslConverter>()});
    _converters.push_back({[](std::filesystem::path const& path) { return path.extension() == ".json"; },
                           make_box<CopyConverter>()});
}

bool gm::recon::ConverterApp::parseArguments(span<char const*> args) {
    if (args.empty()) {
        return false;
    }

    [[maybe_unused]] auto program = args.front();
    args.pop_front();

    enum {
        ArgNone,
        ArgSourceFolder,
        ArgDestinationFolder,
        ArgCacheFolder,
    } argMode = ArgNone;

    for (string_view arg : args) {
        if (!arg.empty() && arg.front() == '-') {
            if (argMode != ArgNone) {
                std::cerr << "Unexpected option: " << arg << '\n';
                return false;
            }

            auto name = arg.substr(1);
            if (name == "source") {
                argMode = ArgSourceFolder;
            }
            else if (name == "dest") {
                argMode = ArgDestinationFolder;
            }
            else if (name == "cache") {
                argMode = ArgCacheFolder;
            }
            else {
                std::cerr << "Unknown option: " << arg << '\n';
                return false;
            }
            continue;
        }

        switch (argMode) {
        case ArgNone:
            std::cerr << "Unexpected value: " << arg << '\n';
            return false;
        case ArgSourceFolder:
            _sourceFolderPath = std::filesystem::absolute(arg);
            argMode = ArgNone;
            break;
        case ArgDestinationFolder:
            _destinationFolderPath = std::filesystem::absolute(arg);
            argMode = ArgNone;
            break;
        case ArgCacheFolder:
            _cacheFolderPath = std::filesystem::absolute(arg);
            argMode = ArgNone;
            break;
        }
    }

    if (argMode != ArgNone) {
        std::cerr << "Value expected\n";
        return false;
    }

    return true;
}

bool gm::recon::ConverterApp::convertFiles(vector<std::filesystem::path> files) {
    bool failed = false;

    for (auto const& path : files) {
        std::cout << path << '\n';

        Converter* converter = findConverter(path);
        if (converter == nullptr) {
            failed = true;
            std::cerr << "Converter not found for `" << path << "'\n";
            continue;
        }

        Context context(path, _sourceFolderPath, _destinationFolderPath);
        if (!converter->convert(context)) {
            failed = true;
            std::cerr << "Failed conversion for `" << path << "'\n";
            continue;
        }
    }

    return !failed;
}

auto gm::recon::ConverterApp::findConverter(path const& path) const -> Converter* {
    for (auto const& mapping : _converters) {
        if (mapping.predicate(path)) {
            return mapping.conveter.get();
        }
    }

    return nullptr;
}

auto gm::recon::ConverterApp::collectSourceFiles() -> vector<std::filesystem::path> {

    if (!std::filesystem::is_directory(_sourceFolderPath)) {
        std::cerr << "`" << _sourceFolderPath << "' does not exist or is not a directory\n";
        return {};
    }

    vector<std::filesystem::path> files;

    for (auto&& path : std::filesystem::recursive_directory_iterator(_sourceFolderPath)) {
        if (path.is_regular_file()) {
            files.push_back(std::filesystem::relative(std::move(path).path(), _sourceFolderPath));
        }
    }

    return files;
}
