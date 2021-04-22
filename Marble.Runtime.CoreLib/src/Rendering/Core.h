#pragma once

#include <inc.h>
#include <SDL.h>

namespace Marble
{
    struct coreapi Color final
    {
        byte r;
        byte g;
        byte b;
        byte a;

        Color();
        Color(byte const (&color)[3]);
        Color(byte const (&color)[4]);
        Color(byte r, byte g, byte b, byte a = 255);
        Color(const Color& other);
        Color(const SDL_Color& color);

        Color& operator=(const Color& rhs);

        bool operator==(const Color& rhs);
        bool operator!=(const Color& rhs);
    };

    namespace Internal
    {
        // Modify on render thread only!
        struct coreapi Texture2D final
        {
            SDL_Texture* internalTexture = nullptr;

            Texture2D() = default;
            ~Texture2D() = default;
        };
    }
}