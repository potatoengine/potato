// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "document.h"

up::Document::Document(DocumentType const& type, string name) : _type(&type), _name(std::move(name)) {}
