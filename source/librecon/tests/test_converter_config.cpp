#include "potato/recon/converter_config.h"
#include "potato/spud/string_view.h"
#include "potato/runtime/null.h"
#include "potato/runtime/logger.h"
#include <doctest/doctest.h>

DOCTEST_TEST_SUITE("[potato][recon] ConverterConfig") {
    using namespace up;
    using namespace recon;

    DOCTEST_TEST_CASE("args") {
        char const* args[] = {"/bin/test/", "-source", "ABC", "-dest", "DEF", "-cache", "GHI"};
        ConverterConfig config;
        NullFileSystem fs;
        Logger logger("test");

        bool ok = parseArguments(config, args, fs, logger);
        DOCTEST_CHECK(ok);

        DOCTEST_CHECK_EQ(config.sourceFolderPath.c_str(), "ABC");
        DOCTEST_CHECK_EQ(config.destinationFolderPath.c_str(), "DEF");
        DOCTEST_CHECK_EQ(config.cacheFolderPath.c_str(), "GHI");
    }

    DOCTEST_TEST_CASE("json") {
        string_view json = R"--({"sourceDir":"ABC","destDir":"DEF","cacheDir":"GHI"})--";
        ConverterConfig config;
        Logger logger("test");

        bool ok = parseConfigString(config, json, "test.json", logger);
        DOCTEST_CHECK(ok);

        DOCTEST_CHECK_EQ(config.sourceFolderPath.c_str(), "ABC");
        DOCTEST_CHECK_EQ(config.destinationFolderPath.c_str(), "DEF");
        DOCTEST_CHECK_EQ(config.cacheFolderPath.c_str(), "GHI");
    }
}
