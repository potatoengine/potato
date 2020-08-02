// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "commands.h"
#include "ui/menu.h"

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
        using EditorId = uint64;

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

        /// @brief Return a string that uniquely identifiers the Editor class type.
        virtual zstring_view editorClass() const = 0;

        /// @brief Return a globally-unique string that identifies the Editor instance.
        virtual EditorId uniqueId() const = 0;

        /// @brief Updates the UI.
        bool updateUi();

        /// @brief Renders the ui for the Document.
        virtual void render(Renderer& renderer, float deltaTime) {}

        virtual void tick(float deltaTime) {}

        virtual void handleCommand(string_view command) {}

        bool isClosed() const noexcept { return _closed; }
        virtual bool isClosable() { return true; }
        void close() noexcept { _wantClose = isClosable(); }

        bool isActive() const noexcept { return _active; }
        void activate(bool active, CommandRegistry& commands, Menu& menu);

        CommandProvider const& commands() const noexcept { return _commands; }
        MenuProvider const& menu() const noexcept { return _menu; }

    protected:
        explicit Editor(zstring_view className);

        ImGuiWindowClass const& documentClass() const noexcept { return _windowClass; }

        auto addPanel(string title, PanelUpdate update) -> PanelId;
        void dockPanel(PanelId panelId, ImGuiDir dir, PanelId otherId, float size);
        auto contentId() const noexcept { return _dockId; }
        void addCommand(CommandDesc command) { _commands.addCommand(std::move(command)); }
        void addMenuItem(MenuItemDesc desc) { _menu.addMenuItem(std::move(desc)); }

        /// @brief Renders the ui for the Document.
        virtual void configure() = 0;
        virtual void content() = 0;
        virtual bool hasMenu() { return false; }
        virtual bool handleClose() { return true; }

    private:
        void _content();

        ImGuiWindowClass _windowClass;
        string _title;
        string _documentId;
        ImGuiID _dockId = 0;

        vector<box<Panel>> _panels;
        CommandProvider _commands;
        MenuProvider _menu;
        bool _wantClose = false;
        bool _closed = false;
        bool _active = false;
    };
} // namespace up::shell
