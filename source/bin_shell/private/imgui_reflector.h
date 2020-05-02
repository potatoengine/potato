// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#include <potato/ecs/reflector.h>

namespace up::shell {
    class ImGuiComponentReflector final : public up::ComponentReflector {
    protected:
        void onField(up::zstring_view name) override;
        void onValue(int& value) override;
        void onValue(float& value) override;
        void onValue(up::EntityId value) override;
        void onValue(glm::vec3& value) override;
        void onValue(glm::quat& value) override;
        void onValue(glm::mat4x4& value) override;

    private:
        up::zstring_view _name;
    };
} // namespace up::shell
