// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/render/texture.h"
#include "potato/render/context.h"
#include "potato/gpu/texture.h"

up::Texture::Texture(Image image, box<GpuTexture> texture) : _texture(std::move(texture)), _image(std::move(image)) {}

up::Texture::~Texture() = default;
