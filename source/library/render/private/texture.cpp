// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/render/texture.h"
#include "grimm/render/context.h"
#include "grimm/gpu/texture.h"

gm::Texture::Texture(Image image, box<gpu::Texture> texture) : _texture(std::move(texture)), _image(std::move(image)) {}

gm::Texture::~Texture() = default;