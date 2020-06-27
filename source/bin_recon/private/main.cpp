// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "converter_app.h"

#include <iostream>

int main(int argc, char const** argv) {
    up::recon::ConverterApp app;

    if (!app.run(up::span<char const*>{argv, static_cast<std::size_t>(argc)})) {
        return 1;
    }

    return 0;
}
