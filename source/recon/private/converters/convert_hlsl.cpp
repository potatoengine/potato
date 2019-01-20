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

namespace {
    struct ReconIncludeHandler : public IDxcIncludeHandler {
        ReconIncludeHandler(gm::fs::FileSystem& fileSystem, IDxcLibrary& library, gm::recon::Context& ctx)
            : _fileSystem(fileSystem),
              _library(library),
              _ctx(ctx) {}

        HRESULT __stdcall LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource) override {
            constexpr int len = 4096;
            char buffer[len];
            auto rs = WideCharToMultiByte(CP_UTF8, 0, pFilename, static_cast<int>(std::wcslen(pFilename)), buffer, len, "?", nullptr);
            if (rs == 0 || rs >= len) {
                return E_INVALIDARG;
            }
            buffer[rs] = 0;

            std::string absolutePath = gm::fs::path::join({_ctx.sourceFolderPath().c_str(), buffer});

            std::cout << "Including `" << absolutePath << "'\n";

            _ctx.addSourceDependency(buffer);

            auto stream = _fileSystem.openRead(absolutePath.c_str(), gm::fs::FileOpenMode::Text);
            if (!stream) {
                return E_FAIL;
            }

            stream.seekg(0, std::ios::end);
            auto tell = stream.tellg();
            stream.seekg(0, std::ios::beg);

            gm::vector<char> bytes;
            bytes.resize(tell);

            stream.read(bytes.data(), bytes.size());

            gm::com_ptr<IDxcBlobEncoding> blob;
            auto rs2 = _library.CreateBlobWithEncodingOnHeapCopy(bytes.data(), static_cast<UINT32>(bytes.size()), CP_UTF8, gm::out_ptr(blob));
            if (!SUCCEEDED(rs2)) {
                return rs2;
            }

            *ppIncludeSource = blob.release();

            return S_OK;
        }

        // Inherited via IDxcIncludeHandler
        HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override {
            if (riid == __uuidof(IDxcIncludeHandler)) {
                *ppvObject = static_cast<IDxcIncludeHandler*>(this);
                return S_OK;
            }
            return E_FAIL;
        }

        // no reference-counting for this object allowed
        ULONG __stdcall AddRef(void) override { return 1; }
        ULONG __stdcall Release(void) override { return 1; }

        gm::fs::FileSystem& _fileSystem;
        IDxcLibrary& _library;
        gm::recon::Context& _ctx;
    };
} // namespace

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

    constexpr int len1 = 4096;
    wchar_t buffer1[len1];
    int rs1 = MultiByteToWideChar(CP_UTF8, 0, ctx.sourceFilePath().data(), static_cast<int>(ctx.sourceFilePath().size()), buffer1, len1);
    if (rs1 == 0 || rs1 >= len1) {
        std::cerr << "Failed to translate path `" << ctx.sourceFilePath() << "' as ucs2\n";
        return false;
    }
    buffer1[rs1] = 0;

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

    ReconIncludeHandler includeHandler(fs, *library, ctx);

    com_ptr<IDxcOperationResult>
        result;
    hr = compiler->Compile(blob.get(), buffer1, entry, profile, nullptr, 0, nullptr, 0, &includeHandler, out_ptr(result));
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
