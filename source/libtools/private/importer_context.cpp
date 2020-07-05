// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "importer_context.h"

void up::ImporterContext::addSourceDependency(zstring_view path) { _sourceDependencies.push_back(string(path)); }

void up::ImporterContext::addLogicalAsset(string name) { _logicalAssets.push_back(std::move(name)); }

void up::ImporterContext::addOutput(string_view logicalAsset, zstring_view path) { _outputs.push_back(Output{logicalAsset, string(path.c_str())}); }

void up::ImporterContext::addMainOutput(zstring_view path) { _outputs.push_back(Output{{}, string(path.c_str())}); }
