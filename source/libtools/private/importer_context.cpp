// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "importer_context.h"

void up::ImporterContext::addSourceDependency(zstring_view path) { _sourceDependencies.push_back(string(path)); }

void up::ImporterContext::addOutput(zstring_view path) { _outputs.push_back(string(path.c_str())); }

void up::ImporterContext::addOutputDependency(zstring_view from, zstring_view on, AssetDependencyType type) {}
