// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "context.h"

void up::recon::Context::addSourceDependency(zstring_view path) { _sourceDependencies.push_back(string(path)); }

void up::recon::Context::addOutput(zstring_view path) { _outputs.push_back(string(path.c_str())); }

void up::recon::Context::addOutputDependency(zstring_view from, zstring_view on, AssetDependencyType type) {}
