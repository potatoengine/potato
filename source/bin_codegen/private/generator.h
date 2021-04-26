#pragma once

#include <potato/format/format.h>
#include <potato/format/std_string.h>

#include <iosfwd>
#include <string_view>

namespace schema {
    struct Module;
}

struct GeneratorContext {
    std::ostream& output;
    schema::Module const& mod;
};

class Generator {
public:
    explicit Generator(GeneratorContext const& ctx) : _output(ctx.output), _module(ctx.mod) {}

    virtual bool generate() = 0;

    int errors() const noexcept { return _errors; }

protected:
    std::ostream& _output;
    schema::Module const& _module;

    void fail(std::string_view message);

    template <typename... ArgsT>
    void fail(std::string_view format_str, ArgsT const&... args) {
        fail(up::format_as<std::string>({format_str.data(), format_str.size()}, args...));
    }

private:
    int _errors = 0;
};
