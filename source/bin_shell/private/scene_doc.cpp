// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "scene_doc.h"

int up::SceneDocument::indexOf(EntityId entityId) const noexcept {
    for (int index : indices()) {
        if (_entities[index].id == entityId) {
            return index;
        }
    }
    return -1;
}

up::EntityId up::SceneDocument::createEntity() {
    return _scene->world().createEntity();
}
