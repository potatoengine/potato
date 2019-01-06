// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/span.h"
#include "grimm/foundation/string_view.h"
#include "grimm/foundation/vector.h"
#include <filesystem>

namespace gm::recon {
    class Converter {
    public:
        Converter();
        ~Converter();

        Converter(Converter const&) = delete;
        auto operator=(Converter const&) = delete;

        bool run(span<char const*> args);

    private:
        bool parseArguments(span<char const*> args);

        vector<std::filesystem::path> collectSourceFiles();
        bool convertFiles(vector<std::filesystem::path> files);

    private:
        string_view _programName;
        std::filesystem::path _sourceFolderPath;
        std::filesystem::path _destinationFolderPath;
        std::filesystem::path _cacheFolderPath;
    };
} // namespace gm::recon
