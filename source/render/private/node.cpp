// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "node.h"
#include "model.h"
#include "camera.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/command_list.h"

gm::Node::Node(box<Camera> camera) : _camera(std::move(camera)) {}

gm::Node::Node(box<Model> model) : _model(std::move(model)) {}

gm::Node::~Node() = default;

void gm::Node::render(gpu::CommandList& commandList, gpu::Device& device) {
    if (_model != nullptr) {
        _model->render(commandList, device);
    }
}
