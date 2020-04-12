// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/render/texture.h"
#include "potato/render/context.h"
#include "potato/render/gpu_texture.h"

up::Texture::Texture(Image image, rc<GpuTexture> texture) : _texture(std::move(texture)), _image(std::move(image)) {}

up::Texture::~Texture() = default;
