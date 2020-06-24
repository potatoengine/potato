// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/string.h"
#include "potato/spud/zstring_view.h"

#include <imgui.h>

namespace up {
    class Renderer;
}

namespace up::shell {
    class Editor {
    public:
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

        /// @brief Renders the ui for the Document.
        virtual void renderContent(Renderer& renderer) = 0;
        virtual void renderMenu() {}
        virtual void renderPanels() {}
        virtual bool isClosable() { return true; }
        virtual bool handleClose() { return true; }
        virtual auto buildDockSpace(ImGuiID dockSpaceId) -> ImGuiID;

    private:
        ImGuiWindowClass _windowClass;
        string _title;
        string _documentId;
        bool _dirty = false;
        bool _wantClose = false;
        bool _closed = false;
    };
} // namespace up::shell
