// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "converter.h"
#include <iostream>

int main(int argc, char const** argv) {
    gm::recon::Converter converter;

    if (!converter.run(gm::span<char const*>{argv, static_cast<std::size_t>(argc)})) {
        return 1;
    }

    return 0;
}
