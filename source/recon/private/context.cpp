// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/recon/context.h"

void gm::recon::Context::addSourceDependency(zstring_view path) {
    _sourceDependencies.push_back(std::string(path));
}

void gm::recon::Context::addOutput(zstring_view path) {
    _outputs.push_back(std::string(path.c_str()));
}

void gm::recon::Context::addOutputDependency(zstring_view from, zstring_view on, AssetDependencyType type) {
}
