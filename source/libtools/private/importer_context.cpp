// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "importer_context.h"

void up::ImporterContext::addSourceDependency(zstring_view path) { _sourceDependencies.push_back(string(path)); }

void up::ImporterContext::addOutput(string logicalAsset, string path) { _outputs.push_back(Output{std::move(logicalAsset), std::move(path)}); }

void up::ImporterContext::addMainOutput(string path) { _outputs.push_back(Output{{}, std::move(path.c_str())}); }
