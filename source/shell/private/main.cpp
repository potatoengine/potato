#include "shell_app.h"

int main(int argc, char* argv[]) {
    gm::ShellApp app;

    int rs = app.initialize();
    if (rs != 0) {
        return rs;
    }

    app.run();

    return 0;
}