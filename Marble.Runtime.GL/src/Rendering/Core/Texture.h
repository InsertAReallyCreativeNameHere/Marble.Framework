#pragma once

#include "inc.h"

#include <bgfx/bgfx.h>

namespace Marble
{
    namespace GL
    {
        class Renderer;

        struct coreapi Texture2D final
        {
            Texture2D(uint8_t* rgbaImageData, uint16_t imageW, uint16_t imageH);
            ~Texture2D();

            void update(uint8_t* rgbaImageData, uint16_t imageW, uint16_t imageH);

            friend class Marble::GL::Renderer;
        private:
            bgfx::TextureHandle texture;
        };
    }
}