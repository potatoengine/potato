// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "node.h"
#include "model.h"
#include "camera.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/command_list.h"

gm::Node::Node(box<Model> model) : _model(std::move(model)) {}

gm::Node::~Node() = default;

void gm::Node::addChild(box<Node> child) {
    _children.push_back(std::move(child));
}

void gm::Node::render(RenderContext& ctx) {
    if (_model != nullptr) {
        _model->render(ctx);
    }

    for (auto& node : _children) {
        node->render(ctx);
    }
}
