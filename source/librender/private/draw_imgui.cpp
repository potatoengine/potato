// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/render/draw_imgui.h"
#include "potato/render/shader.h"
#include "potato/render/gpu_buffer.h"
#include "potato/render/gpu_command_list.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_pipeline_state.h"
#include "potato/render/gpu_texture.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/gpu_sampler.h"
#include <potato/runtime/assertion.h>
#include <imgui.h>
#include <SDL_events.h>
#include <SDL_clipboard.h>

static constexpr up::uint32 bufferSize = 256 * 1024;

up::DrawImgui::DrawImgui() = default;
up::DrawImgui::~DrawImgui() = default;

void up::DrawImgui::bindShaders(rc<Shader> vertShader, rc<Shader> pixelShader) {
    _vertShader = std::move(vertShader);
    _pixelShader = std::move(pixelShader);

    // we'll need to recreate pipeline state at the least
    releaseResources();
}

bool up::DrawImgui::createResources(GpuDevice& device) {
    _ensureContext();

    GpuInputLayoutElement layout[] = {
        {GpuFormat::R32G32Float, GpuShaderSemantic::Position, 0, 0},
        {GpuFormat::R32G32Float, GpuShaderSemantic::TexCoord, 0, 0},
        {GpuFormat::R8G8B8A8UnsignedNormalized, GpuShaderSemantic::Color, 0, 0},
    };

    GpuPipelineStateDesc desc;
    desc.enableScissor = true;
    desc.vertShader = _vertShader->content();
    desc.pixelShader = _pixelShader->content();
    desc.inputLayout = layout;

    _indexBuffer = device.createBuffer(GpuBufferType::Index, bufferSize);
    _vertexBuffer = device.createBuffer(GpuBufferType::Vertex, bufferSize);
    _constantBuffer = device.createBuffer(GpuBufferType::Constant, sizeof(float) * 16);
    _pipelineState = device.createPipelineState(desc);

    auto& imguiIO = ImGui::GetIO();

    int fontWidth = 0;
    int fontHeight = 0;
    unsigned char* pixels = nullptr;
    imguiIO.Fonts->GetTexDataAsRGBA32(&pixels, &fontWidth, &fontHeight);
    GpuTextureDesc texDesc;
    texDesc.format = GpuFormat::R8G8B8A8UnsignedNormalized;
    texDesc.type = GpuTextureType::Texture2D;
    texDesc.width = fontWidth;
    texDesc.height = fontHeight;

    auto font = device.createTexture2D(texDesc, span{pixels, static_cast<uint32>(fontWidth * fontHeight * 4)}.as_bytes());
    _srv = device.createShaderResourceView(font.get());

    _sampler = device.createSampler();

    return true;
}

auto up::DrawImgui::loadFontAwesome5(Stream fontFile) -> bool {
    static constexpr auto s_minGlyph = 0xf000;
    static constexpr auto s_maxGlyph = 0xf897;
    static constexpr ImWchar s_ranges[] = {s_minGlyph, s_maxGlyph, 0};

    _ensureContext();

    ImGui::SetCurrentContext(_context.get());
    auto& io = ImGui::GetIO();

    vector<byte> fontData;
    if (readBinary(fontFile, fontData) != IOResult::Success) {
        return false;
    }

    
    ImFontConfig config; // = {.MergeMode = true, .PixelSnapH = true};
    config.MergeMode = true;
    config.PixelSnapH = true;
    config.FontDataOwnedByAtlas = false;

    auto font = io.Fonts->AddFontFromMemoryTTF(fontData.data(), static_cast<int>(fontData.size()), 13.0f, &config, s_ranges);
    return font != nullptr;
}

void up::DrawImgui::releaseResources() {
    _indexBuffer.reset();
    _vertexBuffer.reset();
    _constantBuffer.reset();
    _pipelineState.reset();
    _srv.reset();
    _sampler.reset();
}

void up::DrawImgui::beginFrame() {
    _ensureContext();

    ImGui::SetCurrentContext(_context.get());
    ImGui::NewFrame();
}

bool up::DrawImgui::handleEvent(SDL_Event const& ev) {
    UP_ASSERT(!_context.empty());

    ImGui::SetCurrentContext(_context.get());
    ImGuiIO& io = ImGui::GetIO();

    switch (ev.type) {
    case SDL_MOUSEMOTION:
        io.MousePos = {(float)ev.motion.x, (float)ev.motion.y};
        return io.WantCaptureMouse;
    case SDL_MOUSEBUTTONDOWN:
        io.MouseDown[0] = true;
        io.MouseClickedPos[0] = {(float)ev.button.x, (float)ev.button.y};
        return io.WantCaptureMouse;
    case SDL_MOUSEBUTTONUP:
        io.MouseDown[0] = false;
        io.MouseClickedPos[0] = {(float)ev.button.x, (float)ev.button.y};
        return io.WantCaptureMouse;
    case SDL_MOUSEWHEEL:
        if (io.WantCaptureMouse) {
            if (ev.wheel.y > 0) {
                io.MouseWheel += 1;
            }
            else if (ev.wheel.y < 0) {
                io.MouseWheel -= 1;
            }

            if (ev.wheel.x > 0) {
                io.MouseWheelH += 1;
            }
            else if (ev.wheel.x < 0) {
                io.MouseWheelH -= 1;
            }
            return true;
        }
        break;
    case SDL_TEXTINPUT:
        if (io.WantTextInput) {
            io.AddInputCharactersUTF8(ev.text.text);
            return true;
        }
        break;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        if (io.WantCaptureKeyboard) {
            int key = ev.key.keysym.scancode;
            IM_ASSERT(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown));
            io.KeysDown[key] = (ev.type == SDL_KEYDOWN);
            io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
            io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
            io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
            io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
            return true;
        }
        break;
    }
    return false;
}

void up::DrawImgui::endFrame(GpuDevice& device, GpuCommandList& commandList) {
    UP_ASSERT(!_context.empty());

    ImGui::SetCurrentContext(_context.get());
    ImGui::Render();

    if (_pipelineState.empty()) {
        createResources(device);
    }

    ImDrawData& data = *ImGui::GetDrawData();

    UP_ASSERT(data.Valid, "DrawImgui::draw() can only be called after Render() but before beginFrame()");

    UP_ASSERT(data.TotalIdxCount * sizeof(ImDrawIdx) <= bufferSize, "Too many ImGui indices");
    UP_ASSERT(data.TotalVtxCount * sizeof(ImDrawVert) <= bufferSize, "Too many ImGui verticies");

    commandList.setPipelineState(_pipelineState.get());
    commandList.setPrimitiveTopology(GpuPrimitiveTopology::Triangles);

    auto indices = commandList.map(_indexBuffer.get(), bufferSize);
    auto vertices = commandList.map(_vertexBuffer.get(), bufferSize);

    uint32 indexOffset = 0;
    uint32 vertexOffset = 0;

    for (int listIndex = 0; listIndex != data.CmdListsCount; ++listIndex) {
        auto const& list = *data.CmdLists[listIndex];

        std::memcpy(indices.data() + indexOffset, list.IdxBuffer.Data, list.IdxBuffer.Size * sizeof(ImDrawIdx));
        std::memcpy(vertices.data() + vertexOffset, list.VtxBuffer.Data, list.VtxBuffer.Size * sizeof(ImDrawVert));

        indexOffset += list.IdxBuffer.Size * sizeof(ImDrawIdx);
        vertexOffset += list.VtxBuffer.Size * sizeof(ImDrawVert);
    }

    commandList.unmap(_indexBuffer.get(), indices);
    commandList.unmap(_vertexBuffer.get(), vertices);

    float L = data.DisplayPos.x;
    float R = data.DisplayPos.x + data.DisplaySize.x;
    float T = data.DisplayPos.y;
    float B = data.DisplayPos.y + data.DisplaySize.y;
    float mvp[4][4] = {
        {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
        {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
        {0.0f, 0.0f, 0.5f, 0.0f},
        {(R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f},
    };

    auto constants = commandList.map(_constantBuffer.get(), sizeof(mvp));
    std::memcpy(constants.data(), mvp, constants.size());
    commandList.unmap(_constantBuffer.get(), constants);

    commandList.bindIndexBuffer(_indexBuffer.get(), GpuIndexFormat::Unsigned16, 0);
    commandList.bindVertexBuffer(0, _vertexBuffer.get(), sizeof(ImDrawVert));
    commandList.bindConstantBuffer(0, _constantBuffer.get(), GpuShaderStage::Vertex);
    commandList.bindSampler(0, _sampler.get(), GpuShaderStage::Pixel);

    GpuViewportDesc viewport;
    viewport.width = data.DisplaySize.x;
    viewport.height = data.DisplaySize.y;
    viewport.leftX = 0;
    viewport.topY = 0;
    viewport.minDepth = 0;
    viewport.maxDepth = 1;
    commandList.setViewport(viewport);

    indexOffset = 0;
    vertexOffset = 0;

    ImVec2 pos = data.DisplayPos;

    for (int listIndex = 0; listIndex != data.CmdListsCount; ++listIndex) {
        auto const& list = *data.CmdLists[listIndex];

        for (int cmdIndex = 0; cmdIndex != list.CmdBuffer.Size; ++cmdIndex) {
            auto const& cmd = list.CmdBuffer.Data[cmdIndex];

            if (cmd.UserCallback != nullptr) {
                cmd.UserCallback(&list, &cmd);
                continue;
            }

            GpuClipRect scissor;
            scissor.left = (uint32)cmd.ClipRect.x - (uint32)pos.x;
            scissor.top = (uint32)cmd.ClipRect.y - (uint32)pos.y;
            scissor.right = (uint32)cmd.ClipRect.z - (uint32)pos.x;
            scissor.bottom = (uint32)cmd.ClipRect.w - (uint32)pos.y;
            commandList.setClipRect(scissor);

            // FIXME: different texture per cmd
            commandList.bindShaderResource(0, _srv.get(), GpuShaderStage::Pixel);
            commandList.drawIndexed(cmd.ElemCount, indexOffset, vertexOffset);

            indexOffset += cmd.ElemCount;
        }

        vertexOffset += list.VtxBuffer.Size;
    }
}

char const* up::DrawImgui::_getClipboardTextContents(void* self) {
    auto imgui = static_cast<DrawImgui*>(self);
    imgui->_clipboardTextData = up::string(SDL_GetClipboardText());
    return imgui->_clipboardTextData.c_str();
}

void up::DrawImgui::_setClipboardTextContents(void* self, char const* zstr) {
    auto imgui = static_cast<DrawImgui*>(self);
    imgui->_clipboardTextData.reset();
    SDL_SetClipboardText(zstr);
}

void up::DrawImgui::_ensureContext() {
    if (_context) {
        return;
    }

    _context = ImGui::CreateContext();
    ImGui::SetCurrentContext(_context.get());
    auto& io = ImGui::GetIO();

    io.Fonts->AddFontDefault();

    io.BackendPlatformName = "potato::grui";
    io.IniFilename = nullptr;

    io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
    io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
    io.KeyMap[ImGuiKey_Insert] = SDL_SCANCODE_INSERT;
    io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
    io.KeyMap[ImGuiKey_Space] = SDL_SCANCODE_SPACE;
    io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
    io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
    io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
    io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
    io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
    io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
    io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
    io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;

    io.SetClipboardTextFn = _setClipboardTextContents;
    io.GetClipboardTextFn = _getClipboardTextContents;
    io.ClipboardUserData = this;
}

void up::DrawImgui::_freeContext(ImGuiContext* ctx) {
    if (ctx != nullptr) {
        ImGui::DestroyContext(ctx);
    }
}
