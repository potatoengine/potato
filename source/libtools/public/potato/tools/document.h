// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "_export.h"

#include "potato/spud/box.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up {
    class Document;

    class DocumentType {
    public:
        virtual ~DocumentType() = default;

        virtual auto uniqueTag() const noexcept -> zstring_view = 0;
        virtual auto create(string name) -> box<Document> = 0;
    };

    class Document {
    public:
        virtual ~Document() = default;

        DocumentType const& type() const noexcept { return *_type; }

        virtual zstring_view name() const noexcept = 0;

    protected:
        UP_TOOLS_API explicit Document(DocumentType const& type, string name);

    private:
        DocumentType const* _type = nullptr;
        string _name;
    };

    class DocumentRegistry {
    public:
        template <typename T, typename... Args> void registerType(Args&&... args) { _types.push_back(new_box<T>(std::forward<Args>(args)...)); }

    private:
        vector<box<DocumentType>> _types;
    };
} // namespace up
