// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/recon/converter_app.h"
#include <iostream>

int main(int argc, char const** argv) {
    up::recon::ConverterApp app;

    if (!app.run(up::span<char const*>{argv, static_cast<std::size_t>(argc)})) {
        return 1;
    }

    return 0;
}
