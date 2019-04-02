#include "grimm/recon/converter_config.h"
#include "grimm/foundation/string_view.h"
#include "grimm/filesystem/filesystem.h"
#include "doctest.h"
#include <spdlog/spdlog.h>

DOCTEST_TEST_SUITE("[grimm][recon] ConverterConfig") {
    using namespace gm;
    using namespace recon;

    DOCTEST_TEST_CASE("args") {
        char const* args[] = {"/bin/test/", "-source", "ABC", "-dest", "DEF", "-cache", "GHI"};
        ConverterConfig config;
        fs::FileSystem fs;

        bool ok = parseArguments(config, args, fs, *spdlog::default_logger_raw());
        DOCTEST_CHECK(ok);

        DOCTEST_CHECK_EQ(config.sourceFolderPath.c_str(), "ABC");
        DOCTEST_CHECK_EQ(config.destinationFolderPath.c_str(), "DEF");
        DOCTEST_CHECK_EQ(config.cacheFolderPath.c_str(), "GHI");
    }

    DOCTEST_TEST_CASE("json") {
        string_view json = R"--({"sourceDir":"ABC","destDir":"DEF","cacheDir":"GHI"})--";
        ConverterConfig config;

        bool ok = parseConfigString(config, json, "test.json", *spdlog::default_logger_raw());
        DOCTEST_CHECK(ok);

        DOCTEST_CHECK_EQ(config.sourceFolderPath.c_str(), "ABC");
        DOCTEST_CHECK_EQ(config.destinationFolderPath.c_str(), "DEF");
        DOCTEST_CHECK_EQ(config.cacheFolderPath.c_str(), "GHI");
    }
}
