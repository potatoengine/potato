// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/render/node.h"
#include "grimm/render/model.h"
#include "grimm/render/camera.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/command_list.h"
#include <glm/gtc/matrix_transform.hpp>

gm::Node::Node(box<Model> model) : _transform(glm::identity<glm::mat4x4>()), _model(std::move(model)) {}

gm::Node::~Node() = default;

void gm::Node::addChild(box<Node> child) {
    _children.push_back(std::move(child));
}

void gm::Node::render(RenderContext& ctx) {
    if (_model != nullptr) {
        _model->render(ctx, _transform);
    }

    for (auto& node : _children) {
        node->render(ctx);
    }
}
