// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/recon/recon_config.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/logger.h"
#include "potato/spud/string_view.h"

#include <catch2/catch.hpp>

TEST_CASE("ReconConfig", "[potato][recon]") {
    using namespace up;
    using namespace recon;

    SECTION("args") {
        ReconConfig config;
        char const* args[] = {"/bin/test/", "-source", "ABC"};
        Logger logger("test");

        bool ok = parseArguments(config, args, logger);
        CHECK(ok);

        CHECK(config.sourceFolderPath == "ABC"_s);
    }

    SECTION("json") {
        string_view json = R"--({"sourceDir":"ABC"})--";
        ReconConfig config;
        Logger logger("test");

        bool ok = parseConfigString(config, json, "test.json", logger);
        CHECK(ok);

        CHECK(config.sourceFolderPath == "ABC"_s);
    }
}
