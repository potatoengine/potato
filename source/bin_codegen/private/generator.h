#pragma once

#include <potato/format/format.h>
#include <potato/format/std_string.h>

#include <iosfwd>
#include <string_view>
#include <unordered_map>

namespace schema {
    struct Module;
}

using GeneratorConfig = std::unordered_map<std::string, std::string>;

struct GeneratorContext {
    std::ostream& output;
    schema::Module const& mod;
    GeneratorConfig const& config;
};

class Generator {
public:
    explicit Generator(GeneratorContext const& ctx) : _output(ctx.output), _module(ctx.mod), _config(ctx.config) {}

    virtual bool generate() = 0;

    int errors() const noexcept { return _errors; }

protected:
    std::ostream& _output;
    schema::Module const& _module;
    GeneratorConfig const& _config;

    void fail(std::string_view message);

    template <typename... ArgsT>
    void fail(std::string_view format_str, ArgsT const&... args) {
        fail(up::format_as<std::string>({format_str.data(), format_str.size()}, args...));
    }

    std::string_view config(std::string const& key) const noexcept {
        auto const it = _config.find(key);
        return it != _config.end() ? std::string_view{it->second} : std::string_view{};
    }

private:
    int _errors = 0;
};
