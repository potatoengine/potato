// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/render/node.h"
#include "potato/render/model.h"
#include "potato/render/camera.h"
#include "potato/gpu/device.h"
#include "potato/gpu/command_list.h"
#include <glm/gtc/matrix_transform.hpp>

up::Node::Node(rc<Model> model) : _transform(glm::identity<glm::mat4x4>()), _model(std::move(model)) {}

up::Node::~Node() = default;

void up::Node::addChild(box<Node> child) {
    _children.push_back(std::move(child));
}

void up::Node::render(RenderContext& ctx) {
    if (_model != nullptr) {
        _model->render(ctx, _transform);
    }

    for (auto& node : _children) {
        node->render(ctx);
    }
}
