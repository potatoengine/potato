// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/recon/recon_config.h"
#include "potato/runtime/logger.h"
#include "potato/runtime/native.h"
#include "potato/spud/string_view.h"

#include <doctest/doctest.h>

DOCTEST_TEST_SUITE("[potato][recon] ReconConfig") {
    using namespace up;
    using namespace recon;

    DOCTEST_TEST_CASE("args") {
        char const* args[] = {"/bin/test/", "-source", "ABC"};
        ReconConfig config;
        NativeFileSystem fs;
        Logger logger("test");

        bool ok = parseArguments(config, args, fs, logger);
        DOCTEST_CHECK(ok);

        DOCTEST_CHECK_EQ(config.sourceFolderPath.c_str(), "ABC");
    }

    DOCTEST_TEST_CASE("json") {
        string_view json = R"--({"sourceDir":"ABC"})--";
        ReconConfig config;
        Logger logger("test");

        bool ok = parseConfigString(config, json, "test.json", logger);
        DOCTEST_CHECK(ok);

        DOCTEST_CHECK_EQ(config.sourceFolderPath.c_str(), "ABC");
    }
}
