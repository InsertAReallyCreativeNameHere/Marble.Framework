#include "Texture.h"

#include <Rendering/Core/Renderer.h>

using namespace Marble::GL;

Texture2D::Texture2D(uint8_t* rgbaImageData, uint16_t imageW, uint16_t imageH) :
texture
(
    bgfx::createTexture2D
    (
        imageW, imageH, false, 1, bgfx::TextureFormat::RGBA8,
        BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE,
        bgfx::makeRef(rgbaImageData, imageW * imageH * 4)
    )
)
{
}
Texture2D::~Texture2D()
{
    bgfx::TextureHandle tex = this->texture;

    Renderer::finalizers.push_back
    (
        [=]
        {
            bgfx::destroy(tex);
        }
    );
}

void Texture2D::update(uint8_t* rgbaImageData, uint16_t imageW, uint16_t imageH)
{
    bgfx::updateTexture2D(this->texture, 1, 0, 0, 0, imageW, imageH, bgfx::makeRef(rgbaImageData, imageW * imageH * 4), imageW * 4);
}