// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "texture.h"
#include "context.h"
#include "gpu_texture.h"

up::Texture::Texture(AssetId id, Image image, rc<GpuTexture> texture)
    : AssetBase(id)
    , _texture(std::move(texture))
    , _image(std::move(image)) {}

up::Texture::~Texture() = default;
