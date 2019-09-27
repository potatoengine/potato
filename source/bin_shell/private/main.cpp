// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "shell_app.h"

int main(int argc, char* argv[]) {
    up::ShellApp app;

    int rs = app.initialize();
    if (rs != 0) {
        return rs;
    }

    app.run();

    return 0;
}
