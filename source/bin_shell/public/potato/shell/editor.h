// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/box.h"
#include "potato/spud/delegate.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

#include <imgui.h>

namespace up {
    class Renderer;
}

namespace up::shell {
    class Editor {
    public:
        using PanelUpdate = delegate<void()>;
        using PanelId = ImGuiID;

        struct Panel {
            string title;
            string imguiLabel;
            bool open = true;
            PanelUpdate update;
            ImGuiID id = 0;
            ImGuiID dockId = 0;
        };

        virtual ~Editor() = default;

        Editor(Editor const&) = delete;
        Editor& operator=(Editor const&) = delete;

        /// @brief Display name of the Document, used in menus and window titles.
        /// @return display name.
        virtual zstring_view displayName() const = 0;

        /// @brief Renders the ui for the Document.
        void render(Renderer& renderer);

        virtual void tick(float deltaTime) {}

        bool isClosed() const noexcept { return _closed; }
        void close() noexcept { _wantClose = isClosable(); }

    protected:
        explicit Editor(zstring_view className);

        ImGuiWindowClass const& documentClass() const noexcept { return _windowClass; }

        auto addPanel(string title, PanelUpdate update) -> PanelId;
        void dockPanel(PanelId panelId, ImGuiDir dir, PanelId otherId, float size);
        auto contentId() const noexcept { return _dockId; }

        /// @brief Renders the ui for the Document.
        virtual void configure() = 0;
        virtual void renderContent(Renderer& renderer) = 0;
        virtual void renderMenu() {}
        virtual bool isClosable() { return true; }
        virtual bool handleClose() { return true; }

    private:
        void _content();

        ImGuiWindowClass _windowClass;
        string _title;
        string _documentId;
        ImGuiID _dockId = 0;

        vector<box<Panel>> _panels;
        bool _wantClose = false;
        bool _closed = false;
    };
} // namespace up::shell
