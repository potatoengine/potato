// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/string.h"
#include "potato/spud/zstring_view.h"

#include <imgui.h>

namespace up {
    class Renderer;
}

namespace up::shell {
    class Document {
    public:
        virtual ~Document() = default;

        Document(Document const&) = delete;
        Document& operator=(Document const&) = delete;

        /// @brief Display name of the Document, used in menus and window titles.
        /// @return display name.
        virtual zstring_view displayName() const = 0;

        /// @brief Renders the ui for the Document.
        void render(Renderer& renderer);

    protected:
        explicit Document(zstring_view className);

        /// @brief Renders the ui for the Document.
        virtual void renderContent(Renderer& renderer) = 0;
        virtual void renderMenu() {}
        virtual void buildDockSpace(ImGuiID dockId, zstring_view docId) {}

    private:
        ImGuiWindowClass _windowClass;
        string _documentId;
    };
} // namespace up::shell
