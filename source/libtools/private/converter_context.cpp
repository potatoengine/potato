// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "converter_context.h"

void up::ConverterContext::addSourceDependency(zstring_view path) { _sourceDependencies.push_back(string(path)); }

void up::ConverterContext::addOutput(zstring_view path) { _outputs.push_back(string(path.c_str())); }

void up::ConverterContext::addOutputDependency(zstring_view from, zstring_view on, AssetDependencyType type) {}
