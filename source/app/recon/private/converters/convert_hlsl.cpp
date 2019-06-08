// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "convert_hlsl.h"
#include "potato/gpu/com_ptr.h"
#include "potato/foundation/out_ptr.h"
#include "potato/foundation/string_view.h"
#include "potato/foundation/string_writer.h"
#include "potato/foundation/std_iostream.h"
#include "potato/filesystem/filesystem.h"
#include "potato/filesystem/stream.h"
#include "potato/filesystem/path.h"
#include "potato/logger/logger.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <fstream>

namespace {
    struct ReconIncludeHandler : public ID3DInclude {
        ReconIncludeHandler(up::FileSystem& fileSystem, up::recon::Context& ctx, up::string_view folder)
            : _fileSystem(fileSystem),
              _ctx(ctx),
              _folder(folder) {}

        HRESULT __stdcall Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) override {
            up::string absolutePath = up::path::join({_folder, pFileName});

            _ctx.logger().info("Including `{}'", absolutePath);

            auto relPath = absolutePath.substr(_ctx.sourceFolderPath().size() + 1);
            _ctx.addSourceDependency(relPath.data());
            
            auto stream = _fileSystem.openRead(absolutePath.c_str(), up::FileOpenMode::Text);
            if (!stream) {
                return E_FAIL;
            }

            up::string shader;
            if (up::readText(stream, shader) != up::IOResult::Success) {
                return E_FAIL;
            }

            *ppData = shader.data();
            *pBytes = static_cast<UINT>(shader.size());

            _shaders.push_back(std::move(shader));

            return S_OK;
        }

        HRESULT __stdcall Close(LPCVOID pData) override {
            return S_OK;
        }

        up::FileSystem& _fileSystem;
        up::recon::Context& _ctx;
        up::string_view _folder;
        up::vector<up::string> _shaders;
    }; // namespace
} // namespace

up::recon::HlslConverter::HlslConverter() = default;

up::recon::HlslConverter::~HlslConverter() = default;

bool up::recon::HlslConverter::convert(Context& ctx) {
    auto absoluteSourcePath = path::join({string_view(ctx.sourceFolderPath()), ctx.sourceFilePath()});

    auto stream = ctx.fileSystem().openRead(absoluteSourcePath.c_str(), up::FileOpenMode::Text);
    if (!stream) {
        return false;
    }

    string shader;
    if (readText(stream, shader) != IOResult::Success) {
        ctx.logger().error("Failed to read `{}'", absoluteSourcePath);
        return false;
    }

    stream.close();

    bool success = compile(ctx, ctx.fileSystem(), absoluteSourcePath, shader, "vertex_main", "vs_5_0");
    success = compile(ctx, ctx.fileSystem(), absoluteSourcePath, shader, "pixel_main", "ps_5_0") && success;

    return success;
}

bool up::recon::HlslConverter::compile(Context& ctx, FileSystem& fileSys, zstring_view absoluteSourcePath, string_view source, zstring_view entryName, zstring_view targetProfileName) {
    ctx.logger().info("Compiling `{}':{} ({})", ctx.sourceFilePath(), entryName, targetProfileName);

    ReconIncludeHandler includeHandler(fileSys, ctx, up::path::parent(absoluteSourcePath));

    com_ptr<ID3DBlob> blob;
    com_ptr<ID3DBlob> errors;
    HRESULT hr = D3DCompile2(source.data(), source.size(), ctx.sourceFilePath().c_str(), nullptr, &includeHandler, entryName.c_str(), targetProfileName.c_str(), D3DCOMPILE_DEBUG | D3DCOMPILE_ENABLE_STRICTNESS, 0, 0, nullptr, 0, out_ptr(blob), out_ptr(errors));
    if (!SUCCEEDED(hr)) {
        ctx.logger().error("Compilation failed for `{}':{} ({))", ctx.sourceFilePath(), entryName, targetProfileName);
        return false;
    }

    string_writer ext;
    ext.write('.');
    ext.write(targetProfileName);
    ext.write(".cbo");

    auto destPath = path::changeExtension(ctx.sourceFilePath(), ext.c_str());
    auto destAbsolutePath = path::join({string_view(ctx.destinationFolderPath()), destPath.c_str()});

    string destParentAbsolutePath(path::parent(destAbsolutePath));

    if (!fileSys.directoryExists(destParentAbsolutePath.c_str())) {
        if (fileSys.createDirectories(destParentAbsolutePath.c_str()) != IOResult::Success) {
            ctx.logger().error("Failed to create `{}'", destParentAbsolutePath);
            // intentionally fall through so we still attempt the copy and get a copy error if fail
        }
    }

    auto compiledOutput = fileSys.openWrite(destAbsolutePath.c_str(), up::FileOpenMode::Binary);
    if (!compiledOutput.isOpen()) {
        ctx.logger().error("Cannot write `{}'", destAbsolutePath);
        return false;
    }

    ctx.addOutput(destPath.c_str());

    compiledOutput.write({(up::byte const*)blob->GetBufferPointer(), blob->GetBufferSize()});
    compiledOutput.close();

    return true;
}
