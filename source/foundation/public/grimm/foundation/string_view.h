// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#pragma once

#include "numeric_util.h"
#include "uhash.h"
#include <cstring>
#include <string_view>


namespace gm {
    using string_view = std::string_view;

    template <typename HashAlgorithm>
    inline void hash_append(HashAlgorithm& hasher, string_view const& string);
} // namespace gm

template <typename HashAlgorithm>
void gm::hash_append(HashAlgorithm& hasher, string_view const& string) {
    hasher(string.begin(), string.size());
}
