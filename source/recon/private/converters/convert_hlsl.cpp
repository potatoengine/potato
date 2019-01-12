// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "convert_hlsl.h"
#include "grimm/gpu/direct3d.h"
#include "grimm/gpu/com_ptr.h"
#include "grimm/foundation/out_ptr.h"
#include "grimm/foundation/string_view.h"
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

    path absoluteSourcePath = ctx.sourceFolderPath() / ctx.sourceFilePath();

    com_ptr<IDxcBlobEncoding> blob;
    hr = library->CreateBlobFromFile(absoluteSourcePath.c_str(), nullptr, out_ptr(blob));
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
    hr = compiler->Compile(blob.get(), ctx.sourceFilePath().c_str(), entry, profile, nullptr, 0, nullptr, 0, nullptr, out_ptr(result));
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

    path destPath = ctx.sourceFilePath();
    destPath.replace_extension(".vs_6_0.dxo");

    path destAbsolutePath = ctx.destinationFolderPath() / destPath;

    com_ptr<IDxcBlob> compiledBlob;
    result->GetResult(out_ptr(compiledBlob));

    std::ofstream compiledOutput(destAbsolutePath);
    if (!compiledOutput.is_open()) {
        std::cerr << "Cannot write `" << destAbsolutePath << '\n';
        return false;
    }

    compiledOutput.write((char*)compiledBlob->GetBufferPointer(), compiledBlob->GetBufferSize());
    compiledOutput.close();

    return true;
}
