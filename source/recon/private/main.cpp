// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "converter_app.h"
#include <iostream>

int main(int argc, char const** argv) {
    gm::recon::ConverterApp app;

    if (!app.run(gm::span<char const*>{argv, static_cast<std::size_t>(argc)})) {
        return 1;
    }

    return 0;
}
