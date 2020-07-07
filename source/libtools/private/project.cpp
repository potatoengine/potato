// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "project.h"

#include "potato/runtime/filesystem.h"
#include "potato/runtime/json.h"
#include "potato/runtime/path.h"
#include "potato/runtime/stream.h"

#include <nlohmann/json.hpp>

auto up::Project::loadFromFile(zstring_view filename) -> box<Project> {
    auto const [rs, docText] = fs::readText(filename);
    if (rs != IOResult::Success) {
        return nullptr;
    }

    auto doc = nlohmann::json::parse(docText, nullptr, false);
    if (doc.is_discarded()) {
        return nullptr;
    }

    if (doc["$type"] != "potato.project") {
        return nullptr;
    }

    if (doc["$version"] != 0) {
        return nullptr;
    }

    auto const assets = doc["assets"];
    if (!assets.is_object()) {
        return nullptr;
    }

    auto const projectBasePath = path::parent(filename);

    auto const rootPath = assets["root"].get<string>();
    auto const targetPath = assets["target"].get<string>();
    auto const cachePath = assets["cache"].get<string>();

    auto project = new_box<Project>();
    project->_assetRootPath = path::join(projectBasePath, rootPath);
    project->_targetPath = path::join(projectBasePath, targetPath);
    project->_cachePath = path::join(projectBasePath, cachePath);

    return project;
}
