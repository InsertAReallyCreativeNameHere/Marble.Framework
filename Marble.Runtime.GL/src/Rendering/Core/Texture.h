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

            Texture2D(const Texture2D&) = delete;
            Texture2D(Texture2D&&) = delete;
            Texture2D& operator=(const Texture2D&) = delete;
            Texture2D& operator=(Texture2D&&) = delete;
            
            void update(uint8_t* rgbaImageData, uint16_t imageW, uint16_t imageH);

            friend class Marble::GL::Renderer;
        private:
            uint8_t* imageData;
            bgfx::TextureHandle texture;
        };
    }
}