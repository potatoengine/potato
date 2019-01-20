// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "convert_hlsl.h"
#include "grimm/gpu/com_ptr.h"
#include "grimm/gpu/direct3d.h"
#include "grimm/foundation/out_ptr.h"
#include "grimm/foundation/string_view.h"
#include "grimm/filesystem/filesystem.h"
#include "grimm/filesystem/path_util.h"
#include <dxc/dxcapi.h>
#include <iostream>
#include <fstream>

gm::recon::HlslConverter::HlslConverter() = default;

gm::recon::HlslConverter::~HlslConverter() = default;

bool gm::recon::HlslConverter::convert(Context& ctx) {
    com_ptr<IDxcLibrary> library;
    HRESULT hr = DxcCreateInstance(CLSID_DxcLibrary, __uuidof(IDxcLibrary), out_ptr(library));
    if (library.empty()) {
        std::cerr << "Failed to create dxc library\n";
        return false;
    }

    fs::FileSystem fs;

    auto absoluteSourcePath = fs::path::join({string_view(ctx.sourceFolderPath()), ctx.sourceFilePath()});

    constexpr int len = 4096;
    wchar_t buffer[len];
    int rs = MultiByteToWideChar(CP_UTF8, 0, absoluteSourcePath.data(), static_cast<int>(absoluteSourcePath.size()), buffer, len);
    if (rs == 0 || rs >= len) {
        std::cerr << "Failed to translate path `" << absoluteSourcePath << "' as ucs2\n";
        return false;
    }
    buffer[rs] = 0;

    com_ptr<IDxcBlobEncoding> blob;
    hr = library->CreateBlobFromFile(buffer, nullptr, out_ptr(blob));
    if (blob.empty()) {
        std::cerr << "Could not load `" << absoluteSourcePath << "' as dxc blob\n";
        return false;
    }

    com_ptr<IDxcCompiler2> compiler;
    hr = DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler2), out_ptr(compiler));
    if (compiler.empty()) {
        std::cerr << "Failed to create dxc library\n";
        return false;
    }

    wchar_t const entry[] = L"vertex_main";
    wchar_t const profile[] = L"vs_6_0";

    std::cout << "Compiling " << absoluteSourcePath << "':" << entry << '(' << profile << ")\n";

    com_ptr<IDxcOperationResult> result;
    hr = compiler->Compile(blob.get(), buffer, entry, profile, nullptr, 0, nullptr, 0, nullptr, out_ptr(result));
    if (result.empty()) {
        std::cerr << "Failed to compile `" << absoluteSourcePath << "':" << entry << '(' << profile << ")\n";
        return false;
    }

    if (!SUCCEEDED(hr)) {
        com_ptr<IDxcBlobEncoding> errors;
        result->GetErrorBuffer(out_ptr(errors));
        std::cerr << "Compilation failed for `" << absoluteSourcePath << "':" << entry << '(' << profile << ")\n"
                  << string_view((char*)errors->GetBufferPointer(), errors->GetBufferSize()) << '\n';
        return false;
    }

    auto destPath = fs::path::changeExtension(ctx.sourceFilePath(), ".vs_6_0.dxo");
    auto destAbsolutePath = fs::path::join({string_view(ctx.destinationFolderPath()), destPath.c_str()});

    com_ptr<IDxcBlob> compiledBlob;
    result->GetResult(out_ptr(compiledBlob));

    std::string destParentAbsolutePath(fs::path::parent(string_view(destAbsolutePath)));
    if (!fs.directoryExists(destParentAbsolutePath.c_str())) {
        if (!fs.createDirectories(destParentAbsolutePath.c_str())) {
            std::cerr << "Failed to create `" << destParentAbsolutePath << "'\n";
            // intentionally fall through so we still attempt the copy and get a copy error if fail
        }
    }

    std::ofstream compiledOutput(destAbsolutePath.c_str());
    if (!compiledOutput.is_open()) {
        std::cerr << "Cannot write `" << destAbsolutePath << '\n';
        return false;
    }

    ctx.addOutput(destPath.c_str());

    compiledOutput.write((char*)compiledBlob->GetBufferPointer(), compiledBlob->GetBufferSize());
    compiledOutput.close();

    return true;
}
